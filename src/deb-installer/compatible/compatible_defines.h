// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPATIBLE_DEFINES_H
#define COMPATIBLE_DEFINES_H

#include <QString>
#include <QSharedPointer>

namespace Compatible {

enum CompCode {
    CompError = -1,
    CompSuccess = 0,
};

struct CompPkgInfo
{
    using Ptr = QSharedPointer<CompPkgInfo>;

    bool installed{false};  // installed in compatible rootfs
    QString filePath;       // for local file
    QString name;
    QString version;
    QString arch;    // architecture
    QString rootfs;  // rootFs name, which installed/target this package
};

struct RootfsInfo
{
    using Ptr = QSharedPointer<RootfsInfo>;

    int prioriy{0};
    QString name;
    QString osName;
};

}  // namespace Compatible

#endif  // COMPATIBLE_DEFINES_H
