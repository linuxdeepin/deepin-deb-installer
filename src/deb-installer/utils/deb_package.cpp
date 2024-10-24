// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deb_package.h"
#include "compatible/compatible_backend.h"
#include "compatible/compatible_defines.h"
#include "utils/utils.h"

namespace Deb {

DebPackage::DebPackage(const QString &debFilePath)
{
    m_debFilePtr = QSharedPointer<QApt::DebFile>::create(debFilePath);
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

void DebPackage::setOperationStatus(Pkg::PackageOperationStatus s)
{
    m_operationStatus = s;
}

Pkg::PackageOperationStatus DebPackage::operationStatus() const
{
    return m_operationStatus;
}

Pkg::PackageInstallStatus DebPackage::installStatus() const
{
    return m_installStatus;
}

PackageDependsStatus &DebPackage::dependsStatus()
{
    return m_dependsStatus;
}

bool DebPackage::containsTemplates()
{
    return Utils::checkPackageContainsDebConf(m_debFilePtr->filePath());
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

const QSharedPointer<Compatible::CompPkgInfo> &DebPackage::compatible()
{
    if (!m_compInfoPtr && isValid()) {
        // might installed in compatbile mode.
        m_compInfoPtr = CompBackend::instance()->containsPackage(m_debFilePtr->packageName());

        if (!m_compInfoPtr) {
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
