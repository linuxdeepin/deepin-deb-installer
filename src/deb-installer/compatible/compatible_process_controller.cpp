// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compatible_process_controller.h"
#include "compatible_json_parser.h"
#include "utils/ddlog.h"

#include <QDebug>

#include "compatible_backend.h"
#include "process/Pty.h"
#include "utils/package_defines.h"
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

// select current user (non-root)
static const QString kEnvUser = "USER";
static const QString kParamUser = "--user";

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
    qCDebug(appLog) << "is running?";
    if (m_process && QProcess::NotRunning != m_process->state()) {
        return true;
    }

    qCDebug(appLog) << "not running";
    return false;
}

bool CompatibleProcessController::install(const Deb::DebPackage::Ptr &package)
{
    qCDebug(appLog) << "install package:";
    if (!package || !package->isValid() || isRunning()) {
        qCDebug(appLog) << "invalid package or running";
        return false;
    }
    if (!ensureProcess()) {
        qCDebug(appLog) << "ensure process failed";
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

    appendCurrentUser(params);

    m_process->start(kPkexecBin, params, {}, 0, false);

    qCInfo(appLog) << "Comaptible install:" << m_process->program();
    Q_EMIT processStart();
    return true;
}

bool CompatibleProcessController::uninstall(const Deb::DebPackage::Ptr &package)
{
    qCDebug(appLog) << "uninstall package:";
    if (!package || !package->isValid() || isRunning()) {
        qCDebug(appLog) << "invalid package or running";
        return false;
    }
    if (!ensureProcess()) {
        qCDebug(appLog) << "ensure process failed";
        return false;
    }

    // e.g.: pkexec deepin-deb-installer-dependsInstall Compatible
    //       remove --rootfs [rootfs name] [package name]
    m_type = Uninstall;
    m_outputList.clear();
    m_currentPackage = package;

    // reset state
    m_currentPackage->setError(Pkg::NoError, {});

    QStringList params{kPkexecBin,
                       kInstallProcessorBin,
                       kParamCompatible,
                       kParamRemove,
                       kParamRootfs,
                       m_currentPackage->compatible()->rootfs,
                       m_currentPackage->debInfo()->packageName()};

    appendCurrentUser(params);

    // For historical reasons, the first argument in programArguments is the
    // name of the program to execute, so create a list consisting of all
    // but the first argument to pass to setProgram()
    // sa: Pty::start()
    m_process->start(kPkexecBin, params, {}, 0, false);

    qCInfo(appLog) << "Comaptible uninstall:" << m_process->program();
    Q_EMIT processStart();
    return true;
}

bool CompatibleProcessController::needTemplates() const
{
    return (Uninstall == m_type) && m_currentPackage && m_currentPackage->containsTemplates();
}

void CompatibleProcessController::writeConfigData(const QString &configData)
{
    if (m_process) {
        m_process->pty()->write(configData.toUtf8());
        m_process->pty()->write("\n");
    }
}

bool CompatibleProcessController::ensureProcess()
{
    qCDebug(appLog) << "ensure process";
    if (!m_process) {
        m_process = new Konsole::Pty(this);

        connect(m_process, &Konsole::Pty::receivedData, this, &CompatibleProcessController::onReadOutput);
        connect(m_process,
                QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this,
                &CompatibleProcessController::onFinished);
    } else if (QProcess::NotRunning != m_process->state()) {
        qCWarning(appLog) << "Unable to restart compatible process is still running";
        return false;
    }

    qCDebug(appLog) << "ensure process success";
        
    return true;
}

void CompatibleProcessController::onReadOutput(const char *buffer, int length, bool isCommandExec)
{
    Q_UNUSED(isCommandExec);
    qCDebug(appLog) << "on read output:" << length;

    const QByteArray output = QByteArray::fromRawData(buffer, length);
    Q_EMIT processOutput(output);

    // simulate porgress, progress = log2(x) * 10
    m_outputList.append(output);

    int bitMax = 0;
    int logCount = m_outputList.size();
    while (logCount > 0) {
        logCount >>= 1;
        bitMax++;
    }

    float progress = qMin(9, bitMax) * 10;
    qCDebug(appLog) << "emit progress:" << progress;

    Q_EMIT progressChanged(progress);
}

void CompatibleProcessController::onFinished(int exitCode, int exitStatus)
{
    bool success = (Pkg::ExitNoError == exitCode && QProcess::NormalExit == exitStatus);
    qCDebug(appLog) << "process finished? " << success;

    if (!success) {
        switch (exitCode) {
            case Pkg::ExitAuthError:
                m_currentPackage->setError(Pkg::ConfigAuthCancel, {});
                qCDebug(appLog) << "auth error";
                break;
            default: {  // up to 512 outputs
                static const int kMaxCheckLen = 512;
                for (int i = m_outputList.size() - 1; i >= qMax(0, m_outputList.size() - kMaxCheckLen); --i) {
                    if (HierarchicalVerify::instance()->checkTransactionError(m_currentPackage->debInfo()->packageName(),
                                                                              m_outputList.at(i))) {
                        // mark hierachical verify failed
                        m_currentPackage->setError(Pkg::DigitalSignatureError, {});
                    }
                }
            } break;
        }
    } else {
        // New version support json data result.
        auto lastRetOutput = m_outputList.last().toUtf8();
        auto retPtr = CompatibleJsonParser::parseCommonField(lastRetOutput);
        if (retPtr && CompSuccess != retPtr->ext.code) {
            qCWarning(appLog) << "Compatible install/uninstall failed" << lastRetOutput;

            m_currentPackage->setError(Pkg::UnknownError, {});
            success = false;
        }
    }

    qCDebug(appLog) << "emit process finished:" << success;
    Q_EMIT processFinished(success);

    if (success) {
        switch (m_type) {
            case Install:
                qCDebug(appLog) << "backend do package installed";
                CompatibleBackend::instance()->packageInstalled(m_currentPackage->compatible());
                break;
            case Uninstall:
                qCDebug(appLog) << "backend do package removed";
                CompatibleBackend::instance()->packageRemoved(m_currentPackage->compatible());
                break;
            default:
                break;
        }
    }

    qCDebug(appLog) << "reset state";

    // release temp data
    m_outputList.clear();
}

void CompatibleProcessController::appendCurrentUser(QStringList &params)
{
    qCDebug(appLog) << "Compatible append current user";
    auto env = QProcessEnvironment::systemEnvironment();
    QString currentUser = env.value(kEnvUser);
    if (!currentUser.isEmpty()) {
        qCDebug(appLog) << "Compatible current user:" << currentUser;
        params << kParamUser << currentUser;
    } else {
        qCWarning(appLog) << "Compatible current user is empty";
    }
}

};  // namespace Compatible
