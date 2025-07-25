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


#include "kptyprocess.h"
#include "kprocess.h"
#include "kptydevice.h"

#include <cstdlib>
#include <unistd.h>
#include <csignal>
#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(procLog)

KPtyProcess::KPtyProcess(QObject *parent) :
    KProcess(new KPtyProcessPrivate, parent)
{
    qCDebug(procLog) << "KPtyProcess constructed";
    Q_D(KPtyProcess);

    d->pty = new KPtyDevice(this);
    d->pty->open();
    connect(this, SIGNAL(stateChanged(QProcess::ProcessState)),
            SLOT(_k_onStateChanged(QProcess::ProcessState)));

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    setupChildProcess();
#else
    // use setChildProcessModifier() replace setupChildProcess()
    setChildProcessModifier([this](){
        setupChildProcessImpl();
    });
#endif
}

KPtyProcess::KPtyProcess(int ptyMasterFd, QObject *parent) :
    KProcess(new KPtyProcessPrivate, parent)
{
    qCDebug(procLog) << "KPtyProcess constructed with master fd" << ptyMasterFd;
    Q_D(KPtyProcess);

    d->pty = new KPtyDevice(this);
    d->pty->open(ptyMasterFd);
    connect(this, SIGNAL(stateChanged(QProcess::ProcessState)),
            SLOT(_k_onStateChanged(QProcess::ProcessState)));

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    setupChildProcess();
#else
    // use setChildProcessModifier() replace setupChildProcess()
    setChildProcessModifier([this](){
        setupChildProcessImpl();
    });
#endif
}

KPtyProcess::~KPtyProcess()
{
    qCDebug(procLog) << "KPtyProcess destructed";
    Q_D(KPtyProcess);

    qCDebug(procLog) << "Process state before cleanup:" << state();
    if (state() != QProcess::NotRunning)
    {
        if (d->addUtmp)
        {
            qCDebug(procLog) << "Logging out from pty";
            d->pty->logout();
            disconnect(SIGNAL(stateChanged(QProcess::ProcessState)),
                    this, SLOT(_k_onStateChanged(QProcess::ProcessState)));
        }
    }
    delete d->pty;
    waitForFinished(300); // give it some time to finish
    if (state() != QProcess::NotRunning)
    {
        qCWarning(procLog) << Q_FUNC_INFO << "the terminal process is still running, trying to stop it by SIGHUP";
        ::kill(pid(), SIGHUP);
        waitForFinished(300);
        if (state() != QProcess::NotRunning)
            qCCritical(procLog) << Q_FUNC_INFO << "process didn't stop upon SIGHUP and will be SIGKILL-ed";
    }
}

void KPtyProcess::setPtyChannels(PtyChannels channels)
{
    Q_D(KPtyProcess);

    qCDebug(procLog) << "Setting PTY channels:" << channels;
    d->ptyChannels = channels;
}

KPtyProcess::PtyChannels KPtyProcess::ptyChannels() const
{
    qCDebug(procLog) << "ptyChannels";
    Q_D(const KPtyProcess);

    return d->ptyChannels;
}

void KPtyProcess::setUseUtmp(bool value)
{
    Q_D(KPtyProcess);

    qCDebug(procLog) << "Setting use utmp to:" << value;
    d->addUtmp = value;
}

bool KPtyProcess::isUseUtmp() const
{
    qCDebug(procLog) << "isUseUtmp";
    Q_D(const KPtyProcess);

    return d->addUtmp;
}

KPtyDevice *KPtyProcess::pty() const
{
    qCDebug(procLog) << "pty";
    Q_D(const KPtyProcess);

    return d->pty;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void KPtyProcess::setupChildProcess()
#else
void KPtyProcess::setupChildProcessImpl()
#endif
{
    Q_D(KPtyProcess);
    qCDebug(procLog) << "Setting up child process PTY channels:" << d->ptyChannels;

    d->pty->setCTty();

#if 0
    if (d->addUtmp)
        d->pty->login(KUser(KUser::UseRealUserID).loginName().toLocal8Bit().data(), qgetenv("DISPLAY"));
#endif
    if (d->ptyChannels & StdinChannel)
        dup2(d->pty->slaveFd(), 0);

    if (d->ptyChannels & StdoutChannel)
        dup2(d->pty->slaveFd(), 1);

    if (d->ptyChannels & StderrChannel)
        dup2(d->pty->slaveFd(), 2);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    KProcess::setupChildProcess();
#endif
}

//#include "kptyprocess.moc"
