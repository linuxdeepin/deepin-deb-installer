// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "immutable_process_controller.h"

#include <QDebug>
#include "utils/ddlog.h"

#include "process/Pty.h"
#include "utils/hierarchicalverify.h"
#include "utils/package_defines.h"

namespace Immutable {

static const QString kPkexecBin = "pkexec";
static const QString kInstallProcessorBin = "deepin-deb-installer-dependsInstall";
static const QString kParamImmutable = "--install_immutable";
static const QString kParamInstallConfig = "--install_config";
static const QString kParamInstall = "--install";
static const QString kParamRemove = "--remove";

ImmutableProcessController::ImmutableProcessController(QObject *parent)
    : QObject{parent}
{
}

const Deb::DebPackage::Ptr &ImmutableProcessController::currentPackage() const
{
    return m_currentPackage;
}

bool ImmutableProcessController::isRunning() const
{
    qCDebug(appLog) << "immutable process is running";
    if (m_process && QProcess::NotRunning != m_process->state()) {
        return true;
    }

    qCDebug(appLog) << "immutable process is not running";
    return false;
}

bool ImmutableProcessController::install(const Deb::DebPackage::Ptr &package)
{
    qCDebug(appLog) << "install immutable package";
    if (!package || !package->isValid() || isRunning()) {
        qCDebug(appLog) << "invalid package or process is running";
        return false;
    }
    if (!ensureProcess()) {
        qCDebug(appLog) << "failed to ensure process";
        return false;
    }

    // e.g.: pkexec deepin-deb-installer-dependsInstall Compatible InstallConfig[Optional]
    //       install --rootfs [rootfs name] [path to deb file]
    m_type = Install;
    m_outputList.clear();
    m_currentPackage = package;

    // reset state
    m_currentPackage->setError(Pkg::NoError, {});

    // For historical reasons, the first argument in programArguments is the
    // name of the program to execute, so create a list consisting of all
    // but the first argument to pass to setProgram()
    // sa: Pty::start()
    QStringList params{kPkexecBin, kInstallProcessorBin, kParamImmutable, kParamInstall, m_currentPackage->filePath()};
    if (m_currentPackage->containsTemplates()) {
        qCDebug(appLog) << "install config templates";
        params << kParamInstallConfig;
    }

    m_process->start(kPkexecBin, params, {}, 0, false);

    qCInfo(appLog) << "Comaptible install:" << m_process->program();
    Q_EMIT processStart();
    return true;
}

bool ImmutableProcessController::uninstall(const Deb::DebPackage::Ptr &package)
{
    qCDebug(appLog) << "uninstall immutable package";
    if (!package || !package->isValid() || isRunning()) {
        qCDebug(appLog) << "invalid package or process is running";
        return false;
    }
    if (!ensureProcess()) {
        qCDebug(appLog) << "failed to ensure process";
        return false;
    }

    // e.g.: pkexec deepin-deb-installer-dependsInstall Compatible
    //       remove --rootfs [rootfs name] [package name]
    m_type = Unintsall;
    m_outputList.clear();
    m_currentPackage = package;

    // reset state
    m_currentPackage->setError(Pkg::NoError, {});

    // For historical reasons, the first argument in programArguments is the
    // name of the program to execute, so create a list consisting of all
    // but the first argument to pass to setProgram()
    // sa: Pty::start()
    m_process->start(
        kPkexecBin,
        {kPkexecBin, kInstallProcessorBin, kParamImmutable, kParamRemove, m_currentPackage->debInfo()->packageName()},
        {},
        0,
        false);

    qCInfo(appLog) << "Comaptible uninstall:" << m_process->program();
    Q_EMIT processStart();
    return true;
}

bool ImmutableProcessController::needTemplates() const
{
    // Unisntallation not need config templates.
    return (Unintsall != m_type) && m_currentPackage && m_currentPackage->containsTemplates();
}

void ImmutableProcessController::writeConfigData(const QString &configData)
{
    qCDebug(appLog) << "write config data to immutable process";
    if (m_process) {
        m_process->pty()->write(configData.toUtf8());
        m_process->pty()->write("\n");
        qCDebug(appLog) << "writed config data:" << configData;
    }
    qCDebug(appLog) << "write config data to immutable process finished";
}

bool ImmutableProcessController::ensureProcess()
{
    qCDebug(appLog) << "ensure immutable process";
    if (!m_process) {
        qCDebug(appLog) << "create immutable process";
        m_process = new Konsole::Pty(this);

        connect(m_process, &Konsole::Pty::receivedData, this, &ImmutableProcessController::onReadOutput);
        connect(m_process,
                QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this,
                &ImmutableProcessController::onFinished);
    } else if (QProcess::NotRunning != m_process->state()) {
        qCWarning(appLog) << "Unable to restart compatible process is still running";
        return false;
    }

    qCDebug(appLog) << "ensure immutable process finished";
    return true;
}

void ImmutableProcessController::onReadOutput(const char *buffer, int length, bool isCommandExec)
{
    Q_UNUSED(isCommandExec);

    const QByteArray output = QByteArray::fromRawData(buffer, length);
    qCDebug(appLog) << "read output from immutable process:" << output;
    Q_EMIT processOutput(output);

    // simulate progress, progress = floor(log2(x)) * 10
    m_outputList.append(output);

    int bitMax = 0;
    int logCount = m_outputList.size();
    while (logCount > 0) {
        logCount >>= 1;
        bitMax++;
    }

    float progress = qMin(9, bitMax) * 10;
    qCDebug(appLog) << "progress:" << progress;
    Q_EMIT progressChanged(progress);
}

void ImmutableProcessController::onFinished(int exitCode, int exitStatus)
{
    const bool success = (Pkg::ExitNoError == exitCode && QProcess::NormalExit == exitStatus);
    qCDebug(appLog) << "immutable process finished, success:" << success << "exit code:" << exitCode << "exit status:" << exitStatus;
    if (!success) {
        switch (exitCode) {
            case Pkg::ExitAuthError:
                m_currentPackage->setError(Pkg::ConfigAuthCancel, {});
                qCDebug(appLog) << "config auth cancel";
                break;
            default: {  // up to 512 outputs
                static const int kMaxCheckLen = 512;
                for (int i = m_outputList.size() - 1; i >= qMax(0, m_outputList.size() - kMaxCheckLen); --i) {
                    if (HierarchicalVerify::instance()->checkTransactionError(m_currentPackage->debInfo()->packageName(),
                                                                              m_outputList.at(i))) {
                        // mark hierachical verify failed
                        m_currentPackage->setError(Pkg::DigitalSignatureError, {});
                        qCDebug(appLog) << "digital signature error";
                    }
                }
            } break;
        }
    }

    Q_EMIT processFinished(success);
    qCDebug(appLog) << "immutable process finished";

    // release temp data
    m_outputList.clear();
}

};  // namespace Immutable
