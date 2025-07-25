/*
 * This file is a part of QTerminal - http://gitorious.org/qterminal
 *
 * This file was un-linked from KDE and modified
 * by Maxim Bourmistrov <maxim@unixconn.com>
 *
 */

/*
    This file is part of the KDE libraries

    Copyright (C) 2007 Oswald Buddenhagen <ossi@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kprocess.h"

#include <qfile.h>
#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(procLog, "org.deepin.deb-installer.process")

#ifdef Q_OS_WIN
# include <windows.h>
#else
# include <unistd.h>
# include <cerrno>
#endif

#ifndef Q_OS_WIN
# define STD_OUTPUT_HANDLE 1
# define STD_ERROR_HANDLE 2
#endif

#ifdef _WIN32_WCE
#include <stdio.h>
#endif

void KProcessPrivate::writeAll(const QByteArray &buf, int fd)
{
    // qCDebug(procLog) << "KProcessPrivate::writeAll called with fd:" << fd << "size:" << buf.size();
#ifdef Q_OS_WIN
#ifndef _WIN32_WCE
    HANDLE h = GetStdHandle(fd);
    if (h) {
        // qCDebug(procLog) << "Got valid handle, writing to file";
        DWORD wr;
        WriteFile(h, buf.data(), buf.size(), &wr, 0);
    }
#else
    qCDebug(procLog) << "Writing to file using fwrite for WIN32_WCE";
    fwrite(buf.data(), 1, buf.size(), (FILE *)fd);
#endif
#else
    int off = 0;
    do {
        int ret = ::write(fd, buf.data() + off, buf.size() - off);
        if (ret < 0) {
            if (errno != EINTR) {
                qCWarning(procLog) << "Write error, errno:" << errno;
                return;
            }
            // qCDebug(procLog) << "Write interrupted, retrying";
        } else {
            off += ret;
        }
    } while (off < buf.size());
    // qCDebug(procLog) << "Wrote" << off << "bytes";
#endif
}

void KProcessPrivate::forwardStd(KProcess::ProcessChannel good, int fd)
{
    // qCDebug(procLog) << "KProcessPrivate::forwardStd called with channel:" << good << "fd:" << fd;
    Q_Q(KProcess);

    QProcess::ProcessChannel oc = q->readChannel();
    q->setReadChannel(good);
    writeAll(q->readAll(), fd);
    q->setReadChannel(oc);
}

void KProcessPrivate::_k_forwardStdout()
{
#ifndef _WIN32_WCE
    forwardStd(KProcess::StandardOutput, STD_OUTPUT_HANDLE);
#else
    forwardStd(KProcess::StandardOutput, (int)stdout);
#endif
}

void KProcessPrivate::_k_forwardStderr()
{
#ifndef _WIN32_WCE
    forwardStd(KProcess::StandardError, STD_ERROR_HANDLE);
#else
    forwardStd(KProcess::StandardError, (int)stderr);
#endif
}

/////////////////////////////
// public member functions //
/////////////////////////////

KProcess::KProcess(QObject *parent) :
    QProcess(parent),
    d_ptr(new KProcessPrivate)
{
    qCDebug(procLog) << "KProcess constructor with parent object";
    d_ptr->q_ptr = this;
    setOutputChannelMode(ForwardedChannels);
}

KProcess::KProcess(KProcessPrivate *d, QObject *parent) :
    QProcess(parent),
    d_ptr(d)
{
    qCDebug(procLog) << "KProcess constructor with private data";
    d_ptr->q_ptr = this;
    setOutputChannelMode(ForwardedChannels);
}

KProcess::~KProcess()
{
    qCDebug(procLog) << "KProcess destructor";
    delete d_ptr;
}

void KProcess::setOutputChannelMode(OutputChannelMode mode)
{
    qCDebug(procLog) << "Setting output channel mode to:" << mode;
    Q_D(KProcess);

    d->outputChannelMode = mode;
    disconnect(this, SIGNAL(readyReadStandardOutput()));
    disconnect(this, SIGNAL(readyReadStandardError()));
    switch (mode) {
    case OnlyStdoutChannel:
        qCDebug(procLog) << "Mode is OnlyStdoutChannel, forwarding stderr";
        connect(this, SIGNAL(readyReadStandardError()), SLOT(_k_forwardStderr()));
        break;
    case OnlyStderrChannel:
        qCDebug(procLog) << "Mode is OnlyStderrChannel, forwarding stdout";
        connect(this, SIGNAL(readyReadStandardOutput()), SLOT(_k_forwardStdout()));
        break;
    default:
        qCDebug(procLog) << "Mode is" << mode << ", setting process channel mode directly";
        QProcess::setProcessChannelMode((ProcessChannelMode)mode);
        return;
    }
    qCDebug(procLog) << "Setting process channel mode to SeparateChannels";
    QProcess::setProcessChannelMode(QProcess::SeparateChannels);
}

KProcess::OutputChannelMode KProcess::outputChannelMode() const
{
    qCDebug(procLog) << "Getting output channel mode";
    Q_D(const KProcess);

    return d->outputChannelMode;
}

void KProcess::setNextOpenMode(QIODevice::OpenMode mode)
{
    qCDebug(procLog) << "Setting next open mode to:" << mode;
    Q_D(KProcess);

    d->openMode = mode;
}

#define DUMMYENV "_KPROCESS_DUMMY_="

void KProcess::clearEnvironment()
{
    qCDebug(procLog) << "Clearing environment";
    setEnvironment(QStringList() << QString::fromLatin1(DUMMYENV));
}

void KProcess::setEnv(const QString &name, const QString &value, bool overwrite)
{
    qCDebug(procLog) << "Setting environment:" << name << "=" << value << "overwrite:" << overwrite;
    QStringList env = environment();
    if (env.isEmpty()) {
        env = systemEnvironment();
        env.removeAll(QString::fromLatin1(DUMMYENV));
    }
    QString fname(name);
    fname.append(QLatin1Char('='));
    for (QStringList::Iterator it = env.begin(); it != env.end(); ++it)
        if ((*it).startsWith(fname)) {
            if (overwrite) {
                *it = fname.append(value);
                setEnvironment(env);
            }
            return;
        }
    env.append(fname.append(value));
    setEnvironment(env);
}

void KProcess::unsetEnv(const QString &name)
{
    qCDebug(procLog) << "Unsetting environment:" << name;
    QStringList env = environment();
    if (env.isEmpty()) {
        env = systemEnvironment();
        env.removeAll(QString::fromLatin1(DUMMYENV));
    }
    QString fname(name);
    fname.append(QLatin1Char('='));
    for (QStringList::Iterator it = env.begin(); it != env.end(); ++it)
        if ((*it).startsWith(fname)) {
            env.erase(it);
            if (env.isEmpty())
                env.append(QString::fromLatin1(DUMMYENV));
            setEnvironment(env);
            return;
        }
}

void KProcess::setProgram(const QString &exe, const QStringList &args)
{
    qCDebug(procLog) << "Setting program:" << exe << "with args:" << args;
    Q_D(KProcess);

    d->prog = exe;
    d->args = args;
#ifdef Q_OS_WIN
    setNativeArguments(QString());
#endif
}

void KProcess::setProgram(const QStringList &argv)
{
    qCDebug(procLog) << "Setting program with argv:" << argv;
    Q_D(KProcess);

    Q_ASSERT(!argv.isEmpty());
    d->args = argv;
    d->prog = d->args.takeFirst();
#ifdef Q_OS_WIN
    setNativeArguments(QString());
#endif
}

KProcess &KProcess::operator<<(const QString &arg)
{
    // qCDebug(procLog) << "Appending argument:" << arg;
    Q_D(KProcess);

    if (d->prog.isEmpty())
        d->prog = arg;
    else
        d->args << arg;
    return *this;
}

KProcess &KProcess::operator<<(const QStringList &args)
{
    // qCDebug(procLog) << "Appending arguments:" << args;
    Q_D(KProcess);

    if (d->prog.isEmpty())
        setProgram(args);
    else
        d->args << args;
    return *this;
}

void KProcess::clearProgram()
{
    qCDebug(procLog) << "Clearing program";
    Q_D(KProcess);

    d->prog.clear();
    d->args.clear();
#ifdef Q_OS_WIN
    setNativeArguments(QString());
#endif
}

#if 0
void KProcess::setShellCommand(const QString &cmd)
{
    qCDebug(procLog) << "Setting shell command:" << cmd;
    Q_D(KProcess);

    KShell::Errors err;
    d->args = KShell::splitArgs(
                  cmd, KShell::AbortOnMeta | KShell::TildeExpand, &err);
    if (err == KShell::NoError && !d->args.isEmpty()) {
        d->prog = KStandardDirs::findExe(d->args[0]);
        if (!d->prog.isEmpty()) {
            d->args.removeFirst();
#ifdef Q_OS_WIN
            setNativeArguments(QString());
#endif
            return;
        }
    }

    d->args.clear();

#ifdef Q_OS_UNIX
// #ifdef NON_FREE // ... as they ship non-POSIX /bin/sh
# if !defined(__linux__) && !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__DragonFly__) && !defined(__GNU__)
    // If /bin/sh is a symlink, we can be pretty sure that it points to a
    // POSIX shell - the original bourne shell is about the only non-POSIX
    // shell still in use and it is always installed natively as /bin/sh.
    d->prog = QFile::symLinkTarget(QString::fromLatin1("/bin/sh"));
    if (d->prog.isEmpty()) {
        // Try some known POSIX shells.
        d->prog = KStandardDirs::findExe(QString::fromLatin1("ksh"));
        if (d->prog.isEmpty()) {
            d->prog = KStandardDirs::findExe(QString::fromLatin1("ash"));
            if (d->prog.isEmpty()) {
                d->prog = KStandardDirs::findExe(QString::fromLatin1("bash"));
                if (d->prog.isEmpty()) {
                    d->prog = KStandardDirs::findExe(QString::fromLatin1("zsh"));
                    if (d->prog.isEmpty())
                        // We're pretty much screwed, to be honest ...
                        d->prog = QString::fromLatin1("/bin/sh");
                }
            }
        }
    }
# else
    d->prog = QString::fromLatin1("/bin/sh");
# endif

    d->args << QString::fromLatin1("-c") << cmd;
#else // Q_OS_UNIX
    // KMacroExpander::expandMacrosShellQuote(), KShell::quoteArg() and
    // KShell::joinArgs() may generate these for security reasons.
    setEnv(PERCENT_VARIABLE, QLatin1String("%"));

#ifndef _WIN32_WCE
    WCHAR sysdir[MAX_PATH + 1];
    UINT size = GetSystemDirectoryW(sysdir, MAX_PATH + 1);
    d->prog = QString::fromUtf16((const ushort *) sysdir, size);
    d->prog += QLatin1String("\\cmd.exe");
    setNativeArguments(QLatin1String("/V:OFF /S /C \"") + cmd + QLatin1Char('"'));
#else
    d->prog = QLatin1String("\\windows\\cmd.exe");
    setNativeArguments(QLatin1String("/S /C \"") + cmd + QLatin1Char('"'));
#endif
#endif
}
#endif
QStringList KProcess::program() const
{
    Q_D(const KProcess);

    QStringList argv = d->args;
    argv.prepend(d->prog);
    return argv;
}

void KProcess::start()
{
    Q_D(KProcess);

    qCDebug(procLog) << "Starting process:" << d->prog << d->args;
    QProcess::start(d->prog, d->args, d->openMode);
}

int KProcess::execute(int msecs)
{
    start();
    if (!waitForFinished(msecs)) {
        kill();
        waitForFinished(-1);
        qCWarning(procLog) << "Process timed out, killing process, return -2";
        return -2;
    }
    return (exitStatus() == QProcess::NormalExit) ? exitCode() : -1;
}

// static
int KProcess::execute(const QString &exe, const QStringList &args, int msecs)
{
    qCDebug(procLog) << "Executing process:" << exe << "with args:" << args << "with msecs:" << msecs;
    KProcess p;
    p.setProgram(exe, args);
    return p.execute(msecs);
}

// static
int KProcess::execute(const QStringList &argv, int msecs)
{
    qCDebug(procLog) << "Executing process with argv:" << argv << "with msecs:" << msecs;
    KProcess p;
    p.setProgram(argv);
    return p.execute(msecs);
}

int KProcess::startDetached()
{
    qCDebug(procLog) << "Starting detached process";
    Q_D(KProcess);

    qint64 pid;
    if (!QProcess::startDetached(d->prog, d->args, workingDirectory(), &pid))
        return 0;
    return (int) pid;
}

// static
int KProcess::startDetached(const QString &exe, const QStringList &args)
{
    qCDebug(procLog) << "Starting detached process with exe:" << exe << "and args:" << args;
    qint64 pid;
    if (!QProcess::startDetached(exe, args, QString(), &pid))
        return 0;
    return (int) pid;
}

// static
int KProcess::startDetached(const QStringList &argv)
{
    qCDebug(procLog) << "Starting detached process with argv:" << argv;
    QStringList args = argv;
    QString prog = args.takeFirst();
    return startDetached(prog, args);
}

int KProcess::pid() const
{
    qCDebug(procLog) << "Getting process id";
#ifdef Q_OS_UNIX
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return (int) QProcess::pid();
#else
    return QProcess::processId();
#endif
#else
    return QProcess::pid() ? QProcess::pid()->dwProcessId : 0;
#endif
}

