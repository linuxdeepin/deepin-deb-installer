// SPDX-FileCopyrightText: 2022 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "installDebThread.h"

#include <QDebug>
#include <QVersionNumber>

static const QString kParamInstallWine = "install_wine";
static const QString kParamInstallConfig = "install_config";
static const QString kParamInstallComaptible = "install_compatible";
static const QString kParamInstallImmutable = "install_immutable";
static const QString kParamInstallUab = "uab";

static const QString kAptBin = "apt";
static const QString kInstall = "install";
static const QString kRemove = "remove";

// for compatible mode
static const QString kCompatibleBin = "deepin-compatible-ctl";
static const QString kCompCheck = "check";
static const QString kCompApp = "app";
static const QString kCompRootfs = "rootfs";
static const QString kCompRootfsName = "--name";
static const QString kCompJsonFormat = "--json";
// current user(non root)
static const QString kCompUser = "user";

// for disable DebConf
static const QString kDebConfEnv = "DEBIAN_FRONTEND";
static const QString kDebConfDisable = "noninteractive";

InstallDebThread::InstallDebThread()
{
    qDebug() << "Initializing InstallDebThread";
    m_proc = new KProcess;

    // Note: 目前 deepin-deb-installer 使用 KPty 捕获所有通道进行设置，因此旧版的
    // installDebThread 手动捕获输入流程不再响应。
    // 修改输入模式为响应主进程输入，而不是手动管理。
    m_proc->setInputChannelMode(QProcess::ForwardedInputChannel);

    connect(m_proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onFinished(int, QProcess::ExitStatus)));
    connect(m_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadoutput()));
    connect(m_proc, &KProcess::readyReadStandardError, this, [this]() { qWarning() << m_proc->readAllStandardError(); });
}

InstallDebThread::~InstallDebThread()
{
    qDebug() << "Destroying InstallDebThread";
    if (m_proc)
        delete m_proc;
}

void InstallDebThread::setParam(const QStringList &arguments)
{
    if (!m_listParam.isEmpty()) {
        qDebug() << "m_listParam is not empty, return";
        return;
    }

    // normal command
    static QMap<QString, Command> kParamMap{{kParamInstallWine, InstallWine},
                                            {kParamInstallConfig, InstallConfig},
                                            {kParamInstallComaptible, Compatible},
                                            {kParamInstallImmutable, Immutable},
                                            {kParamInstallUab, LinglongUab},
                                            {kInstall, Install},
                                            {kRemove, Remove},
                                            {kCompCheck, AppCheck}};

    for (auto itr = kParamMap.begin(); itr != kParamMap.end(); ++itr) {
        m_parser.addOption(QCommandLineOption(itr.key()));
    }
    QCommandLineOption rootfsOpt(kCompRootfs, "", "rootfsname");
    m_parser.addOption(rootfsOpt);
    QCommandLineOption userOpt(kCompUser, "", "current user");
    m_parser.addOption(userOpt);

    m_parser.process(arguments);

    for (auto itr = kParamMap.begin(); itr != kParamMap.end(); ++itr) {
        if (m_parser.isSet(itr.key())) {
            m_cmds.setFlag(itr.value());
        }
    }
    if (m_parser.isSet(rootfsOpt)) {
        m_rootfs = m_parser.value(rootfsOpt);
        qDebug() << "isSet rootfsOpt, m_rootfs:" << m_rootfs;
    }
    if (m_parser.isSet(userOpt)) {
        m_user = m_parser.value(userOpt);
        qDebug() << "isSet userOpt, m_user:" << m_user;
    }

    m_listParam = m_parser.positionalArguments();
    qDebug() << "Set param end, m_listParam:" << m_listParam;
}

void InstallDebThread::getDescription(const QString &debPath)
{
    qDebug() << "Get description:" << debPath;

    // system() 存在可控命令参数注入漏洞，即使拼接也存在命令分隔符（特殊字符）机制，因此更换方式去执行命令
    QProcess process;
    process.start("dpkg", {"-e", debPath, TEMPLATE_DIR});
    process.waitForFinished(-1);

    qDebug() << "Exec finished:" << "dpkg -e " << debPath << " " << TEMPLATE_DIR;

    QFile file;
    file.setFileName(TEMPLATE_PATH);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString tmpData;
        while (!file.atEnd()) {
            tmpData = file.readLine().data();
            if (tmpData.size() > 13) {
                if (tmpData.contains("Description: ")) {
                    QString str = tmpData.mid(13, tmpData.size() - 13);
                    str.remove(QChar('\n'), Qt::CaseInsensitive);
                    m_listDescribeData << str;
                }
            }
        }

        file.close();
    } else {
        qDebug() << "Open file error:" << file.errorString();
    }
}

void InstallDebThread::onReadoutput()
{
    qDebug() << "onReadoutput";

    QString tmp = m_proc->readAllStandardOutput().data();
    qDebug() << "read output tmp:" << tmp;

    foreach (QString eachData, m_listDescribeData) {
        if (tmp.contains(eachData)) {
            char c_input[20];
            while (fgets(c_input, 10, stdin)) {
                QString str = c_input;
                str.remove(QChar('\\'), Qt::CaseInsensitive);
                str.remove(QChar('"'), Qt::CaseInsensitive);

                m_proc->write(str.toLatin1().data());

                m_proc->waitForFinished(1500);
                qDebug() << "break, finished write input:" << str;

                break;
            }
        }
    }
}

void InstallDebThread::onFinished(int num, QProcess::ExitStatus exitStatus)
{
    qDebug() << "onFinished:" << num << exitStatus;
    m_resultFlag = num;
}

void InstallDebThread::run()
{
    qDebug() << "thread enter run >>";
    if (m_listParam.isEmpty()) {
        qDebug() << "m_listParam is empty, return";
        return;
    }

    if (m_cmds.testFlag(InstallWine)) {
        qDebug() << "cmd test flag: InstallWine";   
        installWine();
    } else if (m_cmds.testFlag(Compatible)) {
        qDebug() << "cmd test flag: Compatible";
        compatibleProcess();
    } else if (m_cmds.testFlag(Immutable)) {
        qDebug() << "cmd test flag: Immutable";
        immutableProcess();
    } else if (m_cmds.testFlag(InstallConfig)) {
        qDebug() << "cmd test flag: InstallConfig";
        // InstallConfig must last, Compatible and Immutable maybe set InstallConfig too.
        installConfig();
    } else if (m_cmds.testFlag(LinglongUab)) {
        qDebug() << "cmd test flag: LinglongUab";
        uabProcessCli();
    } else {
        qDebug() << "other unknow cmd:" << m_cmds;
    }
}

/**
 * @brief Install the Wine dependency package, the incoming param is the wine package name.
 */
void InstallDebThread::installWine()
{
    qDebug() << "InstallWine";
    if (m_listParam.isEmpty()) {
        qDebug() << "m_listParam is empty, return";
        return;
    }

    // Note: Notify the front-end installation to start, don't remove it.
    qDebug() << "StartInstallDeepinwine";

    // On immutable system: --fix-missing not support, apt command will transport to deepin-immutable-ctl
    system("echo 'libpam-runtime libpam-runtime/override boolean false' | debconf-set-selections");
    system("echo 'libc6 libraries/restart-without-asking boolean true' | sudo debconf-set-selections\n");
    m_proc->setProgram("apt-get", QStringList() << "install" << m_listParam << "-y");
    m_proc->start();
    m_proc->waitForFinished(-1);
    m_proc->close();

    qDebug() << "Exec finished:" << "apt-get install " << m_listParam << "-y";
}

/**
   @brief Install the package that contains DebConf.
       Work with deepin-deb-installer to handle the configuration process of Deb packages.
 */
void InstallDebThread::installConfig()
{
    qDebug() << "InstallConfig";
    if (m_listParam.isEmpty()) {
        qDebug() << "m_listParam is empty, return";
        return;
    }

    QString debPath = m_listParam.first();
    const QFileInfo info(debPath);
    const QFile debFile(debPath);
    qDebug() << "debPath:" << debPath;

    if (debPath.contains(" ") || debPath.contains("&") || debPath.contains(";") || debPath.contains("|") ||
        debPath.contains("`")) {  // 过滤反引号,修复中危漏洞，bug 115739，处理命令连接符，命令注入导致无法软链接成功
        debPath = SymbolicLink(debPath, "installPackage");
    }

    if (debFile.exists() && info.isFile() && info.suffix().toLower() == "deb") {  // 大小写不敏感的判断是否为deb后缀
        qDebug() << "StartInstallAptConfig";

        getDescription(debPath);

        m_proc->setProgram("sudo",
                           QStringList() << "-S"
                                         << "dpkg"
                                         << "-i" << debPath);
        m_proc->start();
        m_proc->waitForFinished(-1);

        qDebug() << "Exec finished:" << "sudo dpkg -i " << debPath;

        QDir filePath(TEMPLATE_DIR);
        if (filePath.exists()) {
            qDebug() << "remove dir:" << TEMPLATE_DIR;
            filePath.removeRecursively();
        }

        m_proc->close();
        qDebug() << "EndInstallAptConfig";
    }
    qDebug() << "EndInstallConfig";
}

/**
   @brief Install / remove package in compatible mode.
 */
void InstallDebThread::compatibleProcess()
{
    qDebug() << "CompatibleProcess";
    if (m_listParam.isEmpty()) {
        qDebug() << "m_listParam is empty, return";
        return;
    }

    QStringList params;

    if (m_cmds.testFlag(Install)) {
        // only one package support
        QString debPath = m_listParam.first();
        qDebug() << "flag Install, debPath:" << debPath;

        if (debPath.contains(" ") || debPath.contains("&") || debPath.contains(";") || debPath.contains("|") ||
            debPath.contains("`")) {
            debPath = SymbolicLink(debPath, "installPackage");
        }

        if (m_cmds.testFlag(InstallConfig)) {
            qDebug() << "flag InstallConfig";
            getDescription(debPath);
        }

        // e.g.: deepin-comptabile-ctl app --json install [deb file] [--name rootfsname]
        params << kCompApp << kCompJsonFormat << kInstall << debPath;

    } else if (m_cmds.testFlag(Remove)) {
        // e.g.: deepin-compatible-ctl app --json remove [package name]
        params << kCompApp << kCompJsonFormat << kRemove << m_listParam.first();
        qDebug() << "flag Remove, package name:" << m_listParam.first();
    } else if (m_cmds.testFlag(AppCheck)) {
        // e.g.: deepin-compatible-ctl app --json check [package name]
        params << kCompApp << kCompJsonFormat << kCompCheck << m_listParam.first();
        qDebug() << "flag AppCheck, package name:" << m_listParam.first();
    } else {
        qDebug() << "return, other unknow cmd:" << m_cmds;
        return;
    }

    if (!m_rootfs.isEmpty()) {
        qDebug() << "flag Compatible, rootfs name:" << m_rootfs;
        params << kCompRootfsName << m_rootfs;
    }

    if (!m_user.isEmpty()) {
        qDebug() << "flag Compatible, current user:" << m_user;
        auto env = QProcessEnvironment::systemEnvironment();
        env.insert("SUDO_USER", m_user);
        m_proc->setProcessEnvironment(env);
    }

    m_proc->setProgram(kCompatibleBin, params);
    qInfo() << "Exec:" << qPrintable(m_proc->program().join(' '));

    m_proc->start();
    m_proc->waitForFinished(-1);
    m_proc->close();

    qDebug() << "Exec finished:" << qPrintable(m_proc->program().join(' '));

    QDir filePath(TEMPLATE_DIR);
    if (filePath.exists()) {
        qDebug() << "remove dir:" << TEMPLATE_DIR;
        filePath.removeRecursively();
    }
    qDebug() << "EndCompatibleProcess";
}

bool newImmutableVersion() {
    qDebug() << "Check new immutable version";

    QProcess process;
    process.start("dpkg-query", {"-W", "-f='${Version}'", "deepin-immutable-ctl"});
    process.waitForFinished();
    QString versionStr = process.readAllStandardOutput();

    qDebug() << "deepin-immutable-ctl version:" << versionStr;

    QVersionNumber version = QVersionNumber::fromString(versionStr.remove('\''));

    return version >= QVersionNumber(0, 0, 28);
}

/**
   @brief Install / remove package in immutable system.
 */
void InstallDebThread::immutableProcess()
{
    qDebug() << "ImmutableProcess";

    if (m_listParam.isEmpty()) {
        qDebug() << "m_listParam is empty, return";
        return;
    }
    // for immutable system
    static const QString kImmuEnvEnableWait = "DEEPIN_IMMUTABLE_CTL_WAIT_LOCK";
    static const QString kImmuYes = "-y";
    static const QString kImmuAllowDowngrades = "--allow-downgrades";

    QStringList params;

    if (m_cmds.testFlag(Install)) {
        // e.g.: deepin-compatible app install [deb file]
        // only one package support
        QString debPath = m_listParam.first();
        qDebug() << "flag Install, debPath:" << debPath;

        if (debPath.contains(" ") || debPath.contains("&") || debPath.contains(";") || debPath.contains("|") ||
            debPath.contains("`")) {
            debPath = SymbolicLink(debPath, "installPackage");
        }

        if (m_cmds.testFlag(InstallConfig)) {
            getDescription(debPath);
            qDebug() << "flag InstallConfig, get description";
        } else {
            // If current pacakge no DebConf config, disable DebConf
            m_proc->setEnv(kDebConfEnv, kDebConfDisable);
            qDebug() << "other flag, set Env";
        }

        // Note: deepin-immutable-ctl actually use apt to install/uninstall. (params transport to deepin-immutable-ctl)
        // e.g.: apt install [deb file] -y (--allow-downgrades)
        params << kInstall << debPath << kImmuYes;

        // FIXME: temporary code, check if current Immutable System need --allow-downgrades
        if (newImmutableVersion()) {
            qDebug() << "new immutable version, add --allow-downgrades";
            params << kImmuAllowDowngrades;
        }

    } else if (m_cmds.testFlag(Remove)) {
        // e.g.: apt remove [package name] -y
        params << kRemove << m_listParam.first() << kImmuYes;

        qDebug() << "flag Remove, package name:" << m_listParam.first();

        // Disable DebConf while remove package
        m_proc->setEnv(kDebConfEnv, kDebConfDisable);

    } else {
        qDebug() << "return, other unknow cmd:" << m_cmds;
        return;
    }
    qDebug() << "params:" << params;

    // enable wait another process release dpkg lock
    m_proc->setEnv(kImmuEnvEnableWait, "1");
    m_proc->setProgram(kAptBin, params);
    qInfo() << "Exec:" << qPrintable(m_proc->program().join(' '));

    m_proc->start();
    m_proc->waitForFinished(-1);
    m_proc->close();

    qDebug() << "Exec finished:" << qPrintable(m_proc->program().join(' '));

    QDir filePath(TEMPLATE_DIR);
    if (filePath.exists()) {
        qDebug() << "remove dir:" << TEMPLATE_DIR;
        filePath.removeRecursively();
    }
    qDebug() << "EndImmutableProcess";
}

/**
   @brief Install / unisntall uab package in Linglong.

   @todo Linglong's backend DBus interface is unstable, may change frequently in the near future.
        We currently choose the cli interface, use the DBus interface in the future.
 */
void InstallDebThread::uabProcessCli()
{
    qDebug() << "UabProcessCli";

    if (m_listParam.isEmpty()) {
        qDebug() << "m_listParam is empty, return";
        return;
    }
    // The Linglong params
    static const QString kUabBin = "ll-cli";
    static const QString kUabInstall = "install";
    static const QString kUabUninstall = "uninstall";
    static const QString kUabJson = "--json";
    static const QString kUabForce = "--force";  // Force install the application
    static const QString kUabPass = "-y";        // Automatically answer yes to all questions

    QStringList params;

    if (m_cmds.testFlag(Install)) {
        // e.g.: ll-cli install --json --force -y [uab file]
        params << kUabInstall << kUabJson << kUabForce << kUabPass << m_listParam.first();
        qDebug() << "flag Install, uabPath:" << m_listParam.first();
    } else if (m_cmds.testFlag(Remove)) {
        // e.g.: ll-cli uninstall --json [id/version]
        params << kUabUninstall << kUabJson << m_listParam.first();
        qDebug() << "flag Remove, package name:" << m_listParam.first();
    } else {
        qDebug() << "return, other unknow cmd:" << m_cmds;
        return;
    }

    m_proc->setProgram(kUabBin, params);
    qInfo() << "Exec:" << qPrintable(m_proc->program().join(' '));

    m_proc->start();
    m_proc->waitForFinished(-1);
    m_proc->close();

    qDebug() << "Exec finished:" << qPrintable(m_proc->program().join(' '));
}

/**
 * @brief PackagesManager::SymbolicLink 创建软连接
 * @param previousName 原始路径
 * @param packageName 软件包的包名
 * @return 软链接的路径
 */
QString InstallDebThread::SymbolicLink(const QString &previousName, const QString &packageName)
{
    qDebug() << "SymbolicLink";
    if (!mkTempDir()) {
        qWarning() << "Failed to create temporary folder, return:" << previousName;
        return previousName;
    }
    return link(previousName, packageName);
}

/**
 * @brief PackagesManager::mkTempDir 创建软链接存放的临时目录
 * @return 创建目录的结果
 */
bool InstallDebThread::mkTempDir()
{
    qDebug() << "mkTempDir";
    QDir tempPath(m_tempLinkDir);
    if (!tempPath.exists()) {
        qDebug() << "Create temporary folder:" << m_tempLinkDir;
        return tempPath.mkdir(m_tempLinkDir);
    } else {
        qDebug() << "Temporary folder already exists:" << m_tempLinkDir;
        return true;
    }
}

/**
 * @brief PackagesManager::rmTempDir 删除存放软链接的临时目录
 * @return 删除临时目录的结果
 */
bool InstallDebThread::rmTempDir()
{
    qDebug() << "rmTempDir";
    QDir tempPath(m_tempLinkDir);
    if (tempPath.exists()) {
        qDebug() << "Remove temporary folder:" << m_tempLinkDir;
        return tempPath.removeRecursively();
    } else {
        qDebug() << "Temporary folder does not exist:" << m_tempLinkDir;
        return true;
    }
}

/**
 * @brief PackagesManager::link 创建软链接
 * @param linkPath              原文件的路径
 * @param packageName           包的packageName
 * @return                      软链接之后的路径
 */
QString InstallDebThread::link(const QString &linkPath, const QString &packageName)
{
    qDebug() << "InstallDebThread: Create soft link for" << packageName;
    QFile linkDeb(linkPath);

    // 创建软链接时，如果当前临时目录中存在同名文件，即同一个名字的应用，考虑到版本可能有变化，将后续添加进入的包重命名为{packageName}_1
    // 删除后再次添加会在临时文件的后面添加_1,此问题不影响安装。如果有问题，后续再行修改。
    int count = 1;
    QString tempName = packageName;
    while (true) {
        QFile tempLinkPath(m_tempLinkDir + tempName);
        if (tempLinkPath.exists()) {
            tempName = packageName + "_" + QString::number(count);
            qWarning() << "InstallDebThread:"
                       << "A file with the same name exists in the current temporary directory,"
                          "and the current file name is changed to"
                       << tempName;
            count++;
        } else {
            qDebug() << "break, tempName:" << tempName;
            break;
        }
    }
    if (linkDeb.link(linkPath, m_tempLinkDir + tempName))
        return m_tempLinkDir + tempName;
    else {
        qWarning() << "InstallDebThread:"
                   << "Failed to create Symbolick link error.";
        return linkPath;
    }
}
