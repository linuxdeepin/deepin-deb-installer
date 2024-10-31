// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deb_package.h"

#include <QApt/Backend>
#include <QApt/Package>

#include "model/packageanalyzer.h"

namespace Deb {

DebPackage::DebPackage(const QString &debFilePath)
    : m_debFilePtr(QSharedPointer<QApt::DebFile>::create(debFilePath))
{
}

bool DebPackage::isValid() const
{
    return m_debFilePtr->isValid();
}

bool DebPackage::fileExists() const
{
    return m_exists;
}

void DebPackage::markNotExists()
{
    m_exists = false;
}

QString DebPackage::filePath() const
{
    return m_debFilePtr->filePath();
}

QByteArray DebPackage::md5()
{
    if (m_md5.isEmpty() && m_debFilePtr->isValid()) {
        m_md5 = m_debFilePtr->md5Sum();
    }

    return m_md5;
}

const QSharedPointer<QApt::DebFile> &DebPackage::debInfo() const
{
    return m_debFilePtr;
}

PackageDependsStatus &DebPackage::dependsStatus()
{
    return m_dependsStatus;
}

bool DebPackage::containRemovePackages() const
{
    return !m_removePackages.isEmpty();
}

void DebPackage::setMarkedPackages(const QStringList &installDepends)
{
    m_removePackages.clear();
    if (!m_debFilePtr->isValid()) {
        return;
    }

    QApt::Backend *backend = PackageAnalyzer::instance().backendPtr();
    if (!backend) {
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
            if (package && package->isInstalled()) {
                m_removePackages << package->name();
            }
        }
    }
}

QStringList DebPackage::removePackages() const
{
    return m_removePackages;
}

void DebPackage::setError(int code, const QString &string)
{
    m_errorCode = code;
    m_errorString = string;
}

int DebPackage::errorCode() const
{
    return m_errorCode;
}

QString DebPackage::errorString() const
{
    return m_errorString;
}

};  // namespace Deb
