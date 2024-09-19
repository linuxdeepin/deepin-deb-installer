// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PACKAGE_DEFINES_H
#define PACKAGE_DEFINES_H

#include <QString>
#include <QPair>
#include <QList>
#include <QMetaType>

/**
  @brief define package common struct / enum , both deb and uab package.
 */
namespace Pkg {

struct DependInfo
{
    QString packageName;  // depends package name
    QString version;      // depends pacakge version
};
using DependsPair = QPair<QList<DependInfo>, QList<DependInfo>>;

enum PackageType {
    UnknownPackage,
    Uab,  // ensure uab priority
    Deb,
    Ddim,
    Proxy,  // both deb and uab
};

enum PackageReadability {
    PkgReadable,
    PkgNotInLocal,
    PkgNoPermission,
};

enum AppendFailReason {
    PackageInvalid,
    PackageNotLocal,
    PackageNotInstallable,  // can not install, wrong arch or no permission etc.
    PackageAlreadyExists,   // same name and version package intalled
    PackageNotDdim,
};

enum PackageInstallStatus {
    NotInstalled,
    InstalledSameVersion,     // current version installed
    InstalledEarlierVersion,  // earlier version installed
    InstalledLaterVersion,    // later version installed
};

enum DependsStatus {
    DependsOk,            // ready to install
    DependsAvailable,     // alailable but need download depends
    DependsBreak,         // depends are unavaliable, confilcit, or no depends packge etc.
    DependsVerifyFailed,  // signature verify failed
    DependsAuthCancel,    // pre depends (wine, linglong) auth check failed
    ArchBreak,            // arch check failed, e.g.: amd64 package cannot install in arm system
    Prohibit,             // The application is restricted by the domain management and cannot be installed
};

enum PackageOperationStatus {
    Prepare,    // prepare install/uninstall
    Operating,  // installing/uninstalling
    Success,
    Failed,
    Waiting,       // waitng for next intall/uninstall
    VerifyFailed,  // runtime signature verfiy failed (hierarchical verify)
};

}  // namespace Pkg

Q_DECLARE_METATYPE(Pkg::DependsPair)

#endif  // PACKAGE_DEFINES_H
