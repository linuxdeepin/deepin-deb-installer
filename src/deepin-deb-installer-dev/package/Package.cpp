// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Package.h"

#include <QApt/DebFile>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(devLog)

Package::Package()
    : m_pSigntureStatus(new PackageSigntureStatus)
{
    qCDebug(devLog) << "Creating an empty Package object";
}

Package::Package(const QString &packagePath)
    : m_packagePath(packagePath)
    , m_pSigntureStatus(new PackageSigntureStatus)
{
    qCDebug(devLog) << "Creating Package object with path:" << packagePath;
    QApt::DebFile *pDebInfo = new QApt::DebFile(packagePath);
    if (!pDebInfo || !pDebInfo->isValid()) {
        qCWarning(devLog) << "Failed to get deb file info for path:" << packagePath;
        m_valid = false;
        return;
    }

    m_signtureStatus = m_pSigntureStatus->checkPackageSignture(packagePath);
    m_valid = pDebInfo->isValid();
    m_name = pDebInfo->packageName();
    m_version = pDebInfo->version();
    m_architecture = pDebInfo->architecture();
    m_md5 = pDebInfo->md5Sum();
    qCDebug(devLog) << "Package info loaded:" << m_name << m_version;

    delete pDebInfo;
}

Package::Package(int index, const QString &packagePath)
    : m_index(index)
    , m_packagePath(packagePath)
    , m_pSigntureStatus(new PackageSigntureStatus)
{
    qCDebug(devLog) << "Creating Package object with index:" << index << "and path:" << packagePath;
    QApt::DebFile *pDebInfo = new QApt::DebFile(packagePath);

    if (!pDebInfo || !pDebInfo->isValid()) {
        qCWarning(devLog) << "Failed to get deb file info for path:" << packagePath;
        m_valid = false;
        return;
    }

    m_valid = pDebInfo->isValid();
    m_name = pDebInfo->packageName();
    m_version = pDebInfo->version();
    m_architecture = pDebInfo->architecture();
    m_md5 = pDebInfo->md5Sum();
    qCDebug(devLog) << "Package info loaded:" << m_name << m_version;

    m_signtureStatus = m_pSigntureStatus->checkPackageSignture(packagePath);
}

void Package::setPackageReverseDependsList(const QStringList &reverseDepends)
{
    // qCDebug(devLog) << "Setting reverse dependencies for" << m_name << ":" << reverseDepends;
    m_packageReverseDepends = reverseDepends;
}

void Package::setPackageInstallStatus(InstallStatus packageInstallStatus)
{
    // qCDebug(devLog) << "Setting install status for" << m_name << "to" << packageInstallStatus;
    m_installStatus = packageInstallStatus;
}

void Package::setPackageIndex(int index)
{
    // qCDebug(devLog) << "Setting package index to" << index;
    m_index = index;
}

void Package::setPackagePath(const QString &packagePath)
{
    // qCDebug(devLog) << "Setting package path to" << packagePath;
    m_packagePath = packagePath;
}

void Package::setPackageDependStatus(DependsStatus packageDependStatus)
{
    // qCDebug(devLog) << "Setting dependency status for" << m_name << "to" << packageDependStatus;
    m_dependsStatus = packageDependStatus;
}

void Package::setPackageAvailableDepends(const QStringList &depends)
{
    // qCDebug(devLog) << "Setting available dependencies for" << m_name << ":" << depends;
    m_packageAvailableDependList.clear();
    m_packageAvailableDependList << depends;
}

int Package::getIndex()
{
    // qCDebug(devLog) << "Getting package index:" << m_index;
    return m_index;
}

bool Package::getValid()
{
    // qCDebug(devLog) << "Getting package validity:" << m_valid;
    return m_valid;
}

QString Package::getName()
{
    // qCDebug(devLog) << "Getting package name:" << m_name;
    return m_name;
}

QString Package::getPath()
{
    // qCDebug(devLog) << "Getting package path:" << m_packagePath;
    return m_packagePath;
}

QString Package::getVersion()
{
    // qCDebug(devLog) << "Getting package version:" << m_version;
    return m_version;
}

QString Package::getArchitecture()
{
    // qCDebug(devLog) << "Getting package architecture:" << m_architecture;
    return m_architecture;
}

QByteArray Package::getMd5()
{
    // qCDebug(devLog) << "Getting package md5:" << m_md5;
    return m_md5;
}

SigntureStatus Package::getSigntureStatus()
{
    // qCDebug(devLog) << "Getting signature status:" << m_signtureStatus;
    return m_signtureStatus;
}

DependsStatus Package::getDependStatus()
{
    // qCDebug(devLog) << "Getting dependency status:" << m_dependsStatus;
    return m_dependsStatus;
}

InstallStatus Package::getInstallStatus()
{
    // qCDebug(devLog) << "Getting install status:" << m_installStatus;
    return m_installStatus;
}

QStringList Package::getPackageAvailableDepends()
{
    // qCDebug(devLog) << "Getting available dependencies list for" << m_name;
    return m_packageAvailableDependList;
}

QStringList Package::getPackageReverseDependList()
{
    // qCDebug(devLog) << "Getting reverse dependencies list for" << m_name;
    return m_packageReverseDepends;
}
Package::~Package()
{
    // qCDebug(devLog) << "Destroying Package object for" << m_name;
    delete m_pSigntureStatus;
}
