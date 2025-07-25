/*
 * This file is a part of QTerminal - http://gitorious.org/qterminal
 *
 * This file was un-linked from KDE and modified
 * by Maxim Bourmistrov <maxim@unixconn.com>
 *
 */

/*
    This file is part of Konsole, an X terminal.
    Copyright 1997,1998 by Lars Doelle <lars.doelle@on-line.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

// Own
#include "Pty.h"
#include "kpty.h"
#include "kptydevice.h"

// System
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <termios.h>
#include <csignal>

// Qt
#include <QStringList>
#include <QtDebug>
#include <QMessageBox>
#include <QDir>
#include <QRegExp>
#include <QTextCodec>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#else
#include <QRegExp>
#include <QRegExpValidator>
#endif
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(procLog)

using namespace Konsole;

void Pty::setWindowSize(int lines, int cols)
{
    qCDebug(procLog) << "Setting window size to lines:" << lines << "cols:" << cols;
    _windowColumns = cols;
    _windowLines = lines;

    if (pty()->masterFd() >= 0)
        pty()->setWinSize(lines, cols);
}
QSize Pty::windowSize() const
{
    qCDebug(procLog) << "Getting window size, cols:" << _windowColumns << "lines:" << _windowLines;
    return {_windowColumns, _windowLines};
}

void Pty::setFlowControlEnabled(bool enable)
{
    qCDebug(procLog) << "Setting flow control enabled to:" << enable;
    _xonXoff = enable;

    if (pty()->masterFd() >= 0) {
        struct ::termios ttmode;
        pty()->tcGetAttr(&ttmode);
        if (!enable)
            ttmode.c_iflag &= ~(IXOFF | IXON);
        else
            ttmode.c_iflag |= (IXOFF | IXON);
        if (!pty()->tcSetAttr(&ttmode))
            qCWarning(procLog) << "Unable to set terminal attributes.";
    }
}
bool Pty::flowControlEnabled() const
{
    qCDebug(procLog) << "Getting flow control status from terminal.";
    if (pty()->masterFd() >= 0) {
        qCDebug(procLog) << "The terminal is connected.";
        struct ::termios ttmode;
        pty()->tcGetAttr(&ttmode);
        const bool enabled = (ttmode.c_iflag & IXOFF) && (ttmode.c_iflag & IXON);
        qCDebug(procLog) << "Flow control is" << (enabled ? "enabled" : "disabled");
        return enabled;
    }
    qCWarning(procLog) << "Unable to get flow control status, terminal not connected.";
    return false;
}

void Pty::setUtf8Mode(bool enable)
{
#ifdef IUTF8 // XXX not a reasonable place to check it.
    _utf8 = enable;

    qCDebug(procLog) << "Setting UTF-8 mode to" << enable;
    if (pty()->masterFd() >= 0) {
        struct ::termios ttmode;
        pty()->tcGetAttr(&ttmode);
        if (!enable)
            ttmode.c_iflag &= ~IUTF8;
        else
            ttmode.c_iflag |= IUTF8;
        if (!pty()->tcSetAttr(&ttmode))
            qCWarning(procLog) << "Unable to set terminal attributes.";
    }
#endif
}

void Pty::setErase(char erase)
{
    _eraseChar = erase;

    qCDebug(procLog) << "Setting erase character to" << erase;
    if (pty()->masterFd() >= 0) {
        struct ::termios ttmode;
        pty()->tcGetAttr(&ttmode);
        ttmode.c_cc[VERASE] = erase;
        if (!pty()->tcSetAttr(&ttmode))
            qCWarning(procLog) << "Unable to set terminal attributes.";
    }
}

char Pty::erase() const
{
    qCDebug(procLog) << "Getting erase character from terminal.";
    if (pty()->masterFd() >= 0) {
        struct ::termios ttyAttributes;
        pty()->tcGetAttr(&ttyAttributes);
        char eraseChar = ttyAttributes.c_cc[VERASE];
        qCDebug(procLog) << "Terminal erase char is:" << eraseChar;
        return eraseChar;
    }

    qCWarning(procLog) << "Unable to get erase character, terminal not connected. Returning default:" << _eraseChar;
    return _eraseChar;
}

void Pty::addEnvironmentVariables(const QStringList &environment)
{
    qCDebug(procLog) << "Adding environment variables:" << environment;

    QListIterator<QString> iter(environment);
    while (iter.hasNext()) {
        QString pair = iter.next();

        // split on the first '=' character
        int pos = pair.indexOf(QLatin1Char('='));

        if (pos >= 0) {
            QString variable = pair.left(pos);
            QString value = pair.mid(pos + 1);

            setEnv(variable, value);
            qCDebug(procLog) << "Added environment variable:" << variable << "=" << value;
        }
    }
    qCDebug(procLog) << "Environment variables added.";
}

int Pty::start(const QString &program,
               const QStringList &programArguments,
               const QStringList &environment,
               ulong winid,
               bool addToUtmp
               //const QString& dbusService,
               //const QString& dbusSession
              )
{
    qCDebug(procLog) << "Starting program:" << program << "with args:" << programArguments;
    clearProgram();

    // For historical reasons, the first argument in programArguments is the
    // name of the program to execute, so create a list consisting of all
    // but the first argument to pass to setProgram()
    Q_ASSERT(programArguments.count() >= 1);
    setProgram(program, programArguments.mid(1));
    _program = program;

    addEnvironmentVariables(environment);

    setEnv(QLatin1String("WINDOWID"), QString::number(winid));
    setEnv(QLatin1String("COLORTERM"), QLatin1String("truecolor"));

    // unless the LANGUAGE environment variable has been set explicitly
    // set it to a null string
    // this fixes the problem where KCatalog sets the LANGUAGE environment
    // variable during the application's startup to something which
    // differs from LANG,LC_* etc. and causes programs run from
    // the terminal to display messages in the wrong language
    //
    // this can happen if LANG contains a language which KDE
    // does not have a translation for
    //
    // BR:149300
    setEnv(QLatin1String("LANGUAGE"), QString(), false /* do not overwrite existing value if any */);

    setUseUtmp(addToUtmp);

    struct ::termios ttmode;
    pty()->tcGetAttr(&ttmode);
    if (!_xonXoff)
        ttmode.c_iflag &= ~(IXOFF | IXON);
    else
        ttmode.c_iflag |= (IXOFF | IXON);
#ifdef IUTF8 // XXX not a reasonable place to check it.
    if (!_utf8)
        ttmode.c_iflag &= ~IUTF8;
    else
        ttmode.c_iflag |= IUTF8;
#endif

    if (_eraseChar != 0)
        ttmode.c_cc[VERASE] = _eraseChar;

    if (!pty()->tcSetAttr(&ttmode))
        qCWarning(procLog) << "Unable to set terminal attributes.";

    pty()->setWinSize(_windowLines, _windowColumns);

    KProcess::start();

    if (!waitForStarted()) {
        qCWarning(procLog) << "Unable to start program:" << program;
        return -1;
    }

    qCDebug(procLog) << "Program started successfully:" << program;
    return 0;
}

void Pty::setEmptyPTYProperties()
{
    qCDebug(procLog) << "Setting empty PTY properties.";
    struct ::termios ttmode;
    pty()->tcGetAttr(&ttmode);
    if (!_xonXoff)
        ttmode.c_iflag &= ~(IXOFF | IXON);
    else
        ttmode.c_iflag |= (IXOFF | IXON);
#ifdef IUTF8 // XXX not a reasonable place to check it.
    if (!_utf8)
        ttmode.c_iflag &= ~IUTF8;
    else
        ttmode.c_iflag |= IUTF8;
#endif

    if (_eraseChar != 0)
        ttmode.c_cc[VERASE] = _eraseChar;

    if (!pty()->tcSetAttr(&ttmode))
        qCWarning(procLog) << "Unable to set terminal attributes.";
}

void Pty::setWriteable(bool writeable)
{
    qCDebug(procLog) << "Setting TTY" << pty()->ttyName() << "writeable to:" << writeable;
    struct stat sbuf;
    if (stat(pty()->ttyName(), &sbuf) != 0) {
        qCWarning(procLog) << "Failed to stat TTY:" << pty()->ttyName() << "error:" << strerror(errno);
        return;
    }
    if (writeable)
        chmod(pty()->ttyName(), sbuf.st_mode | S_IWGRP);
    else
        chmod(pty()->ttyName(), sbuf.st_mode & ~(S_IWGRP | S_IWOTH));
}

Pty::Pty(int masterFd, QObject *parent)
    : KPtyProcess(masterFd, parent)
{
    qCDebug(procLog) << "Pty created with master FD:" << masterFd;
    init();
}

Pty::Pty(QObject *parent)
    : KPtyProcess(parent)
{
    qCDebug(procLog) << "Pty created without master FD";
    init();
}

void Pty::init()
{
    qCDebug(procLog) << "Initializing Pty properties";
    _windowColumns = 0;
    _windowLines = 0;
    _eraseChar = 0;
    _xonXoff = true;
    _utf8 = true;
    _bUninstall = false;

    connect(pty(), SIGNAL(readyRead()), this, SLOT(dataReceived()));
    setPtyChannels(KPtyProcess::AllChannels);
}

Pty::~Pty()
{
    qCDebug(procLog) << "Pty destroyed";
}

bool Pty::isTerminalRemoved()
{
    qCDebug(procLog) << "Checking if deepin-terminal is removed by checking for /usr/bin/deepin-terminal";
    QFile terminalExecFile("/usr/bin/deepin-terminal");
    if (terminalExecFile.exists()) {
        qCDebug(procLog) << "deepin-terminal exists.";
        return false;
    }

    qCDebug(procLog) << "deepin-terminal does not exist, assuming it is removed.";
    return true;
}

bool isPatternAcceptable(QString strCommand, QString strPattern)
{
    qCDebug(procLog) << "Checking if command '" << strCommand << "' is acceptable for pattern '" << strPattern << "'";
    QString strTrimmedCmd = strCommand.trimmed();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QRegularExpression cmdRegExp(strPattern);
    QRegularExpressionValidator cmdREValidator(cmdRegExp, nullptr);
#else
    QRegExp cmdRegExp;
    cmdRegExp.setPattern(strPattern);
    QRegExpValidator cmdREValidator(cmdRegExp, nullptr);
#endif

    int pos = 0;
    QValidator::State validateState = cmdREValidator.validate(strTrimmedCmd, pos);

    bool isAcceptable = (validateState == QValidator::Acceptable);
    qCDebug(procLog) << "Validation result:" << isAcceptable;
    return isAcceptable;
}

//判断当前命令是否是要删除终端
bool Pty::bWillRemoveTerminal(QString strCommand)
{
    qCDebug(procLog) << "Checking if command will remove terminal:" << strCommand;
    QString packageName = "deepin-terminal";

    QStringList strCommandList;
    strCommandList.append(strCommand);

    if (strCommand.contains("&&")) {
        qCDebug(procLog) << "Command contains '&&', splitting...";
        QStringList cmdList = strCommand.split("&&");
        for (int i = 0; i < cmdList.size(); i++) {
            QString currCmd = cmdList.at(i).trimmed();
            if (currCmd.length() > 0 && currCmd.contains(packageName)) {
                strCommandList.append(currCmd);
            }
        }
    }

    if (strCommand.contains(";")) {
        qCDebug(procLog) << "Command contains ';', splitting...";
        QStringList cmdList = strCommand.split(";");
        for (int i = 0; i < cmdList.size(); i++) {
            QString currCmd = cmdList.at(i).trimmed();
            if (currCmd.length() > 0 && currCmd.contains(packageName)) {
                strCommandList.append(currCmd);
            }
        }
    }

    QList<bool> acceptableList;

    QStringList packageNameList;
    packageNameList << packageName;

    for (int i = 0; i < strCommandList.size(); i++) {
        QString strCurrCommand = strCommandList.at(i);
        for (int j = 0; j < packageNameList.size(); j++) {
            QString packageName = packageNameList.at(j);
            QString removePattern = QString("sudo\\s+apt-get\\s+remove\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+apt\\s+remove\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);
            /******** Modify by nt001000 renfeixiang 2020-05-27:修改 放到bWillPurgeTerminal函数中 Begin***************/
//            removePattern = QString("sudo\\s+dpkg\\s+-P\\s+%1").arg(packageName);
//            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);
            /******** Modify by nt001000 renfeixiang 2020-05-27:修改 放到bWillPurgeTerminal函数中 End***************/

            removePattern = QString("sudo\\s+dpkg\\s+-r\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+rm\\s+.+\\s+/usr/bin/deepin-terminal");
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+rm\\s+/usr/bin/deepin-terminal");
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);
        }
    }

    const bool bWillRemove = acceptableList.contains(true);
    qCDebug(procLog) << "Will remove terminal:" << bWillRemove;
    return bWillRemove;
}

/******** Add by nt001000 renfeixiang 2020-05-27:增加 Purge卸载命令的判断，显示不同的卸载提示框 Begin***************/
bool Pty::bWillPurgeTerminal(QString strCommand)
{
    qCDebug(procLog) << "Checking if command will purge terminal:" << strCommand;
    QString packageName = "deepin-terminal";

    QStringList strCommandList;
    strCommandList.append(strCommand);

    if (strCommand.contains("&&")) {
        qCDebug(procLog) << "Command contains '&&', splitting...";
        QStringList cmdList = strCommand.split("&&");
        for (int i = 0; i < cmdList.size(); i++) {
            QString currCmd = cmdList.at(i).trimmed();
            if (currCmd.length() > 0 && currCmd.contains(packageName)) {
                strCommandList.append(currCmd);
            }
        }
    }

    if (strCommand.contains(";")) {
        qCDebug(procLog) << "Command contains ';', splitting...";
        QStringList cmdList = strCommand.split(";");
        for (int i = 0; i < cmdList.size(); i++) {
            QString currCmd = cmdList.at(i).trimmed();
            if (currCmd.length() > 0 && currCmd.contains(packageName)) {
                strCommandList.append(currCmd);
            }
        }
    }

    QList<bool> acceptableList;

    QStringList packageNameList;
    packageNameList << packageName;

    for (int i = 0; i < strCommandList.size(); i++) {
        QString strCurrCommand = strCommandList.at(i);
        for (int j = 0; j < packageNameList.size(); j++) {
            QString packageName = packageNameList.at(j);
            QString removePattern = QString("sudo\\s+apt-get\\s+purge\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+apt-get\\s+purge\\s+-y\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+apt-get\\s+remove\\s+--purge\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+apt-get\\s+--purge\\s+remove\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+apt\\s+purge\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+apt\\s+purge\\s+-y\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+apt\\s+remove\\s+--purge\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+apt\\s+--purge\\s+remove\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+dpkg\\s+-P\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);
        }
    }

    const bool bWillPurge = acceptableList.contains(true);
    qCDebug(procLog) << "Will purge terminal:" << bWillPurge;
    return bWillPurge;
}
/******** Add by nt001000 renfeixiang 2020-05-27:增加 Purge卸载命令的判断，显示不同的卸载提示框 End***************/

void Pty::sendData(const char *data, int length, const QTextCodec *codec)
{
    _textCodec = codec;

    if (!length) {
        return;
    }
    // qCDebug(procLog) << "Sending data with length:" << length;

    //判断是否是点了自定义命令面板列表项触发的命令
    bool isCustomCommand = false;
    QString currCommand = QString::fromLatin1(data);
    if (currCommand.length() > 0 && currCommand.endsWith('\n')) {
        isCustomCommand = true;
    }

    _isCommandExec = false;
    _bNeedBlockCommand = false;

    //为GBK/GB2312/GB18030编码，且不是输入命令执行的情况（没有按回车）
    if (QString(codec->name()).toUpper().startsWith("GB") && !_isCommandExec) {
        QTextCodec *utf8Codec = QTextCodec::codecForName("UTF-8");
        QString unicodeData = codec->toUnicode(data);
        QByteArray unicode = utf8Codec->fromUnicode(unicodeData);

        // qCDebug(procLog) << "Converted input data to UTF-8:" << unicode;
        if (!pty()->write(unicode.constData(), unicode.length())) {
            qCWarning(procLog) << "Failed to send input data to terminal process";
            return;
        }
    }
    else {
        // qCDebug(procLog) << "Sending raw input data:" << QByteArray(data, length);
        if (!pty()->write(data, length)) {
            qCWarning(procLog) << "Failed to send input data to terminal process";
            return;
        }
    }

}

void Pty::dataReceived()
{
    QByteArray data = pty()->readAll();

    QString recvData = QString(data);
    qCDebug(procLog) << "Received data:" << recvData;

    if (_bNeedBlockCommand) {
        QString judgeData = recvData;
        if (recvData.length() > 1) {
            judgeData = recvData.replace("\r", "");
            judgeData = judgeData.replace("\n", "");
        }

        //使用zsh的时候，发送过来的字符会残留一个字母"e"，需要特殊处理下
        if (_program.endsWith("/zsh")
                && 1 == judgeData.length()
                && judgeData.startsWith("e")
                && -1 == _receiveDataIndex) {
            _receiveDataIndex = 0;
            qCDebug(procLog) << "Ignore the first letter 'e' of the command.";
            return;
        }

        //不显示远程登录时候的敏感信息(主要是expect -f命令跟随的明文密码)
        //同时考虑了zsh的情况
        if (judgeData.startsWith("expect -f")
                || judgeData.startsWith("\bexpect")
                || judgeData.startsWith("\be")
                || judgeData.startsWith("e\bexpect")
                || judgeData.startsWith("e\be")) {
            _receiveDataIndex = 1;
            qCDebug(procLog) << "Ignore the expect command.";
            return;
        }

        if (_receiveDataIndex >= 1) {
            if (judgeData.contains("Press")) {
                //这里需要置回false，否则后面其他命令也会被拦截
                _bNeedBlockCommand = false;

                _receiveDataIndex = -1;
                int pressStringIndex = recvData.indexOf("Press");
                if (pressStringIndex > 0) {
                    recvData = recvData.mid(pressStringIndex);
                }
                QString helpData = recvData.replace("\n", "");
                recvData = "\r\n" + helpData + "\r\n";
                data = recvData.toUtf8();
                emit receivedData(data.constData(), data.count(), _textCodec);
            }
            else {
                ++_receiveDataIndex;
            }
            qCDebug(procLog) << "Ignore the command:" << judgeData;
            return;
        }
    }

    /******** Modify by m000714 daizhengwen 2020-04-30: 处理上传下载时乱码显示命令不执行****************/
    // 乱码提示信息不显示
    if (recvData.contains("bash: $'\\212")
            || recvData.contains("bash: **0800000000022d：")
            || recvData.contains("**^XB0800000000022d")
            || recvData.startsWith("**\u0018B0800000000022d\r\u008A")) {
        qCWarning(procLog) << "Ignoring garbled command:" << recvData;
        return;
    }

    // "\u008A"这个乱码不替换调会导致显示时有\b的效果导致命令错乱bug#23741
    if (recvData.contains("\u008A")) {
        qCDebug(procLog) << "Replacing backspace control character";
        recvData.replace("\u008A", "\b \b #");
        data = recvData.toUtf8();
    }

    if (recvData == "rz waiting to receive.") {
        qCDebug(procLog) << "Detected rz file transfer prompt";
        recvData += "\r\n";
        data = recvData.toUtf8();
    }
    /********************* Modify by m000714 daizhengwen End ************************/
    emit receivedData(data.constData(), data.count(), _isCommandExec);
}

void Pty::lockPty(bool lock)
{
    qCDebug(procLog) << "Pty::lockPty called with lock:" << lock;
    Q_UNUSED(lock);

// TODO: Support for locking the Pty
    //if (lock)
    //suspend();
    //else
    //resume();
}

int Pty::foregroundProcessGroup() const
{
    int pid = tcgetpgrp(pty()->masterFd());
    qDebug() << "Foreground process group:" << pid;

    if (pid != -1) {
        return pid;
    }

    qDebug() << "Unable to get foreground process group, returning 0.";
    return 0;
}

void Pty::setSessionId(int sessionId)
{
    _sessionId = sessionId;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void Pty::setupChildProcess()
#else
void Pty::setupChildProcessImpl()
#endif
{

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    KPtyProcess::setupChildProcess();
#else
    KPtyProcess::setupChildProcessImpl();
#endif

    // reset all signal handlers
    // this ensures that terminal applications respond to
    // signals generated via key sequences such as Ctrl+C
    // (which sends SIGINT)
    struct sigaction action;
    sigset_t sigset;
    sigemptyset(&action.sa_mask);
    sigemptyset(&sigset);
    action.sa_handler = SIG_DFL;
    action.sa_flags = 0;
    for (int signal = 1; signal < NSIG; signal++) {
        sigaction(signal, &action, nullptr);
        sigaddset(&sigset, signal);
    }
    sigprocmask(SIG_UNBLOCK, &sigset, nullptr);
}
