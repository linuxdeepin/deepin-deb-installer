// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PackagesManager.h"
#include "status/PackageSigntureStatus.h"
#include "status/GetStatusThread.h"
#include "installer/PackageInstaller.h"
#include "package/Package.h"

#include <QFileInfo>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(devLog)

PackagesManager::PackagesManager()
    : m_pPackageStatus(new PackageStatus())
    , m_pPackageInstaller(new PackageInstaller(m_pPackageStatus->m_backendFuture.result()))
{
    qCDebug(devLog) << "Creating PackagesManager";
    m_pGetStatusThread = new GetStatusThread(m_pPackageStatus);

    initConnection();
}

void PackagesManager::initConnection()
{
    qCDebug(devLog) << "Initializing connections for PackagesManager";
    connect(m_pPackageInstaller, &PackageInstaller::signal_startInstall, this, &PackagesManager::signal_startInstallPackages);

    connect(m_pPackageInstaller, &PackageInstaller::signal_installProgress, this, &PackagesManager::signal_installProgress);

    connect(
        m_pPackageInstaller, &PackageInstaller::signal_installDetailStatus, this, &PackagesManager::signal_installDetailStatus);

    connect(m_pPackageInstaller, &PackageInstaller::signal_installError, this, &PackagesManager::signal_installErrorOccured);

    connect(m_pPackageInstaller, &PackageInstaller::signal_installFinished, this, &PackagesManager::slot_installFinished);

    connect(m_pPackageInstaller, &PackageInstaller::signal_uninstallFinished, this, &PackagesManager::slot_uninstallFinished);

    connect(m_pGetStatusThread, &GetStatusThread::signal_dependsStatus, this, &PackagesManager::slot_getDependsStatus);

    connect(m_pGetStatusThread, &GetStatusThread::signal_installStatus, this, &PackagesManager::slot_getInstallStatus);
}

void PackagesManager::slot_getDependsStatus(int index, DependsStatus dependsStatus)
{
    // qCDebug(devLog) << "Slot: Get depends status for index" << index << "status:" << dependsStatus;
    Package *pkg = searchByIndex(index);
    if (pkg) {
        pkg->setPackageDependStatus(dependsStatus);

        // 根据获取的依赖结果，发送信号
        switch (dependsStatus) {
        case DependsOk:
            qCDebug(devLog) << "Depends status: OK";
        case DependsAvailable:  // 依赖满足，不发送信号
            qCDebug(devLog) << "Depends status: Available";
            break;
        case DependsBreak:  // 依赖不满足
            qCWarning(devLog) << "Depends status: Break";
            emit signal_dependStatusError(index, DependsBreak);
            break;
        case DependsAuthCancel:  // 依赖下载授权被取消
            qCWarning(devLog) << "Depends status: AuthCancel";
            emit signal_dependStatusError(index, DependsAuthCancel);
            break;
        case DependsUnknown:  // 依赖未知（下载依赖失败）
            qCWarning(devLog) << "Depends status: Unknown";
            emit signal_dependStatusError(index, DependsUnknown);
            break;
        case ArchBreak:  // 依赖架构错误
            qCWarning(devLog) << "Depends status: ArchBreak";
            emit signal_dependStatusError(index, ArchBreak);
            break;
        }
    } else {
        // 未获取到 当前包的下标
        qCWarning(devLog) << "Package not found for index" << index;
    }
}

void PackagesManager::slot_getInstallStatus(int index, InstallStatus installStatus)
{
    // qCDebug(devLog) << "Slot: Get install status for index" << index << "status:" << installStatus;
    Package *pkg = searchByIndex(index);
    pkg->setPackageInstallStatus(installStatus);
    if (!m_appendFinished) {
        qCDebug(devLog) << "Append finished flag set to true";
        m_appendFinished = true;
    } else {
        qCDebug(devLog) << "Emitting add package success signal for index" << index;
        emit signal_addPackageSuccess(index);
    }
}

void PackagesManager::appendPackages(QStringList packages)
{
    qCDebug(devLog) << "Appending packages:" << packages;
    for (int i = 0; i < packages.size(); i++) {
        getPackageInfo(packages[i], i);
    }
}

void PackagesManager::removePackage(int index)
{
    qCDebug(devLog) << "Removing package at index:" << index;
    Package *pkg = searchByIndex(index);
    if (pkg) {
        m_packages.removeOne(pkg);
        m_packagesMd5.remove(pkg->getMd5());
        emit signal_removePackageSuccess(index);
    }
    delete pkg;
    qCDebug(devLog) << "Package removed successfully";
}

int PackagesManager::checkInstallStatus(int index)
{
    qCDebug(devLog) << "Checking install status for index:" << index;
    Package *pkg = searchByIndex(index);
    if (pkg) {
        return pkg->getInstallStatus();
    } else {
        qCWarning(devLog) << "Package not found for index" << index;
        return InstallStatusUnknown;
    }
}

bool PackagesManager::checkPackageValid(int index)
{
    qCDebug(devLog) << "Checking package validity for index:" << index;
    Package *package = searchByIndex(index);
    if (package) {
        return package->getValid();
    } else {
        qCWarning(devLog) << "Package not found for index" << index;
        return false;
    }
}

bool PackagesManager::checkPackageSignture(int index)
{
    qCDebug(devLog) << "Checking package signature for index:" << index;
    Package *package = searchByIndex(index);
    if (package) {
        if (SigntureVerifySuccess == package->getSigntureStatus()) {
            qCDebug(devLog) << "Signature verify success for index:" << index;
            return true;
        } else {
            qCWarning(devLog) << "Package verify signature failed with status:" << package->getSigntureStatus();
            return false;
        }
    } else {
        qCWarning(devLog) << "Package not found for index" << index;
        return false;
    }
}

bool PackagesManager::checkPackageDependsStatus(int index)
{
    qCDebug(devLog) << "Checking package dependency status for index:" << index;
    Package *package = searchByIndex(index);
    if (package) {
        bool status = (package->getDependStatus() == DependsOk || package->getDependStatus() == DependsAvailable);
        qCDebug(devLog) << "Dependency status for index" << index << "is" << status;
        return status;

    } else {
        qCWarning(devLog) << "Package not found for index" << index;
        return false;
    }
}

bool PackagesManager::checkPackageSuffix(QString packagePath)
{
    qCDebug(devLog) << "Checking package suffix for path:" << packagePath;
    const QFileInfo info(packagePath);

    if (info.exists() && info.isFile() && info.suffix().toLower() == "deb") {  // 大小写不敏感的判断是否为deb后缀
        qCDebug(devLog) << "Suffix is .deb, check passed";
        return true;
    }
    qCWarning(devLog) << "Package suffix check failed for path:" << packagePath;
    return false;
}

void PackagesManager::getPackageInfo(QString packagePath, int index)
{
    qCDebug(devLog) << "Getting package info for path:" << packagePath << "at index:" << index;
    m_appendFinished = false;
    if (!checkPackageSuffix(packagePath)) {
        emit signal_packageInvalid(index);
        return;
    }
    m_pGetStatusThread->setPackage(index, packagePath);
    m_pGetStatusThread->start();

    Package *packageFile = new Package(index, packagePath);

    if (!packageFile->getValid()) {
        qCWarning(devLog) << "Package file is invalid:" << packagePath;
        emit signal_packageInvalid(index);
        return;
    }

    auto md5 = packageFile->getMd5();
    if (m_packagesMd5.contains(md5)) {
        qCWarning(devLog) << "Package with md5" << md5 << "already exists.";
        emit signal_packageAlreadyExits(index);
        return;
    }

    if (packageFile->getSigntureStatus() != SigntureVerifySuccess) {
        qCWarning(devLog) << "Package signature error with status:" << packageFile->getSigntureStatus();
        emit signal_signatureError(index, packageFile->getSigntureStatus());
        return;
    }

    m_packagesMd5 << md5;
    m_packages.append(packageFile);

    if (!m_appendFinished) {
        m_appendFinished = true;
    } else {
        emit signal_addPackageSuccess(index);
    }
}

void PackagesManager::install()
{
    qCDebug(devLog) << "Starting installation";
    if (m_packages.size() > 0) {
        m_pPackageInstaller->appendPackage(m_packages[0]);
        m_pPackageInstaller->installPackage();
    } else {
        qCWarning(devLog) << "No packages to install, index invalid.";
        emit signal_invalidIndex(0);
    }
}

void PackagesManager::uninstall(int index)
{
    qCDebug(devLog) << "Starting uninstallation for index:" << index;
    Package *package = searchByIndex(index);
    if (package) {
        if (package->getInstallStatus() <= NotInstalled) {
            qCWarning(devLog) << "Package is not installed, cannot uninstall. Index:" << index;
            emit signal_packageNotInstalled(index);
            return;
        }
        QStringList reverseDepends =
            m_pPackageStatus->getPackageReverseDependsList(package->getName(), package->getArchitecture());

        if (!reverseDepends.isEmpty()) {
            qCDebug(devLog) << "Setting reverse dependencies:" << reverseDepends;
            package->setPackageReverseDependsList(reverseDepends);
        }

        m_pPackageInstaller->appendPackage(package);
        m_pPackageInstaller->uninstallPackage();
    } else {
        qCWarning(devLog) << "Package not found for uninstall, index:" << index;
    }
}

Package *PackagesManager::searchByIndex(int index)
{
    qCDebug(devLog) << "Searching for package by index:" << index;
    auto iter =
        std::find_if(m_packages.begin(), m_packages.end(), [index](const auto &package) { return package->getIndex() == index; });
    if (iter != m_packages.end()) {
        qCDebug(devLog) << "Package found for index:" << index;
        return *iter;
    }
    emit signal_invalidIndex(index);
    qCWarning(devLog) << "Package not found for index:" << index;
    return nullptr;
}

void PackagesManager::slot_installFinished(QApt::ExitStatus exitStatus)
{
    qCDebug(devLog) << "Slot: Install finished with exit status:" << exitStatus;
    if (QApt::ExitSuccess == exitStatus) {
        qCDebug(devLog) << "Installation successful, removing package from queue.";
        m_packagesMd5.remove(m_packages[0]->getMd5());
        m_packages.removeAt(0);

        if (m_packages.isEmpty()) {
            qCDebug(devLog) << "All packages installed, emitting install finished signal.";
            emit signal_installFinished();
            return;
        }
        qCDebug(devLog) << "More packages to install, reloading cache and continuing.";
        QApt::Backend *backend = m_pPackageStatus->m_backendFuture.result();
        backend->reloadCache();
        install();
    } else {
        qCWarning(devLog) << "Installation failed with exit status:" << exitStatus;
    }
}

void PackagesManager::slot_uninstallFinished(QApt::ExitStatus exitStatus)
{
    qCDebug(devLog) << "Slot: Uninstall finished with exit status:" << exitStatus;
    if (QApt::ExitSuccess == exitStatus) {
        qCDebug(devLog) << "Uninstallation successful, removing package from queue.";
        m_packagesMd5.remove(m_packages[0]->getMd5());
        m_packages.removeAt(0);
        emit signal_uninstallFinished();
    } else {
        qCWarning(devLog) << "Uninstallation failed with exit status:" << exitStatus;
    }
}

PackagesManager::~PackagesManager()
{
    qCDebug(devLog) << "Destroying PackagesManager";
    m_packages.clear();
    m_packagesMd5.clear();
    delete m_pPackageStatus;
}
