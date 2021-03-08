/*
* Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
*
* Author:     cuizhen <cuizhen@uniontech.com>
* Maintainer: cuizhen <cuizhen@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "PackagesManager.h"
#include "status/PackageSigntureStatus.h"
#include "status/GetStatusThread.h"
#include "installer/PackageInstaller.h"
#include "package/Package.h"

#include <QFileInfo>

PackagesManager::PackagesManager()
    : m_pPackageStatus(new PackageStatus())
    , m_pPackageInstaller(new PackageInstaller(m_pPackageStatus->m_backendFuture.result()))
{
    qInfo() << "Packages Manager";
    m_pGetStatusThread = new GetStatusThread(m_pPackageStatus);

    initConnection();
}

void PackagesManager::initConnection()
{
    qInfo() << "Packages Manager" << "initConnection";
    connect(m_pPackageInstaller, &PackageInstaller::signal_startInstall, this, &PackagesManager::signal_startInstallPackages);

    connect(m_pPackageInstaller, &PackageInstaller::signal_installProgress, this, &PackagesManager::signal_installProgress);

    connect(m_pPackageInstaller, &PackageInstaller::signal_installDetailStatus, this, &PackagesManager::signal_installDetailStatus);

    connect(m_pPackageInstaller, &PackageInstaller::signal_installError, this, &PackagesManager::signal_installErrorOccured);

    connect(m_pPackageInstaller, &PackageInstaller::signal_installFinished, this, &PackagesManager::slot_installFinished);

    connect(m_pPackageInstaller, &PackageInstaller::signal_uninstallFinished, this, &PackagesManager::slot_uninstallFinished);

    connect(m_pGetStatusThread, &GetStatusThread::signal_dependsStatus, this, &PackagesManager::slot_getDependsStatus);

    connect(m_pGetStatusThread, &GetStatusThread::signal_installStatus, this, &PackagesManager::slot_getInstallStatus);
}

void PackagesManager::slot_getDependsStatus(int index, DependsStatus dependsStatus)
{
    Package *pkg = searchByIndex(index);
    if (pkg) {
        pkg->setPackageDependStatus(dependsStatus);

        //根据获取的依赖结果，发送信号
        switch (dependsStatus) {
        case DependsOk:
        case DependsAvailable:  //依赖满足，不发送信号
            qInfo() << "[PackagesManager]<< slot_getDependsStatus" << "Package depends ok";
            break;
        case DependsBreak:          //依赖不满足
            qInfo() << "[PackagesManager]<< slot_getDependsStatus" << "Package depends Break" << index << DependsBreak;
            emit signal_dependStatusError(index, DependsBreak);
            break;
        case DependsAuthCancel:     //依赖下载授权被取消
            qInfo() << "[PackagesManager]<< slot_getDependsStatus" << "Package depends Auth Cancel" << index << DependsAuthCancel;
            emit signal_dependStatusError(index, DependsAuthCancel);
            break;
        case DependsUnknown:        //依赖未知（下载依赖失败）
            qInfo() << "[PackagesManager]<< slot_getDependsStatus" << "Package depends Unknown" << index << DependsUnknown;
            emit signal_dependStatusError(index, DependsUnknown);
            break;
        case ArchBreak:             //依赖架构错误
            qInfo() << "[PackagesManager]<< slot_getDependsStatus" << "Package Architecture Error" << index << ArchBreak;
            emit signal_dependStatusError(index, ArchBreak);
            break;
        }
    } else {
        //未获取到 当前包的下标
        qInfo() << "[PackagesManager]<< slot_getDependsStatus" << "Package not found";
    }
}

void PackagesManager::slot_getInstallStatus(int index, InstallStatus installStatus)
{
    Package *pkg = searchByIndex(index);
    pkg->setPackageInstallStatus(installStatus);
    if (!m_appendFinished) {
        m_appendFinished = true;
    } else {
        qInfo() << "[PackagesManager]<< getPackageInfo" << "appendPackage" << "success";
        emit signal_addPackageSuccess(index);
    }
}

void PackagesManager::appendPackages(QStringList packages)
{
    qInfo() << "[PackagesManager]<< appendPackages" << "packages size:" << packages.size();

    for (int i = 0; i < packages.size(); i++) {
        getPackageInfo(packages[i], i);
    }
    qInfo() << "[PackagesManager]<< appendPackages" << "end";
}

void PackagesManager::removePackage(int index)
{
    qInfo() << "[PackagesManager]" << "removePackage" << m_packages.size();
    Package *pkg = searchByIndex(index);
    if (pkg) {
        qInfo() << "[PackagesManager]" << "removePackage" << pkg->getPath();
        m_packages.removeOne(pkg);
        m_packagesMd5.remove(pkg->getMd5());
        emit signal_removePackageSuccess(index);
    }
    qInfo() << "[PackagesManager]" << "removePackage" << "Currently remaining" << m_packages.size();
    delete pkg;
}

int PackagesManager::checkInstallStatus(int index)
{
    qInfo() << "[PackagesManager]" << "checkInstallStatus";
    Package *pkg = searchByIndex(index);
    if (pkg) {
        qInfo() << "[packageManager]" << "check install status" << pkg->getInstallStatus();
        return pkg->getInstallStatus();
    } else {
        qWarning() << "[PackagesManager]<< checkInstallStatus" << "Package not found";
        return InstallStatusUnknown;
    }
}

bool PackagesManager::checkPackageValid(int index)
{
    Package *package = searchByIndex(index);
    if (package) {
        qInfo() << "[PackagesManager]<< checkPackageValid" << "Return package validity" << package->getValid();
        return package->getValid();
    } else {
        qWarning() << "[PackagesManager]<< checkPackageValid" << "Package not found";
        return false;
    }
}

bool PackagesManager::checkPackageSignture(int index)
{
    qInfo() << "[PackagesManager]<< checkPackageSignture";
    Package *package = searchByIndex(index);
    if (package) {

        if (SigntureVerifySuccess ==  package->getSigntureStatus()) {
            qInfo() << "[PackagesManager]<< checkPackageSignture" << "package verify signture sucess";
            return true;
        } else {
            qWarning() << "[PackagesManager]<< checkPackageSignture" << "package verify signture" << package->getSigntureStatus();
            return false;
        }
    } else {
        qWarning() << "[PackagesManager]<< checkPackageSignture" << "Package not found";
        return false;
    }
}

bool PackagesManager::checkPackageDependsStatus(int index)
{
    qInfo() << "[PackagesManager]<< checkPackageDependsStatus";
    Package *package = searchByIndex(index);
    if (package) {
        qInfo() << "[PackagesManager]<< checkPackageDependsStatus" << "Package status" << package->getDependStatus();
        return (package->getDependStatus() == DependsOk || package->getDependStatus() == DependsAvailable);

    } else {
        qWarning() << "[PackagesManager]<< checkPackageDependsStatus" << "Package not found";
        return false;
    }
}

bool PackagesManager::checkPackageSuffix(QString packagePath)
{
    qInfo() << "[PackagesManager]" << "checkPackageSuffix" << packagePath;
    const QFileInfo info(packagePath);

    if (info.exists() && info.isFile() && info.suffix().toLower() == "deb") {        //大小写不敏感的判断是否为deb后缀
        qInfo() << "[PackagesManager]" << "checkPackageSuffix" << "The suffix is correct";
        return false;
    }
    qWarning() << "[PackagesManager]" << "checkPackageSuffix" << "Suffix error";
    return true;
}

void PackagesManager::getPackageInfo(QString packagePath, int index)
{
    QTime md5Time;
    md5Time.start();

    m_appendFinished = false;

    qInfo() << "[PackagesManager]" << "getPackageInfo";
    if (checkPackageSuffix(packagePath)) {
        emit signal_packageInvalid(index);
        return;
    }
    m_pGetStatusThread->setPackage(index, packagePath);
    m_pGetStatusThread->start();

    qInfo() << "[PackagesManager]" << "getPackageInfo" << "check package suffix 用时" << md5Time.elapsed();

    Package *packageFile = new Package(index, packagePath);

    qInfo() << "[PackagesManager]" << "getPackageInfo" << "package 构造用时" << md5Time.elapsed();
    if (!packageFile->getValid()) {
        qWarning() << "[PackagesManager]" << "getPackageInfo" << "packageFile->getValid()" << packageFile->getValid();
        emit signal_packageInvalid(index);
        return;
    }

    qInfo() << "[PackagesManager]" << "getPackageInfo" << "get valid 构造用时" << md5Time.elapsed();
    auto md5 = packageFile->getMd5();
    if (m_packagesMd5.contains(md5)) {
        qWarning() << "[PackagesManager]" << "getPackageInfo" <<  "md5 already exists";
        emit signal_packageAlreadyExits(index);
        return;
    }

    qInfo() << "[PackagesManager]" << "getPackageInfo" << "get md5 构造用时" << md5Time.elapsed();
    if (packageFile->getSigntureStatus() != SigntureVerifySuccess) {
        qInfo() << "[PackagesManager]" << "getPackageInfo" <<  "get package signature Status" << packageFile->getSigntureStatus();
        emit signal_signatureError(index, packageFile->getSigntureStatus());
        return;
    }
    qInfo() << "[PackagesManager]" << "getPackageInfo" << "package 签名验证" << md5Time.elapsed();

    qInfo() << "[PackagesManager]<< getPackageInfo" << "append package " << packageFile << "to list";
    m_packagesMd5 << md5;
    m_packages.append(packageFile);

    if (!m_appendFinished) {
        m_appendFinished = true;
    } else {
        qInfo() << "[PackagesManager]<< getPackageInfo" << "appendPackage" << "success";
        emit signal_addPackageSuccess(index);
    }
}

void PackagesManager::install()
{
    qInfo() << "PackagesManager" << "install";
    if (m_packages.size() > 0) {

        qInfo() << "PackageManager" << "install" << m_packages[0]->getPath();
        m_pPackageInstaller->appendPackage(m_packages[0]);
        m_pPackageInstaller->installPackage();
    } else {
        qWarning() << "PackagesManager" << "install" << "index invalid";
        emit signal_invalidIndex(0);
    }
}

void PackagesManager::uninstall(int index)
{
    qInfo() << "PackagesManager" << "uninstall";
    Package *package = searchByIndex(index);
    if (package) {
        qInfo() << "PackagesManager" << "uninstall" << package->getName();
        if (package->getInstallStatus() <= NotInstalled) {
            qInfo() << "PackagesManager" << "uninstall" << package->getName() << "not installed";
            emit signal_packageNotInstalled(index);
            return;
        }
        QStringList reverseDepends = m_pPackageStatus->getPackageReverseDependsList(package->getName(), package->getArchitecture());

        if (!reverseDepends.isEmpty()) {
            qInfo() << "PackagesManager" << "uninstall" << "reverse depends" << reverseDepends;
            package->setPackageReverseDependsList(reverseDepends);
        }

        m_pPackageInstaller->appendPackage(package);
        m_pPackageInstaller->uninstallPackage();
    } else {
        qWarning() << "PackagesManager" << "uninstall" << "index invalid";
    }
}

Package *PackagesManager::searchByIndex(int index)
{
    qInfo() << "[PackagesManager]" << "searchByIndex" << index;
    for (auto package : m_packages) {
        if (package->getIndex() == index) {
            qInfo() << "[PackagesManager]<< searchByIndex" << "Package has been found";
            return package;
        }
    }
    emit signal_invalidIndex(index);
    qWarning() << "[PackagesManager]<< searchByIndex" << "Package not found";
    return nullptr;
}

void PackagesManager::slot_installFinished(QApt::ExitStatus exitStatus)
{

    qInfo() << "[PackagesManager]" << "slot_installFinished";
    if (QApt::ExitSuccess == exitStatus) {
        qInfo() << "[PackagesManager]" << "slot_installFinished" << m_packages[0]->getName() << "install Success";
        m_packagesMd5.remove(m_packages[0]->getMd5());
        m_packages.removeAt(0);

        qInfo() << "[PackagesManager]" << "slot_installFinished" << "Now there are" << m_packages.size() << " packages left that are not installed";
        if (m_packages.isEmpty()) {
            emit signal_installFinished();
            qInfo() << "[PackagesManager]" << "slot_installFinished" << "install Finished";
            return;
        }
        QApt::Backend *backend = m_pPackageStatus->m_backendFuture.result();
        backend->reloadCache();
        qInfo() << "[PackagesManager]" << "slot_installFinished" << "Now start to install" << m_packages[0]->getPath();
        install();
    }
    qInfo() << "[PackagesManager]" << "slot_installFinished" << "install failed" << exitStatus;
}

void PackagesManager::slot_uninstallFinished(QApt::ExitStatus exitStatus)
{
    qInfo() << "[PackagesManager]" << "slot_uninstallFinished";
    if (QApt::ExitSuccess == exitStatus) {
        qInfo() << "[PackagesManager]" << "slot_uninstallFinished" << m_packages[0]->getName() << "uninstall Success";
        m_packagesMd5.remove(m_packages[0]->getMd5());
        m_packages.removeAt(0);
        emit signal_uninstallFinished();
    }
    qInfo() << "[PackagesManager]" << "slot_uninstallFinished" << "uninstall Failed" << exitStatus;
}

PackagesManager::~PackagesManager()
{
    m_packages.clear();
    m_packagesMd5.clear();
    delete m_pPackageStatus;
}
