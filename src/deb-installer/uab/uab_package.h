// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UABPACKAGE_H
#define UABPACKAGE_H

#include "utils/package_defines.h"
#include "uab_defines.h"

namespace Uab {

class UabPackage
{
public:
    using Ptr = QSharedPointer<UabPackage>;

    explicit UabPackage(const Uab::UabPkgInfo::Ptr &metaPtr);

    const Uab::UabPkgInfo::Ptr &info() const;

    bool isValid() const;

    Pkg::DependsStatus dependsStatus();
    Pkg::PackageInstallStatus installStatus();
    Pkg::PackageOperationStatus operationStatus();

    QString installedVersion();
    QString failedReason() const;

    static UabPackage::Ptr fromFilePath(const QString &filePath);

private:
    void reset();

private:
    Uab::UabPkgInfo::Ptr m_metaPtr;

    Pkg::DependsStatus m_dependsStatus{Pkg::DependsOk};            // package's depends info
    Pkg::PackageInstallStatus m_installStatus{Pkg::NotInstalled};  // version check
    Pkg::PackageOperationStatus m_operationStatus{Pkg::Prepare};   // status for operation flow

    QString m_installedVersion;
    QString m_failReason;

    friend class UabPackageListModel;

    Q_DISABLE_COPY(UabPackage)
};

}  // namespace Uab

#endif  // UABPACKAGE_H
