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
static const QString kParamCompatible = "Compatible";
static const QString kParamInstallConfig = "InstallConfig";
static const QString kParamInstall = "Install";
static const QString kParamRemove = "Remove";

CompatibleProcessController::CompatibleProcessController(QObject *parent)
    : QObject{parent}
{
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

    // TODO: DebConf InstallConfig

    // e.g.: pkexec deepin-deb-installer-dependsInstall Compatible InstallConfig[Optional] Install [path to deb file]
    m_type = Install;
    m_outputList.clear();
    m_currentPackage = package;
    m_process->start(
        kPkexecBin, {kInstallProcessorBin, kParamCompatible, kParamInstall, m_currentPackage->filePath()}, {}, 0, false);

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

    // e.g.: pkexec deepin-deb-installer-dependsInstall Compatible Remove [package name]
    m_type = Unintsall;
    m_outputList.clear();
    m_currentPackage = package;
    m_process->start(kPkexecBin,
                     {kInstallProcessorBin, kParamCompatible, kParamRemove, m_currentPackage->debInfo()->packageName()},
                     {},
                     0,
                     false);

    Q_EMIT processStart();
    return true;
}

bool CompatibleProcessController::ensureProcess()
{
    if (!m_process) {
        m_process = new Konsole::Pty(this);

        connect(m_process, &QProcess::readyRead, this, &CompatibleProcessController::onReadOutput);
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

void CompatibleProcessController::onReadOutput()
{
    const QByteArray output = m_process->readAllStandardOutput();
    Q_EMIT processOutput(output);

    // TODO: use `sudo apt -o Dpkg::Progress-Fancy="1" *.deb` instead

    // simulate porgress
    m_outputList.append(output);
    float progress = sqrt(m_outputList.size()) * 10;
    Q_EMIT progressChanged(progress);
}

void CompatibleProcessController::onFinished(int exitCode, int exitStatus)
{
    bool success = (CompSuccess == exitCode && QProcess::NormalExit == exitStatus);
    if (!success) {
        // up to 100 outputs
        static const int kMaxCheckLen = 100;
        for (int i = m_outputList.size() - 1; i >= qMax(0, m_outputList.size() - kMaxCheckLen); --i) {
            if (HierarchicalVerify::instance()->checkTransactionError(m_currentPackage->debInfo()->packageName(),
                                                                      m_outputList.at(i))) {
                // mark hierachical verify failed
                m_currentPackage->setError(Pkg::DigitalSignatureError, {});
            }
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
    m_currentPackage.clear();
}

};  // namespace Compatible
