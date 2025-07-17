// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uab_package_list_model.h"

#include <QAbstractListModel>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QDebug>

#include "uab_backend.h"
#include "utils/utils.h"
#include "utils/ddlog.h"

namespace Uab {

static const float kCompleteProgress = 100.0;
static const int kPkgInitedIndex = -1;

/**
 * @class UabPackageListModel
 * @brief Manage the list model of UAB packages, and handle the install and uninstall of multiple UAB packages.
 *        Emit execution status and error messages.
 */
UabPackageListModel::UabPackageListModel(QObject *parent)
    : AbstractPackageListModel{parent}
    , m_processor(new UabProcessController(this))
    , m_fileWatcher(new QFileSystemWatcher(this))
{
    qCDebug(appLog) << "Creating UabPackageListModel";
    m_supportPackageType = Pkg::Uab;

    connect(m_processor, &UabProcessController::processOutput, this, &UabPackageListModel::signalAppendOutputInfo);
    connect(m_processor, &UabProcessController::progressChanged, this, &UabPackageListModel::slotBackendProgressChanged);
    connect(m_processor, &UabProcessController::processFinished, this, &UabPackageListModel::slotBackendProcessFinished);

    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &UabPackageListModel::slotFileChanged);

    // call init uab backend while uab package list model create
    connect(Uab::UabBackend::instance(), &Uab::UabBackend::backendInitFinsihed, this, [this]() {
        qCDebug(appLog) << "UAB backend initialized, processing delayed packages:" << m_delayAppendPackages.size();
        if (!m_delayAppendPackages.isEmpty()) {
            slotAppendPackage(m_delayAppendPackages);
        }
        m_delayAppendPackages.clear();
    });
    qCDebug(appLog) << "Initializing UAB backend";
    Uab::UabBackend::instance()->initBackend();
}

QVariant UabPackageListModel::data(const QModelIndex &index, int role) const
{
    if (!QAbstractListModel::checkIndex(index, CheckIndexOption::IndexIsValid)) {
        qCWarning(appLog) << "Invalid index" << index;
        return {};
    }

    const UabPackage::Ptr &uabPtr = m_uabPkgList.at(index.row());
    if (!uabPtr || !uabPtr->isValid()) {
        qCWarning(appLog) << "Invalid package at index" << index;
        return {};
    }

    qCDebug(appLog) << "Getting data for role" << role << "at index" << index;
    switch (role) {
        case PackageNameRole:
            qCDebug(appLog) << "Getting package name for index" << index;
            return uabPtr->info()->appName;
        case PackagePathRole:
            qCDebug(appLog) << "Getting package path for index" << index;
            return uabPtr->info()->filePath;
        case PackageVersionRole:
            qCDebug(appLog) << "Getting package version for index" << index;
            return uabPtr->info()->version;
        case PackageInstalledVersionRole:
            qCDebug(appLog) << "Getting installed version for index" << index;
            return uabPtr->installedVersion();
        case PackageShortDescriptionRole:
            Q_FALLTHROUGH();
        case PackageLongDescriptionRole:
            qCDebug(appLog) << "Getting long description for index" << index;
            return uabPtr->info()->description;
        case PackageVersionStatusRole:
            qCDebug(appLog) << "Getting version status for index" << index;
            return uabPtr->installStatus();
        case PackageDependsStatusRole:
            qCDebug(appLog) << "Getting depends status for index" << index;
            return uabPtr->dependsStatus();
        case PackageFailReasonRole:
            qCDebug(appLog) << "Getting fail reason for index" << index;
            return uabPtr->failedReason();
        case PackageOperateStatusRole:
            qCDebug(appLog) << "Getting operate status for index" << index;
            return uabPtr->operationStatus();
        case PackageTypeRole:
            qCDebug(appLog) << "Getting package type for index" << index;
            return Pkg::Uab;

        case Qt::SizeHintRole:
            qCDebug(appLog) << "Getting size hint for index" << index;
            return QSize(0, 48);
        default:
            qCDebug(appLog) << "Getting default data for index" << index;
            break;
    }

    return {};
}

int UabPackageListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    qCDebug(appLog) << "Getting row count:" << m_uabPkgList.size();
    return m_uabPkgList.size();
}

void UabPackageListModel::slotAppendPackage(const QStringList &packageList)
{
    qCDebug(appLog) << "Appending packages to model:" << packageList;
    if (!isWorkerPrepare()) {
        qCWarning(appLog) << "Cannot append packages - worker not ready";
        return;
    }

    // delay append when uab backend not ready. sa backendInitFinsihed()
    if (!Uab::UabBackend::instance()->backendInited()) {
        qCDebug(appLog) << "Backend not initialized, delaying package append";
        m_delayAppendPackages.append(packageList);
        return;
    }

    const int oldRowCount = rowCount();
    for (const QString &path : packageList) {
        qCDebug(appLog) << "Processing package:" << path;
        auto uabPtr = preCheckPackage(path);

        if (uabPtr && uabPtr->isValid()) {
            qCDebug(appLog) << "Adding valid package to model:" << uabPtr->info()->id;
            m_uabPkgList.append(uabPtr);
            m_fileWatcher->addPath(path);
        } else {
            qCWarning(appLog) << "Package validation failed:" << path;
        }
    }

    if (oldRowCount != rowCount()) {
        qCDebug(appLog) << "Package count changed from" << oldRowCount << "to" << rowCount();
        Q_EMIT signalPackageCountChanged(rowCount());
    }
}

void UabPackageListModel::removePackage(int index)
{
    qCDebug(appLog) << "Removing package at index:" << index;
    if (!isWorkerPrepare()) {
        qCWarning(appLog) << "Cannot remove package - worker not ready";
        return;
    }

    if (0 <= index && index < rowCount()) {
        auto uabPtr = m_uabPkgList.takeAt(index);
        if (uabPtr && uabPtr->info()) {
            qCDebug(appLog) << "Removing path from file watcher:" << uabPtr->info()->filePath;
            m_fileWatcher->removePath(uabPtr->info()->filePath);
        }

        qCDebug(appLog) << "Package count changed to:" << rowCount();
        Q_EMIT signalPackageCountChanged(rowCount());
    } else {
        qCWarning(appLog) << "Invalid index for package removal:" << index;
    }
}

QString UabPackageListModel::checkPackageValid(const QString &packagePath)
{
    qCDebug(appLog) << "Checking package validity:" << packagePath;
    if (Pkg::PkgReadable != Utils::checkPackageReadable(packagePath)) {
        qCWarning(appLog) << "Package not readable:" << packagePath;
        return "You can only install local ueb packages";
    }

    auto uabPtr = Uab::UabPackage::fromFilePath(packagePath);
    if (!uabPtr->isValid()) {
        qCWarning(appLog) << "Invalid UAB package:" << packagePath;
        return "The uab package may be broken";
    }

    if (packageExists(uabPtr)) {
        qCWarning(appLog) << "Package already exists in the list:" << packagePath;
        return "The uab package already added";
    }

    qCDebug(appLog) << "Package is valid:" << packagePath;
    return "";
}

Pkg::PackageInstallStatus UabPackageListModel::checkInstallStatus(const QString &packagePath)
{
    qCDebug(appLog) << "Checking install status for package:" << packagePath;
    auto uabPtr = Uab::UabPackage::fromFilePath(packagePath);
    if (!uabPtr || !uabPtr->isValid()) {
        qCWarning(appLog) << "Invalid UAB package, assuming not installed:" << packagePath;
        return Pkg::NotInstalled;
    }

    qCDebug(appLog) << "Install status for" << packagePath << "is" << uabPtr->installStatus();
    return uabPtr->installStatus();
}

Pkg::DependsStatus UabPackageListModel::checkDependsStatus(const QString &packagePath)
{
    qCDebug(appLog) << "Checking dependency status for package:" << packagePath;
    if (!Uab::UabBackend::instance()->linglongExists()) {
        qCWarning(appLog) << "Linglong environment not found, dependency broken";
        return Pkg::DependsBreak;
    }

    auto uabPtr = Uab::UabPackage::fromFilePath(packagePath);
    if (!uabPtr || !uabPtr->isValid()) {
        qCWarning(appLog) << "Invalid UAB package, assuming dependency broken:" << packagePath;
        return Pkg::DependsBreak;
    }
    qCDebug(appLog) << "Dependency status OK for package:" << packagePath;
    return Pkg::DependsOk;
}

QStringList UabPackageListModel::getPackageInfo(const QString &packagePath)
{
    qCDebug(appLog) << "Getting package info for:" << packagePath;
    auto uabPtr = Uab::UabPackage::fromFilePath(packagePath);
    if (!uabPtr || !uabPtr->isValid()) {
        qCWarning(appLog) << "Invalid UAB package, cannot get info:" << packagePath;
        return {};
    }

    // adapt deb package info interface
    auto infoPtr = uabPtr->info();
    QStringList infoList;
    infoList << infoPtr->appName;
    infoList << infoPtr->filePath;
    infoList << infoPtr->version;
    infoList << infoPtr->architecture;
    infoList << infoPtr->description;
    infoList << infoPtr->description;

    qCDebug(appLog) << "Package info retrieved for:" << packagePath;
    return infoList;
}

QString UabPackageListModel::lastProcessError()
{
    qCDebug(appLog) << "Getting last process error";
    auto findItr = std::find_if(m_uabPkgList.rbegin(), m_uabPkgList.rend(), [](const UabPackage::Ptr &uabPtr) {
        return Pkg::Failed == uabPtr->operationStatus();
    });

    if (findItr != m_uabPkgList.rend()) {
        qCDebug(appLog) << "Found last process error:" << (*findItr)->processError();
        return (*findItr)->processError();
    }
    qCDebug(appLog) << "No process error found";
    return {};
}

bool UabPackageListModel::containsSignatureFailed() const
{
    qCDebug(appLog) << "Checking if any package has signature verification failed";
    // check if a pacakge signature verify failed.
    auto findItr = std::find_if(m_uabPkgList.begin(), m_uabPkgList.end(), [](const UabPackage::Ptr &uabPtr) {
        return Pkg::DigitalSignatureError == uabPtr->m_errorCode;
    });

    const bool failed = findItr != m_uabPkgList.end();
    qCDebug(appLog) << "Signature verification failed status:" << failed;
    return failed;
}

bool UabPackageListModel::slotInstallPackages()
{
    qCDebug(appLog) << "Starting package installation for" << rowCount() << "packages";
    bool callRet = false;

    do {
        if (!isWorkerPrepare() || rowCount() <= 0) {
            qCWarning(appLog) << "Cannot install packages - worker not ready or empty package list";
            break;
        }

        if (!linglongExists()) {
            qCWarning(appLog) << "Cannot install packages - Linglong environment not found";
            break;
        }

        qCDebug(appLog) << "Resetting install status for all packages";
        // reset, mark package waiting install
        resetInstallStatus();
        for (auto uabPtr : m_uabPkgList) {
            uabPtr->m_operationStatus = Pkg::Waiting;
        }
        Q_EMIT dataChanged(index(0), index(m_uabPkgList.size() - 1), {PackageOperateStatusRole});

        setWorkerStatus(WorkerProcessing);
        qCDebug(appLog) << "Starting installation process";
        installNextUab();
        callRet = true;
    } while (false);

    if (!callRet) {
        qCWarning(appLog) << "Package installation failed, marking all as failed";
        for (auto uabPtr : m_uabPkgList) {
            uabPtr->m_operationStatus = Pkg::Failed;
        }
        Q_EMIT dataChanged(index(0), index(m_uabPkgList.size() - 1), {PackageOperateStatusRole});

        setWorkerStatus(WorkerFinished);
        return false;
    }

    qCDebug(appLog) << "Package installation started successfully";
    return true;
}

bool UabPackageListModel::slotUninstallPackage(const int i)
{
    qCDebug(appLog) << "Starting uninstallation for package at index:" << i;
    bool callRet = false;

    do {
        if (!isWorkerPrepare() || rowCount() <= 0) {
            qCWarning(appLog) << "Cannot uninstall - worker not ready or empty list";
            break;
        }

        if (!linglongExists()) {
            qCWarning(appLog) << "Cannot uninstall - Linglong not found";
            break;
        }

        if (!checkIndex(index(i))) {
            qCWarning(appLog) << "Invalid index for uninstallation:" << i;
            break;
        }

        // only single page show uninstall flow
        m_operatingIndex = i;
        setWorkerStatus(WorkerUnInstall);
        qCDebug(appLog) << "Worker status set to UnInstall";

        auto uabPtr = m_uabPkgList.value(m_operatingIndex);
        if (!uabPtr || !uabPtr->isValid()) {
            qCWarning(appLog) << "Invalid package at index:" << i;
            break;
        }
        auto removeInfoPtr = Uab::UabBackend::instance()->findPackage(uabPtr->info()->id);
        if (!removeInfoPtr) {
            qCWarning(appLog) << "Package to be uninstalled not found in backend:" << uabPtr->info()->id;
            break;
        }

        setCurrentOperation(Pkg::Waiting);

        m_processor->reset();
        m_processor->markUninstall(Uab::UabPackage::fromInfo(removeInfoPtr));
        qCDebug(appLog) << "Committing uninstallation changes";
        callRet = m_processor->commitChanges();
    } while (false);

    if (!callRet) {
        qCWarning(appLog) << "Uninstallation failed, setting status to Failed";
        setCurrentOperation(Pkg::Failed);
        setWorkerStatus(WorkerFinished);
        return false;
    }

    qCDebug(appLog) << "Uninstallation process started successfully";
    return true;
}

void UabPackageListModel::reset()
{
    qCDebug(appLog) << "Resetting UabPackageListModel";
    setWorkerStatus(WorkerPrepare);
    m_operatingIndex = kPkgInitedIndex;

    QStringList files = m_fileWatcher->files();
    if (!files.isEmpty()) {
        qCDebug(appLog) << "Removing" << files.size() << "files from watcher";
        m_fileWatcher->removePaths(files);
    }

    qCDebug(appLog) << "Clearing package list";
    m_uabPkgList.clear();
}

void UabPackageListModel::resetInstallStatus()
{
    qCDebug(appLog) << "Resetting install status for all packages";
    setWorkerStatus(WorkerPrepare);
    m_operatingIndex = kPkgInitedIndex;

    // reset deb file
    for (auto &ptr : m_uabPkgList) {
        ptr->reset();
    }
    Q_EMIT dataChanged(index(0), index(m_uabPkgList.size() - 1), {PackageOperateStatusRole});
    qCDebug(appLog) << "Install status reset completed";
}

bool UabPackageListModel::installNextUab()
{
    qCDebug(appLog) << "Installing next UAB";
    m_operatingIndex++;
    qCDebug(appLog) << "Processing package at index:" << m_operatingIndex;
    Q_ASSERT_X(m_operatingIndex >= 0, "install uab", "operating index invalid");
    if (m_operatingIndex < 0) {
        qCWarning(appLog) << "Invalid operating index:" << m_operatingIndex;
        setWorkerStatus(WorkerFinished);
        return false;
    }

    // check install finish
    if (m_operatingIndex >= rowCount()) {
        qCDebug(appLog) << "All packages processed, installation complete";
        Q_EMIT signalCurrentPacakgeProgressChanged(static_cast<int>(kCompleteProgress));
        Q_EMIT signalWholeProgressChanged(static_cast<int>(kCompleteProgress));

        setWorkerStatus(WorkerFinished);
        return true;
    }

    // notify list view scroll to current package
    Q_EMIT signalCurrentProcessPackageIndex(m_operatingIndex);

    auto uabPtr = m_uabPkgList.value(m_operatingIndex);
    if (!uabPtr || Pkg::DependsOk != uabPtr->m_dependsStatus) {
        qCWarning(appLog) << "Skipping invalid package or dependency issue at index:" << m_operatingIndex;
        setCurrentOperation(Pkg::Failed);
        installNextUab();
        return false;
    }

    qCDebug(appLog) << "Starting installation for package:" << uabPtr->info()->id << uabPtr->info()->version;
    setCurrentOperation(Pkg::Operating);

    m_processor->reset();

    // Note: Current Linglong environment supports multi version package same time,
    //       check if install same version package.
    Pkg::PackageInstallStatus installStatus = uabPtr->installStatus();
    if (Pkg::InstalledLaterVersion == installStatus) {
        if (auto sameInfoPtr = Uab::UabBackend::instance()->findPackage(uabPtr->info()->id, uabPtr->info()->version)) {
            const int ret = Utils::compareVersion(uabPtr->info()->version, sameInfoPtr->version);

            if (ret == 0) {
                installStatus = Pkg::InstalledSameVersion;
                qCDebug(appLog) << "Same version already installed:" << uabPtr->info()->id << uabPtr->info()->version;
            } else if (ret < 0) {
                installStatus = Pkg::InstalledLaterVersion;
                qCDebug(appLog) << "Newer version already installed:" << uabPtr->info()->id << uabPtr->info()->version;
            } else {
                installStatus = Pkg::InstalledEarlierVersion;
                qCDebug(appLog) << "Older version already installed:" << uabPtr->info()->id << uabPtr->info()->version;
            }
        }
    }

    switch (installStatus) {
        case Pkg::NotInstalled:
            qCDebug(appLog) << "Marking package for fresh install:" << uabPtr->info()->id;
            m_processor->markInstall(uabPtr);
            break;
        case Pkg::InstalledSameVersion: {
            qCDebug(appLog) << "Marking package for reinstall (same version):" << uabPtr->info()->id;
            auto oldInfoPtr = Uab::UabBackend::instance()->findPackage(uabPtr->info()->id, uabPtr->info()->version);
            m_processor->markUninstall(Uab::UabPackage::fromInfo(oldInfoPtr));
            m_processor->markInstall(uabPtr);
        } break;
        default: {
            qCDebug(appLog) << "Marking package for upgrade/downgrade:" << uabPtr->info()->id;
            auto oldInfoPtr = Uab::UabBackend::instance()->findPackage(uabPtr->info()->id);
            m_processor->markInstall(uabPtr);
            m_processor->markUninstall(Uab::UabPackage::fromInfo(oldInfoPtr));
        } break;
    }

    if (!m_processor->commitChanges()) {
        qCWarning(appLog) << "Failed to commit changes for package:" << uabPtr->info()->id;
        setCurrentOperation(Pkg::Failed);
        installNextUab();
        return false;
    }

    qCDebug(appLog) << "Successfully started installation process for package:" << uabPtr->info()->id;
    return true;
}

void UabPackageListModel::slotBackendProgressChanged(float progress)
{
    qCDebug(appLog) << "Backend progress changed:" << progress;
    Q_ASSERT_X(rowCount() > 0, "check count", "row count invalid");

    const float base = kCompleteProgress / rowCount();
    const float wholeProgress = (m_operatingIndex + (progress / kCompleteProgress)) * base;

    qCDebug(appLog) << "Emitting progress signals: current" << progress << "whole" << wholeProgress;
    Q_EMIT signalCurrentPacakgeProgressChanged(static_cast<int>(progress));
    Q_EMIT signalWholeProgressChanged(static_cast<int>(wholeProgress));
}

void UabPackageListModel::slotBackendProcessFinished(bool success)
{
    qCDebug(appLog) << "Backend process finished with success:" << success;
    Q_ASSERT_X(rowCount() > 0, "check count", "row count invalid");

    // update installed status
    setCurrentOperation(success ? Pkg::Success : Pkg::Failed);
    // update all data
    Q_EMIT dataChanged(index(m_operatingIndex), index(m_operatingIndex));
    qCDebug(appLog) << "Data changed signal emitted for index:" << m_operatingIndex;

    // update progress
    const float base = kCompleteProgress / rowCount();
    Q_EMIT signalCurrentPacakgeProgressChanged(static_cast<int>(kCompleteProgress));
    Q_EMIT signalWholeProgressChanged(static_cast<int>(base * (m_operatingIndex + 1)));
    qCDebug(appLog) << "Progress signals emitted";

    switch (m_workerStatus) {
        case WorkerProcessing:
            qCDebug(appLog) << "Worker processing, installing next UAB";
            installNextUab();
            break;
        case WorkerUnInstall:
            qCDebug(appLog) << "Worker uninstalling, setting worker to finished";
            setWorkerStatus(WorkerFinished);
            break;
        default:
            qCDebug(appLog) << "Unhandled worker status:" << m_workerStatus;
            break;
    }
}

void UabPackageListModel::setCurrentOperation(Pkg::PackageOperationStatus s)
{
    qCDebug(appLog) << "Setting current operation status to" << s << "for index" << m_operatingIndex;
    if (!checkIndexValid(m_operatingIndex)) {
        qCWarning(appLog) << "Invalid index, cannot set operation status:" << m_operatingIndex;
        return;
    }

    auto &uabPtr = m_uabPkgList[m_operatingIndex];
    uabPtr->m_operationStatus = s;

    // mark error info
    if (Pkg::Failed == s) {
        qCWarning(appLog) << "Operation failed, setting process error";
        uabPtr->setProcessError(Pkg::UnknownError, tr("Installation Failed"));
    }

    Q_EMIT dataChanged(index(m_operatingIndex), index(m_operatingIndex), {PackageOperateStatusRole});
    qCDebug(appLog) << "Data changed signal emitted for operation status update";
}

bool UabPackageListModel::checkIndexValid(int index) const
{
    const bool isValid = 0 <= index && index < rowCount();
    qCDebug(appLog) << "Checking if index" << index << "is valid:" << isValid;
    return isValid;
}

UabPackage::Ptr UabPackageListModel::preCheckPackage(const QString &packagePath)
{
    qCDebug(appLog) << "Pre-checking package:" << packagePath;
    auto readablilty = Utils::checkPackageReadable(packagePath);
    switch (readablilty) {
        case Pkg::PkgNotInLocal:
            qCDebug(appLog) << "Package not in local:" << packagePath;
            Q_EMIT signalAppendFailMessage(Pkg::PackageNotLocal, Pkg::Uab);
            return {};
        case Pkg::PkgNoPermission:
            qCDebug(appLog) << "Package no permission:" << packagePath;
            Q_EMIT signalAppendFailMessage(Pkg::PackageNotInstallable, Pkg::Uab);
            return {};
        default:
            break;
    }

    auto uabPtr = Uab::UabPackage::fromFilePath(packagePath);
    if (!uabPtr || !uabPtr->isValid()) {
        qCDebug(appLog) << "Package invalid:" << packagePath;
        Q_EMIT signalAppendFailMessage(Pkg::PackageInvalid, Pkg::Uab);
        return {};
    }

    if (packageExists(uabPtr)) {
        qCDebug(appLog) << "Package already exists:" << packagePath;
        Q_EMIT signalAppendFailMessage(Pkg::PackageAlreadyExists, Pkg::Uab);
        return {};
    }

    return uabPtr;
}

bool UabPackageListModel::packageExists(const UabPackage::Ptr &uabPtr) const
{
    qCDebug(appLog) << "Checking if package exists:" << uabPtr->info()->filePath;
    if (!uabPtr) {
        qCDebug(appLog) << "Package is null";
        return false;
    }

    // check already added
    auto findItr = std::find_if(m_uabPkgList.begin(), m_uabPkgList.end(), [&uabPtr](const UabPackage::Ptr &cmpUabPtr) {
        qCDebug(appLog) << "Checking if package exists:" << uabPtr->info()->filePath;
        if (uabPtr->info()->filePath == cmpUabPtr->info()->filePath) {
            qCDebug(appLog) << "Package exists:" << uabPtr->info()->filePath;
            return true;
        }

        if (uabPtr->info()->appName == cmpUabPtr->info()->appName) {
            qCDebug(appLog) << "Package exists:" << uabPtr->info()->appName;
            return true;
        }

        return false;
    });

    qCDebug(appLog) << "Package does not exist:" << uabPtr->info()->filePath;
    return findItr != m_uabPkgList.end();
}

bool UabPackageListModel::linglongExists()
{
    qCDebug(appLog) << "Checking if linglong executable exists";
    // check if linglong execuatable exits
    if (!Uab::UabBackend::instance()->recheckLinglongExists()) {
        qCDebug(appLog) << "Linglong executable does not exist";
        for (const auto &uabPtr : m_uabPkgList) {
            uabPtr->setDependsStatus(Pkg::DependsBreak);
        }

        Q_EMIT dataChanged(index(0), index(m_uabPkgList.size() - 1), {PackageDependsStatusRole});
        qCDebug(appLog) << "Data changed signal emitted for depends status update";
        return false;
    }

    qCDebug(appLog) << "Linglong executable exists";
    return true;
}

void UabPackageListModel::slotFileChanged(const QString &filePath)
{
    qCDebug(appLog) << "Checking if file changed:" << filePath;
    auto findItr = std::find_if(m_uabPkgList.begin(), m_uabPkgList.end(), [&](const UabPackage::Ptr &uabPtr) {
        qCDebug(appLog) << "Checking if file changed:" << filePath;
        if (uabPtr && uabPtr->isValid()) {
            return uabPtr->info()->filePath == filePath;
        }
        return false;
    });

    if (findItr != m_uabPkgList.end()) {
        qCDebug(appLog) << "File changed:" << filePath;
        const QFileInfo info((*findItr)->info()->filePath);
        if (!info.exists()) {
            qCDebug(appLog) << "File does not exist:" << filePath;
            (*findItr)->markNotExists();
            removePackage(static_cast<int>(std::distance(findItr, m_uabPkgList.begin())));
            m_fileWatcher->removePath(filePath);

            Q_EMIT signalPackageCannotFind(info.fileName());
        }
    }
    qCDebug(appLog) << "File changed check completed";
}

}  // namespace Uab
