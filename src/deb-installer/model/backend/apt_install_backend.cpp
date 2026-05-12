// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "apt_install_backend.h"

#include "model/deblistmodel.h"
#include "manager/packagesmanager.h"
#include "manager/PackageDependsStatus.h"
#include "model/packageanalyzer.h"
#include "utils/ddlog.h"
#include "utils/utils.h"
#include "utils/hierarchicalverify.h"
#include "utils/package_defines.h"
#include "view/pages/AptConfigMessage.h"
#include "view/pages/settingdialog.h"
#include "singleInstallerApplication.h"

#include <QApt/Backend>
#include <QApt/DebFile>
#include <QApt/Transaction>

#include <QApplication>
#include <QTimer>

using namespace QApt;

AptInstallBackend::AptInstallBackend(DebListModel *model, QObject *parent)
    : InstallBackend{model, parent}
{
}

AptInstallBackend::~AptInstallBackend() = default;

bool AptInstallBackend::verifySignature(int index)
{
    Q_UNUSED(index)
    qCDebug(appLog) << "AptInstallBackend::verifySignature entering";

    // 分级管控可用时，交由分级管控进行签名验证
    if (HierarchicalVerify::instance()->isValid()) {
        qCDebug(appLog) << "Hierarchical verification is valid, skipping signature check";
        return true;
    }

    const auto stat = m_model->m_packagesManager->getPackageDependsStatus(m_model->m_operatingIndex);
    if (stat.isBreak() || stat.isAuthCancel()) {
        qCDebug(appLog) << "Dependency is broken or auth cancelled, skipping signature check";
        return true;
    }
    SettingDialog dialog;
    m_model->m_isDigitalVerify = dialog.isDigitalVerified();
    int digitalSigntual = Utils::Digital_Verify(m_model->m_packagesManager->package(m_model->m_operatingIndex));
    qCInfo(appLog) << "m_isDevelopMode:" << m_model->m_isDevelopMode << " /m_isDigitalVerify:" << m_model->m_isDigitalVerify
                   << " /digitalSigntual:" << digitalSigntual;
    if (m_model->m_isDevelopMode && !m_model->m_isDigitalVerify) {
        qCDebug(appLog) << "Developer mode and digital verification is disabled, returning true";
        return true;
    } else if (m_model->m_isDevelopMode && m_model->m_isDigitalVerify) {
        qCDebug(appLog) << "Developer mode and digital verification is enabled";
        if (digitalSigntual == Utils::VerifySuccess) {
            qCDebug(appLog) << "Signature verification successful in developer mode";
            return true;
        } else {
            qCDebug(appLog) << "Signature verification failed in developer mode";
            Pkg::ErrorCode code;
            if (digitalSigntual == Utils::DebfileInexistence) {
                qCDebug(appLog) << "Deb file inexistence, setting code to NoDigitalSignature";
                code = Pkg::NoDigitalSignature;
            } else {
                qCDebug(appLog) << "Digital signature error, setting code to DigitalSignatureError";
                code = Pkg::DigitalSignatureError;
            }
            m_model->showDevelopDigitalErrWindow(code);
            return false;
        }
    } else {
        qCDebug(appLog) << "Not in developer mode, checking signature status";
        bool verifiedResult = false;
        switch (digitalSigntual) {
            case Utils::VerifySuccess:
                qCDebug(appLog) << "Signature verification successful";
                verifiedResult = true;
                break;
            case Utils::DebfileInexistence:
                qCDebug(appLog) << "Deb file inexistence, showing no digital signature window";
                m_model->showNoDigitalErrWindow();
                verifiedResult = false;
                break;
            case Utils::ExtractDebFail:
                qCDebug(appLog) << "Failed to extract deb, showing digital error window";
                m_model->showDigitalErrWindow();
                verifiedResult = false;
                break;
            case Utils::DebVerifyFail:
            case Utils::OtherError:
                qCDebug(appLog) << "Deb verification failed or other error, showing digital error window";
                m_model->showDigitalErrWindow();
                verifiedResult = false;
                break;
            default:
                qCInfo(appLog) << "unknown mistake";
                qCDebug(appLog) << "Unknown signature verification error";
                verifiedResult = false;
                break;
        }
        return verifiedResult;
    }
}

bool AptInstallBackend::install(int index)
{
    Q_UNUSED(index)
    qCDebug(appLog) << "AptInstallBackend::install entering for operating index" << m_model->m_operatingIndex;

    DebFile deb(m_model->m_packagesManager->package(m_model->m_operatingIndex));
    if (!deb.isValid()) {
        qCDebug(appLog) << "Deb file is invalid, returning false";
        return false;
    }
    qCInfo(appLog)
        << QString("Prepare to install %1, ver: %2, arch: %3").arg(deb.packageName()).arg(deb.version()).arg(deb.architecture());

    Q_ASSERT_X(m_model->m_workerStatus == AbstractPackageListModel::WorkerProcessing, Q_FUNC_INFO, "installer status error");
    Q_ASSERT_X(m_currentTransaction.isNull(), Q_FUNC_INFO, "previous transaction not finished");

    // Emit signal before checking dpkg, same as original
    qCDebug(appLog) << "Emitting signalWorkerStart";
    Q_EMIT m_model->signalWorkerStart();

    auto *const backend = PackageAnalyzer::instance().backendPtr();
    if (!backend) {
        qCDebug(appLog) << "Backend pointer is null, returning false";
        return false;
    }

    Transaction *transaction = nullptr;

    const auto dependsStat = m_model->m_packagesManager->getPackageDependsStatus(m_model->m_operatingStatusIndex);

    // Dependencies not satisfiable
    if (!dependsStat.canInstall()) {
        qCDebug(appLog) << "Dependencies cannot be satisfied";
        m_model->refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Failed);

        m_model->m_packageFailCode.insert(m_model->m_operatingPackageMd5, -1);
        m_model->m_packageFailReason.insert(m_model->m_operatingPackageMd5,
                                            m_model->packageFailedReason(m_model->m_operatingStatusIndex));

        qCDebug(appLog) << "Bumping install index due to unsatisfied dependencies";
        m_model->bumpInstallIndex();
        return true;
    }

    if (dependsStat.isAvailable()) {
        // Dependencies available, need to download and install deps first
        qCDebug(appLog) << "Dependencies are available";
        if (DebListModel::isDpkgRunning()) {
            qCInfo(appLog) << "AptInstallBackend: dpkg running, waiting...";
            qCDebug(appLog) << "Dpkg is running, waiting before next install check";
            QTimer::singleShot(1000, m_model, [this]() { m_model->installNextDeb(); });
            return true;
        }

        Q_ASSERT_X(m_model->m_packageOperateStatus[m_model->m_operatingPackageMd5],
                   Q_FUNC_INFO,
                   "package operate status error when start install available dependencies");

        const QStringList availableDepends = m_model->m_packagesManager->packageAvailableDepends(m_model->m_operatingIndex);
        qCInfo(appLog) << QString("Prepare install package: %1 , install depends: ").arg(deb.packageName()) << availableDepends;

        for (auto const &p : availableDepends) {
            if (p.contains(" not found")) {
                qCDebug(appLog) << "Dependency not found:" << p;
                m_model->refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Failed);
                m_model->m_packageFailCode.insert(m_model->m_operatingPackageMd5, DownloadDisallowedError);
                m_model->m_packageFailReason.insert(m_model->m_operatingPackageMd5, p);
                Q_EMIT m_model->signalAppendOutputInfo(m_model->m_packagesManager->package(m_model->m_operatingIndex) + "'s depend " + " " +
                                                        p);
                m_model->bumpInstallIndex();

                qWarning() << QString("Package %1 install failed, not found depend package: %2").arg(deb.packageName()).arg(p);
                return true;
            }
            qCDebug(appLog) << "Marking dependency for installation:" << p;
            backend->markPackageForInstall(p);
        }
        // Print depends changes
        m_model->printDependsChanges();

        qCDebug(appLog) << "Committing changes for dependency installation";
        transaction = backend->commitChanges();
        if (!transaction) {
            qCDebug(appLog) << "Transaction is null after committing changes, returning false";
            return false;
        }
        // Dependency install result handler
        connect(transaction, &Transaction::finished, this, &AptInstallBackend::slotDependsInstallTransactionFinished);
    } else {
        // Direct install (all deps satisfied)
        qCDebug(appLog) << "Dependencies are satisfied, installing directly";
        if (DebListModel::isDpkgRunning()) {
            qCInfo(appLog) << "AptInstallBackend: dpkg running, waiting...";
            qCDebug(appLog) << "Dpkg is running, waiting before next install check";
            QTimer::singleShot(1000, m_model, [this]() { m_model->installNextDeb(); });
            return true;
        }
        qCDebug(appLog) << "Installing file directly";
        transaction = backend->installFile(deb);
        if (!transaction) {
            qCDebug(appLog) << "Transaction is null after installing file, returning false";
            return false;
        }
        // Progress and finish handlers
        connect(transaction, &Transaction::progressChanged, m_model, &DebListModel::signalCurrentPacakgeProgressChanged);
        connect(transaction, &Transaction::finished, this, &AptInstallBackend::slotTransactionFinished);
    }

    // NOTE: DO NOT remove this.
    transaction->setLocale(".UTF-8");

    // Log output
    connect(transaction, &Transaction::statusDetailsChanged, m_model, &DebListModel::signalAppendOutputInfo);

    // Refresh operating status
    connect(transaction, &Transaction::statusDetailsChanged, this, &AptInstallBackend::slotTransactionOutput);

    // Auth handling
    connect(transaction, &Transaction::statusChanged, this, &AptInstallBackend::slotTransactionStatusChanged);

    // Error handling
    connect(transaction, &Transaction::errorOccurred, this, &AptInstallBackend::slotTransactionErrorOccurred);

    m_currentTransaction = transaction;

#ifdef ENABLE_QAPT_SETENV
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QVariantMap map;

    QString currentUser = env.value("USER");
    QString realUser = env.value("SUDO_USER");
    if (realUser.isEmpty())
        realUser = currentUser;
    map.insert("SUDO_USER", realUser);
    m_currentTransaction->setEnvVariable(map);
#endif

    qCDebug(appLog) << "Running current transaction";
    m_currentTransaction->run();

    return true;
}

bool AptInstallBackend::uninstall(int index)
{
    Q_UNUSED(index)
    qCDebug(appLog) << "AptInstallBackend::uninstall entering for index" << m_model->m_operatingIndex;

    DebFile debFile(m_model->m_packagesManager->package(m_model->m_operatingIndex));
    if (!debFile.isValid()) {
        qCWarning(appLog) << "Invalid deb file at index" << m_model->m_operatingIndex;
        return false;
    }
    const QStringList rdepends =
        m_model->m_packagesManager->packageReverseDependsList(debFile.packageName(), debFile.architecture());
    qCInfo(appLog) << QString("Will remove reverse depends before remove %1, Lists:").arg(debFile.packageName()) << rdepends;

    Backend *backend = PackageAnalyzer::instance().backendPtr();

    qCInfo(appLog) << "Uninstalling reverse dependencies:" << rdepends;
    for (const auto &r : rdepends) {
        if (backend->package(r)) {
            qCDebug(appLog) << "Purging reverse dependency:" << r;
            backend->package(r)->setPurge();
        } else {
            qCWarning(appLog) << "Failed to find reverse dependency package:" << r;
        }
    }

    const QString packageId = debFile.packageName() + ':' + debFile.architecture();
    qCDebug(appLog) << "Looking up package to uninstall:" << packageId;
    QApt::Package *uninstalledPackage = backend->package(packageId);

    if (!uninstalledPackage) {
        qCWarning(appLog) << "Failed to find package to uninstall:" << packageId;
        m_model->refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Failed);
        return false;
    }
    uninstalledPackage->setPurge();

    m_model->refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Operating);
    Transaction *transsaction = backend->commitChanges();

    connect(transsaction, &Transaction::progressChanged, m_model, &DebListModel::signalCurrentPacakgeProgressChanged);
    connect(transsaction, &Transaction::statusDetailsChanged, m_model, &DebListModel::signalAppendOutputInfo);
    connect(transsaction, &Transaction::statusChanged, this, &AptInstallBackend::slotTransactionStatusChanged);
    connect(transsaction, &Transaction::errorOccurred, this, &AptInstallBackend::slotTransactionErrorOccurred);
    connect(transsaction, &Transaction::finished, this, &AptInstallBackend::slotUninstallFinished);
    connect(transsaction, &Transaction::finished, transsaction, &Transaction::deleteLater);

    m_currentTransaction = transsaction;

#ifdef ENABLE_QAPT_SETENV
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QVariantMap map;

    QString currentUser = env.value("USER");
    QString realUser = env.value("SUDO_USER");
    if (realUser.isEmpty())
        realUser = currentUser;
    map.insert("SUDO_USER", realUser);
    transsaction->setEnvVariable(map);
#endif

    qCDebug(appLog) << "Starting uninstall transaction for package:" << packageId;
    transsaction->run();

    return true;
}

void AptInstallBackend::slotTransactionOutput()
{
    qCDebug(appLog) << "Entering AptInstallBackend::slotTransactionOutput";
    if (m_model->m_workerStatus == AbstractPackageListModel::WorkerProcessing) {
        qCInfo(appLog) << "installer status error";
    }
    Transaction *trans = qobject_cast<Transaction *>(sender());
    if (!trans) {
        qCDebug(appLog) << "Transaction object is null, returning";
        return;
    }

    qCDebug(appLog) << "Refreshing package status to Operating";
    m_model->refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Operating);

    disconnect(trans, &Transaction::statusDetailsChanged, this, &AptInstallBackend::slotTransactionOutput);
}

void AptInstallBackend::slotTransactionFinished()
{
    qCDebug(appLog) << "Entering AptInstallBackend::slotTransactionFinished";
    if (m_model->m_workerStatus == AbstractPackageListModel::WorkerProcessing) {
        qWarning() << "installer status still processing";
    }
    Transaction *transaction = qobject_cast<Transaction *>(sender());
    if (!transaction) {
        qCDebug(appLog) << "Transaction object is null, returning";
        return;
    }
    // Prevent next signal
    qCDebug(appLog) << "Disconnecting transaction finished signal";
    disconnect(transaction, &Transaction::finished, this, &AptInstallBackend::slotTransactionFinished);

    // Report progress
    int progressValue = static_cast<int>(100. * (m_model->m_operatingIndex + 1) / m_model->preparedPackages().size());
    qCDebug(appLog) << "Updating whole progress to" << progressValue;
    Q_EMIT m_model->signalWholeProgressChanged(progressValue);

    qCInfo(appLog) << "AptInstallBackend: transaction finished with exit status:" << transaction->exitStatus();
    if (transaction->exitStatus()) {
        qCDebug(appLog) << "Transaction finished with an error";
        qWarning() << transaction->error() << transaction->errorDetails() << transaction->errorString();

        QString errorInfo = transaction->errorDetails();
        if (errorInfo.isEmpty()) {
            qCDebug(appLog) << "Error details is empty, using error string";
            errorInfo = transaction->errorString();
        }
        QString sPackageName = m_model->m_packagesManager->package(m_model->m_operatingIndex);
        bool verifyError = HierarchicalVerify::instance()->checkTransactionError(sPackageName, errorInfo);
        qCDebug(appLog) << "Hierarchical verification result:" << verifyError;

        if (verifyError) {
            qCDebug(appLog) << "Hierarchical verification failed, setting flag";
            m_model->m_hierarchicalVerifyError = true;
        }

        m_model->m_packageFailCode[m_model->m_operatingPackageMd5] =
            verifyError ? static_cast<int>(Pkg::DigitalSignatureError) : static_cast<int>(transaction->error());
        m_model->m_packageFailReason[m_model->m_operatingPackageMd5] = transaction->errorString();

        qCDebug(appLog) << "Refreshing package status to Failed";
        m_model->refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Failed);
        Q_EMIT m_model->signalAppendOutputInfo(transaction->errorString());
    } else if (m_model->m_packageOperateStatus.contains(m_model->m_operatingPackageMd5) &&
               m_model->m_packageOperateStatus[m_model->m_operatingPackageMd5] != Pkg::PackageOperationStatus::Failed) {
        qCDebug(appLog) << "Transaction finished successfully";
        m_model->refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Success);

        if (m_model->m_operatingStatusIndex < m_model->preparedPackages().size() - 1) {
            qCDebug(appLog) << "Preparing for next package installation";
            auto md5 = m_model->m_packagesManager->getPackageMd5(m_model->m_operatingIndex + 1);
            m_model->m_packageOperateStatus[md5] = Pkg::PackageOperationStatus::Waiting;
        }
    }

    if (!m_currentTransaction.isNull()) {
        qCDebug(appLog) << "Deleting current transaction";
        m_currentTransaction->deleteLater();
        m_currentTransaction = nullptr;
    }
    transaction = nullptr;
    qCDebug(appLog) << "Bumping install index";
    m_model->bumpInstallIndex();
}

void AptInstallBackend::slotDependsInstallTransactionFinished()
{
    qCDebug(appLog) << "Entering AptInstallBackend::slotDependsInstallTransactionFinished";
    if (m_model->m_workerStatus == AbstractPackageListModel::WorkerProcessing) {
        qWarning() << "installer status still processing";
    }
    Transaction *transaction = qobject_cast<Transaction *>(sender());
    if (!transaction) {
        qCDebug(appLog) << "Transaction object is null, returning";
        return;
    }

    const auto transExitStatus = transaction->exitStatus();
    qCDebug(appLog) << "Transaction exit status:" << transExitStatus;

    if (transExitStatus) {
        qCDebug(appLog) << "Dependency installation transaction failed";
        qWarning() << transaction->error() << transaction->errorDetails() << transaction->errorString();
        m_model->m_packageFailCode[m_model->m_operatingPackageMd5] = transaction->error();
        m_model->m_packageFailReason[m_model->m_operatingPackageMd5] = transaction->errorString();
        m_model->refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Failed);
        Q_EMIT m_model->signalAppendOutputInfo(transaction->errorString());
    }

    if (!m_currentTransaction.isNull()) {
        qCDebug(appLog) << "Deleting current transaction";
        m_currentTransaction->deleteLater();
        m_currentTransaction = nullptr;
    }
    transaction = nullptr;

    if (transExitStatus) {
        qCDebug(appLog) << "Bumping install index due to failed dependency install";
        m_model->bumpInstallIndex();
    } else {
        qCDebug(appLog) << "Dependency installation successful, installing next deb";
        // Dependencies already verified, no need to re-verify
        m_model->installNextDeb();
    }
}

void AptInstallBackend::slotTransactionErrorOccurred()
{
    qCDebug(appLog) << "Entering AptInstallBackend::slotTransactionErrorOccurred";
    if (AbstractPackageListModel::WorkerProcessing != m_model->m_workerStatus) {
        qWarning() << "installer status error" << m_model->m_workerStatus;
    }
    Transaction *transaction = qobject_cast<Transaction *>(sender());
    if (!transaction) {
        qCWarning(appLog) << "Transaction object is null, returning.";
        return;
    }

    m_model->refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Failed);
    m_model->m_packageOperateStatus[m_model->m_operatingPackageMd5] = Pkg::PackageOperationStatus::Failed;

    m_model->m_packageFailCode[m_model->m_operatingPackageMd5] = transaction->error();
    m_model->m_packageFailReason[m_model->m_operatingPackageMd5] = transaction->errorString();

    if (!transaction->errorString().contains("proper authorization was not provided")) {
        qCDebug(appLog) << "Emitting signalAppendOutputInfo with error:" << transaction->errorString();
        Q_EMIT m_model->signalAppendOutputInfo(transaction->errorString());
    }

    const QApt::ErrorCode errorCode = transaction->error();

    qWarning() << "AptInstallBackend:"
               << "Transaction Error:" << errorCode << m_model->workerErrorString(errorCode, transaction->errorString());
    qWarning() << "AptInstallBackend:"
               << "Error Information:" << transaction->errorDetails() << transaction->errorString();

    if (transaction->isCancellable()) {
        qCDebug(appLog) << "Cancelling transaction.";
        transaction->cancel();
    }

    // Special handling for auth errors
    if (AuthError == errorCode) {
        qCDebug(appLog) << "Handling authorization error.";
        transaction->deleteLater();
        QTimer::singleShot(
            100, this, &AptInstallBackend::checkBoxStatus);
        qWarning() << "AptInstallBackend:"
                   << "Authorization error";

        Q_EMIT m_model->signalLockForAuth(false);
        Q_EMIT m_model->signalAuthCancel();
        Q_EMIT m_model->signalEnableCloseButton(true);
        m_model->m_workerStatus = AbstractPackageListModel::WorkerPrepare;
        return;
    }

    qCDebug(appLog) << "Setting transaction exit status to failed.";
    transaction->setProperty("exitStatus", QApt::ExitFailed);
}

void AptInstallBackend::slotTransactionStatusChanged(TransactionStatus transactionStatus)
{
    qCDebug(appLog) << "AptInstallBackend::slotTransactionStatusChanged status:" << transactionStatus;
    switch (transactionStatus) {
        case TransactionStatus::AuthenticationStatus:
            qCDebug(appLog) << "Transaction status: AuthenticationStatus, emitting signalLockForAuth(true).";
            Q_EMIT m_model->signalLockForAuth(true);
            break;
        case TransactionStatus::WaitingStatus:
            qCDebug(appLog) << "Transaction status: WaitingStatus, emitting signalLockForAuth(false).";
            Q_EMIT m_model->signalLockForAuth(false);
            break;
        default:
            qCDebug(appLog) << "Transaction status: unhandled status.";
            break;
    }
}

void AptInstallBackend::slotUninstallFinished()
{
    qCDebug(appLog) << "Entering AptInstallBackend::slotUninstallFinished";
    if (m_model->m_workerStatus == AbstractPackageListModel::WorkerProcessing) {
        qCInfo(appLog) << "installer status error";
    }

    Transaction *trans = qobject_cast<Transaction *>(sender());
    if (!trans) {
        qCDebug(appLog) << "Transaction object is null, returning";
        return;
    }

    if (trans->exitStatus()) {
        qCDebug(appLog) << "Uninstall finished with an error";
        m_model->m_workerStatus = AbstractPackageListModel::WorkerFinished;
        m_model->refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Failed);
        m_model->m_packageOperateStatus[m_model->m_operatingPackageMd5] = Pkg::PackageOperationStatus::Failed;
        qWarning() << "AptInstallBackend:"
                   << "uninstall finished with finished code:" << trans->error() << "finished details:" << trans->errorString();
    } else {
        qCDebug(appLog) << "Uninstall finished successfully";
        m_model->m_workerStatus = AbstractPackageListModel::WorkerFinished;
        m_model->refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Success);
        m_model->m_packageOperateStatus[m_model->m_operatingPackageMd5] = Pkg::PackageOperationStatus::Success;
    }
    qCDebug(appLog) << "Emitting signalWorkerFinished for uninstall";
    Q_EMIT m_model->signalWorkerFinished();
    trans->deleteLater();
}

void AptInstallBackend::setEndEnable()
{
    qCDebug(appLog) << "Entering AptInstallBackend::setEndEnable";
    Q_EMIT m_model->signalEnableReCancelBtn(true);
}

void AptInstallBackend::checkBoxStatus()
{
    qCDebug(appLog) << "Entering AptInstallBackend::checkBoxStatus";
    QTime startTime = QTime::currentTime();
    Transaction *transation = nullptr;
    auto *const backend = PackageAnalyzer::instance().backendPtr();
    qCDebug(appLog) << "Committing changes to backend";
    transation = backend->commitChanges();

    QTime stopTime = QTime::currentTime();
    int elapsed = startTime.msecsTo(stopTime);
    if (elapsed > 20000) {
        qCDebug(appLog) << "Commit timeout, re-checking status";
        QTimer::singleShot(100, this, &AptInstallBackend::checkBoxStatus);
        return;
    }

    if (transation) {
        if (transation->isCancellable()) {
            qCDebug(appLog) << "Transaction is cancellable, cancelling it";
            transation->cancel();
            QTimer::singleShot(100, this, &AptInstallBackend::setEndEnable);
        } else {
            qCDebug(appLog) << "Transaction is not cancellable, re-checking status";
            QTimer::singleShot(100, this, &AptInstallBackend::checkBoxStatus);
        }
    } else {
        qWarning() << "AptInstallBackend:"
                   << "Transaction is Nullptr";
    }
}
