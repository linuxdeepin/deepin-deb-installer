// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uab_process_controller.h"

#include <QRegularExpression>
#include <QProcess>
#include <QDebug>

namespace Uab {

const QString kLinglongBin = "ll-cli";
const QString kLinglongUninstall = "uninstall";

const int kInitedIndex = -1;

enum UabExitCode {
    InstallError = -1,
    InstallSuccess = 0,
};

UabProcessController::UabProcessController(QObject *parent)
    : QObject{parent}
{
}

UabProcessController::ProcFlags UabProcessController::procFlag() const
{
    return m_procFlag;
}

bool UabProcessController::isRunning() const
{
    if (m_procFlag & (Uninstalling | Installing | Processing)) {
        return true;
    }

    if (m_process && QProcess::NotRunning != m_process->state()) {
        return true;
    }

    return false;
}

bool UabProcessController::reset()
{
    if (isRunning() || !ensureProcess()) {
        return false;
    }

    m_currentIndex = kInitedIndex;
    m_procList.clear();

    return true;
}

bool Uab::UabProcessController::markInstall(const UabPkgInfo::Ptr &installPtr)
{
    if (isRunning() || !ensureProcess()) {
        return false;
    }

    m_procList.append(qMakePair(Installing, installPtr));
    return true;
}

bool Uab::UabProcessController::markUninstall(const UabPkgInfo::Ptr &uninstallPtr)
{
    if (isRunning() || !ensureProcess()) {
        return false;
    }

    m_procList.append(qMakePair(Uninstalling, uninstallPtr));
    return true;
}

bool UabProcessController::commitChanges()
{
    if (isRunning() || !ensureProcess() || m_procList.isEmpty()) {
        return false;
    }

    m_procFlag = Processing;
    m_currentIndex = kInitedIndex;
    if (!nextProcess()) {
        m_procFlag = Error;
        return false;
    }

    Q_EMIT processStart();
    return true;
}

bool Uab::UabProcessController::ensureProcess()
{
    if (!m_process) {
        m_process = new QProcess(this);

        connect(m_process, &QProcess::readyRead, this, &UabProcessController::onReadOutput);
        connect(
            m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &UabProcessController::onFinished);
    } else if (QProcess::NotRunning != m_process->state()) {
        qWarning() << "Unable to restart uab process is still running";
        return false;
    }

    return true;
}

void UabProcessController::onReadOutput()
{
    QByteArray output = m_process->readAllStandardOutput();
    Q_EMIT processOutput(output);

    // Detect progress change, filter ANSI code, receive data such as:
    // "\r\x1B[K\x1B[?25l0% prepare for installing uab\x1B[?25h"
    // "\r\x1B[K\x1B[?25l100% install uab successfully\x1B[?25h\n"
    //
    // pattern: ^\r?(\x1B\[\W?\d*\w)+(\d+)%
    static const QRegularExpression kRegExp("^\\r?(\\x1B\\[\\W?\\d*\\w)+(\\d+)%");
    QRegularExpressionMatch match = kRegExp.match(QString(output), 0, QRegularExpression::PartialPreferFirstMatch);
    if (match.hasMatch()) {
        // get (\\d+) capture
        float progress = match.captured(match.lastCapturedIndex()).toFloat();

        int count = m_procList.size();
        if (count > 1) {
            progress = (m_currentIndex * 100.0f + progress) / count;
        }

        Q_EMIT progressChanged(progress);
    }
}

void UabProcessController::onFinished(int exitCode, int exitStatus)
{
    Q_UNUSED(exitStatus)

    const bool exitSuccess = InstallSuccess == exitCode;

    // continue next process
    if (exitSuccess && nextProcess()) {
        return;
    }

    m_procFlag = Error;
    Q_EMIT processFinished(false);
}

bool UabProcessController::nextProcess()
{
    m_procFlag.setFlag(Installing, false);
    m_procFlag.setFlag(Uninstalling, false);

    // check process finish
    m_currentIndex++;
    if (m_currentIndex > m_process->size()) {
        m_procFlag = Finish;
        Q_EMIT processFinished(true);
        return true;
    }

    if (!checkIndexValid()) {
        qWarning() << qPrintable("Invalid process index") << m_currentIndex;
        return false;
    }

    auto currentProc = m_procList.value(m_currentIndex);
    switch (currentProc.first) {
        case Installing:
            return installImpl(currentProc.second);
        case Uninstalling:
            return uninstallImpl(currentProc.second);
        default:
            break;
    }

    qWarning() << qPrintable("Invalid process type") << currentProc.first;
    return false;
}

bool Uab::UabProcessController::installImpl(const Uab::UabPkgInfo::Ptr &installPtr)
{
    if (!installPtr || installPtr->filePath.isEmpty()) {
        return false;
    }

    m_procFlag.setFlag(Installing);

    m_process->setProgram(installPtr->filePath);
    m_process->start();

    return true;
}

bool Uab::UabProcessController::uninstallImpl(const Uab::UabPkgInfo::Ptr &uninstallPtr)
{
    if (!uninstallPtr || uninstallPtr->id.isEmpty() || uninstallPtr->version.isEmpty()) {
        return false;
    }

    m_procFlag.setFlag(Uninstalling);

    // e.g.: ll-cli uninstall org.deepin.package/1.0.0
    m_process->setProgram(kLinglongBin);
    m_process->setArguments({kLinglongUninstall, QString("%1/%2").arg(uninstallPtr->id).arg(uninstallPtr->version)});
    m_process->start();

    return true;
}

bool UabProcessController::checkIndexValid()
{
    return 0 <= m_currentIndex && m_currentIndex < m_procList.size();
}

}  // namespace Uab
