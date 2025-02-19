// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPATIBLE_DEFINES_H
#define COMPATIBLE_DEFINES_H

#include <QList>
#include <QString>
#include <QSharedPointer>

namespace Compatible {

enum CompResultCode {
    CompSuccess = 0,
    CompError = -1,
};

struct RootfsInfo
{
    using Ptr = QSharedPointer<RootfsInfo>;

    int prioriy{0};
    QString name;
    QString osName;
};

struct CompPkgInfo
{
    using Ptr = QSharedPointer<CompPkgInfo>;

    QString filePath;  // for local file
    QString name;
    QString version;
    QString arch;          // architecture
    QString rootfs;        // rootfs name, which installed this package
    QString targetRootfs;  // target rootfs, prepare for install pacakge to specific rootfs

    bool checked{false};                   // mark check current support rootfs
    QList<RootfsInfo::Ptr> supportRootfs;  // current support rootfs list (use deepin-compatible-ctl app check)

    inline bool installed() const { return !rootfs.isEmpty(); }
};

}  // namespace Compatible

#endif  // COMPATIBLE_DEFINES_H
