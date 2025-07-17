// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uab_package.h"
#include "uab_backend.h"
#include "utils/utils.h"
#include "utils/ddlog.h"

namespace Uab {

/**
 * @class UabPackage
 * @brief Uab package, contains package information and runtime status.
 *        Store version, install status, operation status, error code, and error message.
 */
UabPackage::UabPackage(const UabPkgInfo::Ptr &metaPtr)
    : m_metaPtr(metaPtr)
{
    qCDebug(appLog) << "Creating UabPackage for:" << (metaPtr ? metaPtr->id : "null");
    reset();
}

const UabPkgInfo::Ptr &UabPackage::info() const
{
    qCDebug(appLog) << "Getting package info for:" << (m_metaPtr ? m_metaPtr->id : "null");
    return m_metaPtr;
}

bool UabPackage::isValid() const
{
    qCDebug(appLog) << "Checking if package is valid:" << (m_metaPtr ? m_metaPtr->id : "null");
    if (!m_metaPtr || !fileExists()) {
        qCDebug(appLog) << "Package is invalid: null meta pointer or file does not exist";
        return false;
    }

    if (m_metaPtr->id.isEmpty() || m_metaPtr->appName.isEmpty()) {
        qCDebug(appLog) << "Package is invalid: empty id or app name";
        return false;
    }

    qCDebug(appLog) << "Package is valid:" << m_metaPtr->id;
    return true;
}

void UabPackage::setDependsStatus(Pkg::DependsStatus status)
{
    qCDebug(appLog) << "Setting depends status for package:" << (m_metaPtr ? m_metaPtr->id : "null")
                 << "from" << m_dependsStatus << "to" << status;
    m_dependsStatus = status;
    switch (m_dependsStatus) {
        case Pkg::DependsBreak:
            m_failReason = QObject::tr("The system has not installed Linglong environment, please install it first");
            qCWarning(appLog) << "Package dependency broken:" << (m_metaPtr ? m_metaPtr->id : "null");
            break;
        default:
            qCDebug(appLog) << "Package dependency status OK:" << (m_metaPtr ? m_metaPtr->id : "null");
            break;
    }
}

void UabPackage::setProcessError(Pkg::ErrorCode err, const QString &errorString)
{
    qCWarning(appLog) << "Setting process error for package:" << (m_metaPtr ? m_metaPtr->id : "null")
                  << "Code:" << err << "Message:" << errorString;
    m_errorCode = err;
    m_processError = errorString;
}

bool UabPackage::fileExists() const
{
    qCDebug(appLog) << "Checking if file exists for package:" << (m_metaPtr ? m_metaPtr->id : "null") << m_exists;
    return m_exists;
}

void UabPackage::markNotExists()
{
    qCDebug(appLog) << "Marking package as not existing:" << (m_metaPtr ? m_metaPtr->id : "null");
    m_exists = false;
}

Pkg::DependsStatus UabPackage::dependsStatus() const
{
    qCDebug(appLog) << "Getting depends status for package:" << (m_metaPtr ? m_metaPtr->id : "null") << m_dependsStatus;
    return m_dependsStatus;
}

Pkg::PackageInstallStatus UabPackage::installStatus() const
{
    qCDebug(appLog) << "Getting install status for package:" << (m_metaPtr ? m_metaPtr->id : "null") << m_installStatus;
    return m_installStatus;
}

Pkg::PackageOperationStatus UabPackage::operationStatus() const
{
    qCDebug(appLog) << "Getting operation status for package:" << (m_metaPtr ? m_metaPtr->id : "null") << m_operationStatus;
    return m_operationStatus;
}

Pkg::ErrorCode UabPackage::errorCode() const
{
    qCDebug(appLog) << "Getting error code for package:" << (m_metaPtr ? m_metaPtr->id : "null") << m_errorCode;
    return m_errorCode;
}

QString UabPackage::installedVersion() const
{
    qCDebug(appLog) << "Getting installed version for package:" << (m_metaPtr ? m_metaPtr->id : "null") << m_installedVersion;
    return m_installedVersion;
}

QString UabPackage::failedReason() const
{
    qCDebug(appLog) << "Getting failed reason for package:" << (m_metaPtr ? m_metaPtr->id : "null");
    if (m_failReason.isEmpty()) {
        qCDebug(appLog) << "No specific fail reason, returning process error:" << m_processError;
        return m_processError;
    }

    qCDebug(appLog) << "Returning fail reason:" << m_failReason;
    return m_failReason;
}

QString UabPackage::processError() const
{
    qCDebug(appLog) << "Getting process error for package:" << (m_metaPtr ? m_metaPtr->id : "null") << m_processError;
    return m_processError;
}

UabPackage::Ptr UabPackage::fromInfo(const UabPkgInfo::Ptr &infoPtr)
{
    qCDebug(appLog) << "Creating UabPackage from info object for package:" << (infoPtr ? infoPtr->id : "null");
    return Uab::UabPackage::Ptr::create(infoPtr);
}

UabPackage::Ptr UabPackage::fromFilePath(const QString &filePath)
{
    qCDebug(appLog) << "Creating UabPackage from file path:" << filePath;
    QString error;
    auto infoPtr = Uab::UabBackend::packageFromMetaData(filePath, &error);
    auto uabPtr = Uab::UabPackage::Ptr::create(infoPtr);

    if (!error.isEmpty()) {
        qCWarning(appLog) << qPrintable("Uab from path:") << error;
    }
    qCDebug(appLog) << "UabPackage created from file path:" << filePath;
    return uabPtr;
}

void UabPackage::reset()
{
    qCDebug(appLog) << "Resetting package state";
    if (!m_metaPtr) {
        qCWarning(appLog) << "Cannot reset package with null meta pointer";
        return;
    }

    qCDebug(appLog) << "Resetting package state:" << m_metaPtr->id;
    m_operationStatus = Pkg::Prepare;

    m_installedVersion.clear();
    m_installStatus = Pkg::NotInstalled;

    m_errorCode = Pkg::NoError;
    m_processError.clear();

    if (!Uab::UabBackend::instance()->linglongExists()) {
        qCWarning(appLog) << "Linglong environment not found for package:" << m_metaPtr->id;
        setDependsStatus(Pkg::DependsBreak);
    } else {
        qCDebug(appLog) << "Linglong environment exists for package:" << m_metaPtr->id;
        setDependsStatus(Pkg::DependsOk);
    }

    if (Uab::UabBackend::instance()->backendInited()) {
        qCDebug(appLog) << "Checking installed version for package:" << m_metaPtr->id;
        auto uabInfoPtr = Uab::UabBackend::instance()->findPackage(m_metaPtr->id);
        if (uabInfoPtr) {
            m_installedVersion = uabInfoPtr->version;
            const int ret = Utils::compareVersion(m_metaPtr->version, uabInfoPtr->version);

            if (ret == 0) {
                m_installStatus = Pkg::InstalledSameVersion;
                qCDebug(appLog) << "Package version matches installed:" << m_metaPtr->id << m_metaPtr->version;
            } else if (ret < 0) {
                m_installStatus = Pkg::InstalledLaterVersion;
                qCDebug(appLog) << "Package version is older than installed:" << m_metaPtr->id
                            << m_metaPtr->version << "<" << uabInfoPtr->version;
            } else {
                m_installStatus = Pkg::InstalledEarlierVersion;
                qCDebug(appLog) << "Package version is newer than installed:" << m_metaPtr->id
                            << m_metaPtr->version << ">" << uabInfoPtr->version;
            }
        } else {
            qCDebug(appLog) << "No installed version found for package:" << m_metaPtr->id;
        }
    } else {
        qCDebug(appLog) << "Backend not initialized, skipping version check for package:" << m_metaPtr->id;
    }
}

}  // namespace Uab
