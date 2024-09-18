// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uab_package.h"
#include "uab_backend.h"
#include "utils/utils.h"

namespace Uab {

UabPackage::UabPackage(const UabPkgInfo::Ptr &metaPtr)
    : m_metaPtr(metaPtr)
{
    reset();
}

const UabPkgInfo::Ptr &UabPackage::info() const
{
    return m_metaPtr;
}

bool UabPackage::isValid() const
{
    if (!m_metaPtr) {
        return false;
    }

    if (m_metaPtr->id.isEmpty() || m_metaPtr->appName.isEmpty()) {
        return false;
    }

    return true;
}

Pkg::DependsStatus UabPackage::dependsStatus()
{
    return m_dependsStatus;
}

Pkg::PackageInstallStatus UabPackage::installStatus()
{
    return m_installStatus;
}

Pkg::PackageOperationStatus UabPackage::operationStatus()
{
    return m_operationStatus;
}

QString UabPackage::installedVersion()
{
    return m_installedVersion;
}

QString UabPackage::failedReason() const
{
    return m_failReason;
}

UabPackage::Ptr UabPackage::fromFilePath(const QString &filePath)
{
    QString error;
    auto infoPtr = Uab::UabBackend::packageFromMetaData(filePath, &error);
    auto uabPtr = Uab::UabPackage::Ptr::create(infoPtr);
    uabPtr->m_failReason = error;
    return uabPtr;
}

void UabPackage::reset()
{
    if (!m_metaPtr) {
        return;
    }

    m_operationStatus = Pkg::Prepare;

    m_installedVersion.clear();
    m_installStatus = Pkg::NotInstalled;

    if (Uab::UabBackend::instance()->backendInited()) {
        auto uabInfoPtr = Uab::UabBackend::instance()->findPackage(m_metaPtr->id);
        if (uabInfoPtr) {
            m_installedVersion = uabInfoPtr->version;
            const int ret = Utils::compareVersion(m_metaPtr->version, uabInfoPtr->version);

            if (ret == 0) {
                m_installStatus = Pkg::InstalledSameVersion;
            } else if (ret < 0) {
                m_installStatus = Pkg::InstalledLaterVersion;
            } else {
                m_installStatus = Pkg::InstalledEarlierVersion;
            }
        }
    }
}

}  // namespace Uab
