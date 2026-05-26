// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compatible_install_backend.h"
#include "compatible/compatible_backend.h"

#include "model/deblistmodel.h"
#include "view/pages/AptConfigMessage.h"
#include "utils/ddlog.h"
#include "utils/package_defines.h"

#ifndef DISABLE_COMPATIBLE

CompatibleInstallBackend::CompatibleInstallBackend(DebListModel *model, QObject *parent)
    : InstallBackend{model, parent}
{
}

CompatibleInstallBackend::~CompatibleInstallBackend() = default;

bool CompatibleInstallBackend::verifySignature(int index)
{
    Q_UNUSED(index)
    qCDebug(appLog) << "CompatibleInstallBackend::verifySignature";

    // Compatible mode defers signature verification to the post-install
    // check in CompatibleProcessController::onFinished() via
    // HierarchicalVerify::checkTransactionError().
    return true;
}

bool CompatibleInstallBackend::install(int index)
{
    qCDebug(appLog) << "CompatibleInstallBackend::install, index:" << index;
    ensureProcessor();

    auto pkg = m_model->packagePtr(index);
    if (!pkg) {
        qCWarning(appLog) << "No package pointer for index" << index;
        return false;
    }

    return m_compProcessor->install(pkg);
}

bool CompatibleInstallBackend::uninstall(int index)
{
    qCDebug(appLog) << "CompatibleInstallBackend::uninstall, index:" << index;
    ensureProcessor();

    auto pkg = m_model->packagePtr(index);
    if (!pkg) {
        qCWarning(appLog) << "No package pointer for index" << index;
        return false;
    }

    return m_compProcessor->uninstall(pkg);
}

bool CompatibleInstallBackend::isRunning() const
{
    return m_compProcessor && m_compProcessor->isRunning();
}

void CompatibleInstallBackend::writeConfigData(const QString &configData)
{
    if (m_compProcessor) {
        m_compProcessor->writeConfigData(configData);
    }
}

bool CompatibleInstallBackend::needTemplates() const
{
    return m_compProcessor && m_compProcessor->needTemplates();
}

void CompatibleInstallBackend::ensureProcessor()
{
    qCDebug(appLog) << "Ensuring compatible processor";
    if (!m_compProcessor) {
        m_compProcessor.reset(new Compatible::CompatibleProcessController);

        connect(
            m_compProcessor.data(),
            &Compatible::CompatibleProcessController::processOutput,
            this,
            [this](const QString &output) {
                Q_EMIT m_model->signalAppendOutputInfo(output);

                if (m_model->configWindow->isVisible()) {
                    m_model->configWindow->appendTextEdit(output);
                } else if (m_compProcessor->needTemplates()) {
                    m_model->configWindow->appendTextEdit(output);
                    m_model->configWindow->show();
                }
            });

        connect(
            m_compProcessor.data(),
            &Compatible::CompatibleProcessController::progressChanged,
            this,
            [this](float progress) {
                const int progressValue =
                    static_cast<int>((100. / m_model->preparedPackages().size())
                                    * (m_model->m_operatingIndex + progress / 100.));
                Q_EMIT m_model->signalWholeProgressChanged(progressValue);
                Q_EMIT m_model->signalCurrentPacakgeProgressChanged(static_cast<int>(progress));
            });

        connect(
            m_compProcessor.data(),
            &Compatible::CompatibleProcessController::processFinished,
            this,
            [this](bool success) {
                if (m_model->configWindow->isVisible()) {
                    m_model->configWindow->hide();
                    m_model->configWindow->clearTexts();
                }

                if (success) {
                    m_model->refreshOperatingPackageStatus(Pkg::Success);
                } else {
                    auto pkgPtr = m_compProcessor->currentPackage();
                    if (pkgPtr) {
                        m_model->m_packageFailCode.insert(m_model->m_operatingPackageMd5,
                                                          pkgPtr->errorCode());
                        m_model->m_packageFailReason.insert(m_model->m_operatingPackageMd5,
                                                           pkgPtr->errorString());

                        if (Pkg::DigitalSignatureError == pkgPtr->errorCode()) {
                            m_model->m_hierarchicalVerifyError = true;
                        }

                        if (Pkg::ConfigAuthCancel == pkgPtr->errorCode()) {
                            m_model->m_workerStatus = AbstractPackageListModel::WorkerPrepare;
                            Q_EMIT m_model->signalAuthCancel();
                            return;
                        }
                    }

                    m_model->refreshOperatingPackageStatus(Pkg::Failed);
                }

                m_model->bumpInstallIndex();
            });
    }
}

#endif  // DISABLE_COMPATIBLE
