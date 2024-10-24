// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compatible_process_controller.h"

#include <QDebug>

#include <math.h>

#include "compatible_backend.h"
#include "process/Pty.h"
#include "utils/hierarchicalverify.h"

namespace Compatible {

// transport install/remove task to higher level permission process.
static const QString kPkexecBin = "pkexec";
static const QString kInstallProcessorBin = "deepin-deb-installer-dependsInstall";
static const QString kParamCompatible = "--install_compatible";
static const QString kParamInstallConfig = "--install_config";
static const QString kParamInstall = "--install";
static const QString kParamRemove = "--remove";
static const QString kParamRootfs = "--rootfs";

CompatibleProcessController::CompatibleProcessController(QObject *parent)
    : QObject{parent}
{
}

const Deb::DebPackage::Ptr &CompatibleProcessController::currentPackage() const
{
    return m_currentPackage;
}

bool CompatibleProcessController::isRunning() const
{
    if (m_process && QProcess::NotRunning != m_process->state()) {
        return true;
    }

    return false;
}

bool CompatibleProcessController::install(const Deb::DebPackage::Ptr &package)
{
    if (!package || !package->isValid() || isRunning()) {
        return false;
    }
    if (!ensureProcess()) {
        return false;
    }

    // e.g.: pkexec deepin-deb-installer-dependsInstall Compatible InstallConfig[Optional] \
    //       install --rootfs [rootfs name] [path to deb file]
    m_type = Install;
    m_outputList.clear();
    m_currentPackage = package;

    // For historical reasons, the first argument in programArguments is the
    // name of the program to execute, so create a list consisting of all
    // but the first argument to pass to setProgram()
    // sa: Pty::start()
    QStringList params{kPkexecBin,
                       kInstallProcessorBin,
                       kParamCompatible,
                       kParamInstall,
                       kParamRootfs,
                       m_currentPackage->compatible()->targetRootfs,
                       m_currentPackage->filePath()};
    if (m_currentPackage->containsTemplates()) {
        params << kParamInstallConfig;
    }

    m_process->start(kPkexecBin, params, {}, 0, false);

    qInfo() << "Comaptible install:" << m_process->program();
    Q_EMIT processStart();
    return true;
}

bool CompatibleProcessController::uninstall(const Deb::DebPackage::Ptr &package)
{
    if (!package || !package->isValid() || isRunning()) {
        return false;
    }
    if (!ensureProcess()) {
        return false;
    }

    // e.g.: pkexec deepin-deb-installer-dependsInstall Compatible \
    //       remove --rootfs [rootfs name] [package name]
    m_type = Unintsall;
    m_outputList.clear();
    m_currentPackage = package;

    // For historical reasons, the first argument in programArguments is the
    // name of the program to execute, so create a list consisting of all
    // but the first argument to pass to setProgram()
    // sa: Pty::start()
    m_process->start(kPkexecBin,
                     {kPkexecBin,
                      kInstallProcessorBin,
                      kParamCompatible,
                      kParamRemove,
                      kParamRootfs,
                      m_currentPackage->compatible()->rootfs,
                      m_currentPackage->debInfo()->packageName()},
                     {},
                     0,
                     false);

    qInfo() << "Comaptible uninstall:" << m_process->program();
    Q_EMIT processStart();
    return true;
}

bool CompatibleProcessController::ensureProcess()
{
    if (!m_process) {
        m_process = new Konsole::Pty(this);

        connect(m_process, &Konsole::Pty::receivedData, this, &CompatibleProcessController::onReadOutput);
        connect(m_process,
                QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this,
                &CompatibleProcessController::onFinished);
    } else if (QProcess::NotRunning != m_process->state()) {
        qWarning() << "Unable to restart compatible process is still running";
        return false;
    }

    return true;
}

void CompatibleProcessController::onReadOutput(const char *buffer, int length, bool isCommandExec)
{
    Q_UNUSED(isCommandExec);

    const QByteArray output = QByteArray::fromRawData(buffer, length);
    Q_EMIT processOutput(output);

    // TODO: use `sudo apt -o Dpkg::Progress-Fancy="1" *.deb` instead

    // simulate porgress, progress = log2(x) * 10
    m_outputList.append(output);

    int bitCount = 0;
    int logCount = m_outputList.size();
    while (logCount > 0) {
        logCount >>= 1;
        bitCount++;
    }

    float progress = bitCount * 10;
    Q_EMIT progressChanged(progress);
}

void CompatibleProcessController::onFinished(int exitCode, int exitStatus)
{
    const bool success = (CompSuccess == exitCode && QProcess::NormalExit == exitStatus);
    if (!success) {
        switch (exitCode) {
            case AuthError:
                m_currentPackage->setError(Pkg::ConfigAuthCancel, {});
                break;
            default: {  // up to 100 outputs
                static const int kMaxCheckLen = 100;
                for (int i = m_outputList.size() - 1; i >= qMax(0, m_outputList.size() - kMaxCheckLen); --i) {
                    if (HierarchicalVerify::instance()->checkTransactionError(m_currentPackage->debInfo()->packageName(),
                                                                              m_outputList.at(i))) {
                        // mark hierachical verify failed
                        m_currentPackage->setError(Pkg::DigitalSignatureError, {});
                    }
                }
            } break;
        }
    }

    Q_EMIT processFinished(success);

    if (success) {
        switch (m_type) {
            case Install:
                CompatibleBackend::instance()->packageInstalled(m_currentPackage->compatible());
                break;
            case Unintsall:
                CompatibleBackend::instance()->packageRemoved(m_currentPackage->compatible());
                break;
            default:
                break;
        }
    }

    // release temp data
    m_outputList.clear();
}

};  // namespace Compatible
