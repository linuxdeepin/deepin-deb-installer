// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deb_package.h"
#include "ddlog.h"

#include <QApt/Backend>
#include <QApt/Package>

#include "model/packageanalyzer.h"
#include "compatible/compatible_backend.h"
#include "compatible/compatible_defines.h"
#include "utils/utils.h"

namespace Deb {

bool needRemoveByVersion(const QString &installed_version, const QString &conflict_version, QApt::RelationType type)
{
    // 如果版本条件为空，则需要移除
    if (conflict_version.isEmpty()) {
        return true;
    }

    // 比较版本号
    const int result = QApt::Package::compareVersion(installed_version, conflict_version);

    // 根据关系类型判断是否需要移除
    switch (type) {
    case QApt::LessOrEqual:
        return result <= 0;
    case QApt::GreaterOrEqual:
        return result >= 0;
    case QApt::LessThan:
        return result < 0;
    case QApt::GreaterThan:
        return result > 0;
    case QApt::Equals:
        return result == 0;
    case QApt::NotEqual:
        return result != 0;
    default:
        return true;
    }
}

DebPackage::DebPackage(const QString &debFilePath)
    : m_debFilePtr(QSharedPointer<QApt::DebFile>::create(debFilePath))
{
    qCDebug(appLog) << "Creating DebPackage for:" << debFilePath;
}

bool DebPackage::isValid() const
{
    const bool valid = m_debFilePtr->isValid();
    // qCDebug(appLog) << "Checking if deb package is valid:" << valid;
    return valid;
}

bool DebPackage::fileExists() const
{
    qCDebug(appLog) << "Checking if deb file exists:" << m_exists;
    return m_exists;
}

void DebPackage::markNotExists()
{
    qCDebug(appLog) << "Marking deb file as not existing";
    m_exists = false;
}

QString DebPackage::filePath() const
{
    qCDebug(appLog) << "Getting file path:" << m_debFilePtr->filePath();
    return m_debFilePtr->filePath();
}

QByteArray DebPackage::md5()
{
    qCDebug(appLog) << "Getting MD5 sum";
    if (m_md5.isEmpty() && m_debFilePtr->isValid()) {
        qCDebug(appLog) << "MD5 sum is empty, calculating it";
        m_md5 = m_debFilePtr->md5Sum();
    }

    return m_md5;
}

const QSharedPointer<QApt::DebFile> &DebPackage::debInfo() const
{
    qCDebug(appLog) << "Getting deb file info";
    return m_debFilePtr;
}

void DebPackage::setOperationStatus(Pkg::PackageOperationStatus s)
{
    qCInfo(appLog) << "Package operation status changed from" << m_operationStatus << "to" << s;
    m_operationStatus = s;
}

Pkg::PackageOperationStatus DebPackage::operationStatus() const
{
    qCDebug(appLog) << "Getting operation status:" << m_operationStatus;
    return m_operationStatus;
}

Pkg::PackageInstallStatus DebPackage::installStatus() const
{
    qCDebug(appLog) << "Getting install status:" << m_installStatus;
    return m_installStatus;
}

PackageDependsStatus &DebPackage::dependsStatus()
{
    qCDebug(appLog) << "Getting depends status";
    return m_dependsStatus;
}

bool DebPackage::containsTemplates()
{
    qCDebug(appLog) << "Checking if package contains templates";
    if (UnknownTemplates == m_templatesState) {
        qCDebug(appLog) << "Templates state is unknown, checking now";
        m_templatesState = Utils::checkPackageContainsDebConf(m_debFilePtr->filePath()) ? ContainTemplates : NoTemplates;
        qCDebug(appLog) << "Templates state set to:" << m_templatesState;
    }

    return m_templatesState == ContainTemplates;
}

bool DebPackage::containRemovePackages() const
{
    const bool contains = !m_removePackages.isEmpty();
    // qCDebug(appLog) << "Checking if package contains packages to remove:" << contains;
    return contains;
}

void DebPackage::setMarkedPackages(const QStringList &installDepends)
{
    qCDebug(appLog) << "Setting marked packages for" << m_debFilePtr->filePath() << "with depends:" << installDepends;
    m_removePackages.clear();
    if (!m_debFilePtr->isValid()) {
        qCWarning(appLog) << "Invalid deb file when setting marked packages:" << m_debFilePtr->filePath();
        return;
    }

    QApt::Backend *backend = PackageAnalyzer::instance().backendPtr();
    if (!backend) {
        qCWarning(appLog) << "Backend is null when setting marked packages";
        return;
    }

    backend->saveCacheState();

    for (const QString &depends : installDepends) {
        // annother error
        if (depends.contains(" not found")) {
            backend->undo();
            return;
        }
        backend->markPackageForInstall(depends);
    }

    QApt::PackageList markedPackages = backend->markedPackages();

    for (const QApt::Package *package : markedPackages) {
        if (package->state() & QApt::Package::ToRemove) {
            m_removePackages << package->name();
        }
    }

    // must restore changes, not install now
    backend->undo();

    /* In the dependency check of the PacakgesManager, the dependency satisfaction of
       the breaks/conflicts package has been detected, but only the packages that
       will be installed have been marked.
       Mark the breaks/conflicts package for uninstalling.
    */
    const QList<QApt::DependencyItem> selfRemovePackages = m_debFilePtr->breaks() + m_debFilePtr->conflicts();
    for (const QApt::DependencyItem &item : selfRemovePackages) {
        for (const QApt::DependencyInfo &info : item) {
            QApt::Package *package = backend->package(info.packageName());
            if (package && package->isInstalled() && needRemoveByVersion(package->installedVersion(), info.packageVersion(), info.relationType())) {
                m_removePackages << package->name();
            }
        }
    }
}

QStringList DebPackage::removePackages() const
{
    // qCDebug(appLog) << "Getting list of packages to remove:" << m_removePackages;
    return m_removePackages;
}

void DebPackage::setError(int code, const QString &string)
{
    qCWarning(appLog) << "Package error occurred, code:" << code << "message:" << string;
    m_errorCode = code;
    m_errorString = string;
}

int DebPackage::errorCode() const
{
    // qCDebug(appLog) << "Getting error code:" << m_errorCode;
    return m_errorCode;
}

QString DebPackage::errorString() const
{
    // qCDebug(appLog) << "Getting error string:" << m_errorString;
    return m_errorString;
}

const QSharedPointer<Compatible::CompPkgInfo> &DebPackage::compatible()
{
    qCDebug(appLog) << "Getting compatible info";
    if (!m_compInfoPtr && isValid()) {
        qCDebug(appLog) << "Checking compatible mode for package:" << m_debFilePtr->packageName();
        // might installed in compatbile mode.
        m_compInfoPtr = CompBackend::instance()->containsPackage(m_debFilePtr->packageName());

        if (!m_compInfoPtr) {
            qCDebug(appLog) << "Creating compatible info";
            m_compInfoPtr = Compatible::CompPkgInfo::Ptr::create();

            m_compInfoPtr->name = m_debFilePtr->packageName();
            m_compInfoPtr->version = m_debFilePtr->version();
            m_compInfoPtr->arch = m_debFilePtr->architecture();
        }

        m_compInfoPtr->filePath = m_debFilePtr->filePath();
    }

    return m_compInfoPtr;
}

};  // namespace Deb
