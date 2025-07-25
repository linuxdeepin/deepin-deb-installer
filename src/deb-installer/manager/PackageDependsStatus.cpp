// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PackageDependsStatus.h"
#include "utils/package_defines.h"
#include "utils/ddlog.h"

PackageDependsStatus PackageDependsStatus::ok()
{
    // qCDebug(appLog) << "Creating OK status";
    return {Pkg::DependsStatus::DependsOk, QString()};
}

PackageDependsStatus PackageDependsStatus::available(const QString &package)
{
    // qCDebug(appLog) << "Creating AVAILABLE status for package:" << package;
    // 修复卸载p7zip导致deepin-wine-helper被卸载的问题，Available 添加packageName
    return {Pkg::DependsStatus::DependsAvailable, package};
}

PackageDependsStatus PackageDependsStatus::_break(const QString &package)
{
    qCDebug(appLog) << "Creating BREAK status for package:" << package;
    return {Pkg::DependsStatus::DependsBreak, package};
}

PackageDependsStatus PackageDependsStatus::_prohibit(const QString &package)
{
    qCDebug(appLog) << "Creating PROHIBIT status for package:" << package;
    return {Pkg::DependsStatus::DependsBreak, package};
}

PackageDependsStatus::PackageDependsStatus()
    : PackageDependsStatus(Pkg::DependsStatus::DependsOk, QString())
{
    qCDebug(appLog) << "Default PackageDependsStatus created";
}

PackageDependsStatus::PackageDependsStatus(const int status, const QString &package)
    : status(status)
    , package(package)
{
    qCDebug(appLog) << "Creating PackageDependsStatus with status:" << status << "package:" << package;
}

PackageDependsStatus &PackageDependsStatus::operator=(const PackageDependsStatus &other)
{
    // qCDebug(appLog) << "Assigning PackageDependsStatus from package:" << other.package;
    status = other.status;
    package = other.package;

    return *this;
}

PackageDependsStatus PackageDependsStatus::max(const PackageDependsStatus &other)
{
    // qCDebug(appLog) << "Comparing PackageDependsStatus with status:" << other.status << "for package:" << other.package;
    if (other.status > status) {
        qCDebug(appLog) << "Upgrading status from" << status << "to" << other.status << "for package:" << other.package;
        *this = other;
    }

    return *this;
}

PackageDependsStatus PackageDependsStatus::maxEq(const PackageDependsStatus &other)
{
    // qCDebug(appLog) << "Comparing PackageDependsStatus with status:" << other.status << "for package:" << other.package;
    if (other.status >= status) {
        // qCDebug(appLog) << "Upgrading status from" << status << "to" << other.status << "for package:" << other.package;
        *this = other;
    }

    return *this;
}

PackageDependsStatus PackageDependsStatus::min(const PackageDependsStatus &other)
{
    // qCDebug(appLog) << "Comparing PackageDependsStatus with status:" << other.status << "for package:" << other.package;
    if (other.status < status) {
        // qCDebug(appLog) << "Downgrading status from" << status << "to" << other.status << "for package:" << other.package;
        *this = other;
    }

    return *this;
}

PackageDependsStatus PackageDependsStatus::minEq(const PackageDependsStatus &other)
{
    // qCDebug(appLog) << "Comparing PackageDependsStatus with status:" << other.status << "for package:" << other.package;
    if (other.status <= status) {
        // qCDebug(appLog) << "Downgrading status from" << status << "to" << other.status << "for package:" << other.package;
        *this = other;
    }

    return *this;
}

bool PackageDependsStatus::isBreak() const
{
    // qCDebug(appLog) << "Checking if package is break";
    bool result = status == Pkg::DependsStatus::DependsBreak;
    if (result) {
        qCDebug(appLog) << "Package" << package << "has broken dependencies";
    }
    return result;
}

bool PackageDependsStatus::isAuthCancel() const
{
    bool result = status == Pkg::DependsStatus::DependsAuthCancel;
    // qCDebug(appLog) << "Checking auth cancel status:" << result;
    return result;
}

bool PackageDependsStatus::isAvailable() const
{
    bool result = status == Pkg::DependsStatus::DependsAvailable;
    // qCDebug(appLog) << "Checking available status:" << result << "for package:" << package;
    return result;
}

bool PackageDependsStatus::isProhibit() const
{
    bool result = status == Pkg::DependsStatus::Prohibit;
    // qCDebug(appLog) << "Checking prohibit status:" << result << "for package:" << package;
    return result;
}

bool PackageDependsStatus::canInstall() const
{
    bool result = status == Pkg::DependsAvailable || status == Pkg::DependsOk;
    qCDebug(appLog) << "Package" << package << (result ? "can" : "cannot") << "be installed";
    return result;
}

bool PackageDependsStatus::canInstallCompatible() const
{
    bool result = status == Pkg::CompatibleNotInstalled;
    // qCDebug(appLog) << "Checking compatible install status:" << result << "for package:" << package;
    return result;
}
