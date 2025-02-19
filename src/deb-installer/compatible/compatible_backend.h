// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPATIBLEBACKEND_H
#define COMPATIBLEBACKEND_H

#include <QObject>
#include <QHash>

#include "compatible_defines.h"

class QProcess;

namespace Compatible {

/*
    Warning: Compatibility mode is temporary deprecated, this class is invalid forever. see macro DISABLE_COMPATIBLE.
             However, there is still a high probability that this module will be enabled later, so the code is retained.
             To restore it, remove the DISABLE_COMPATIBLE macro.
 */
// #define DISABLE_COMPATIBLE

// Backend for comaptible mode package manage.
#ifdef DISABLE_COMPATIBLE
class QT_DEPRECATED_X("Temporary disable compatibility mode!") CompatibleBackend : public QObject
#else
class CompatibleBackend : public QObject
#endif
{
    Q_OBJECT

public:
    static CompatibleBackend *instance();

    void initBackend(bool async = true);
    Q_SIGNAL void compatibleInitFinished();

#ifdef DISABLE_COMPATIBLE
    [[nodiscard]] bool compatibleValid() const { return false; }
    [[nodiscard]] bool compatibleInited() const { return false; }
    [[nodiscard]] bool compatibleExists() const { return false; }
    bool recheckCompatibleExists() { return false; }
#else
    [[nodiscard]] bool compatibleValid() const;
    [[nodiscard]] bool compatibleInited() const;
    [[nodiscard]] bool compatibleExists() const;
    bool recheckCompatibleExists();
#endif

    [[nodiscard]] QList<RootfsInfo::Ptr> rootfsList() const;
    [[nodiscard]] QString osName(const QString &rootfsName) const;
    [[nodiscard]] CompPkgInfo::Ptr containsPackage(const QString &packageName);

    // update backend database after controller process finished.
    void packageInstalled(const CompPkgInfo::Ptr &appendPtr);
    void packageRemoved(const CompPkgInfo::Ptr &removePtr);

    // use app check support rootfs with special package
    [[nodiscard]] bool supportAppCheck() const;
    [[nodiscard]] bool checkPackageSupportRootfs(const CompPkgInfo::Ptr &checkPtr);
    Q_SIGNAL void packageSupportRootfsChanged(const CompPkgInfo::Ptr &checkPtr);

private:
    explicit CompatibleBackend(QObject *parent = nullptr);
    ~CompatibleBackend() override = default;

    void initFinished(const QList<RootfsInfo::Ptr> &rootfsList, const QHash<QString, CompPkgInfo::Ptr> &packages);
    // parse raw output data
    static void backendProcessWithRaw(CompatibleBackend *backend);
    [[nodiscard]] static QList<RootfsInfo::Ptr> parseRootfsFromRawOutputV1(const QByteArray &output);
    [[nodiscard]] static QList<RootfsInfo::Ptr> parseRootfsFromRawOutputV2(const QByteArray &output);
    [[nodiscard]] static QHash<QString, CompPkgInfo::Ptr> parseAppListFromRawOutput(const QByteArray &output);
    // parse json output data
    static void backendProcessWithJson(CompatibleBackend *backend);

    bool m_init{false};
    bool m_compatibleExists{false};
    QList<RootfsInfo::Ptr> m_rootfsList;          // rootfs list
    QHash<QString, CompPkgInfo::Ptr> m_packages;  // all packages, every package is unique

    Q_DISABLE_COPY(CompatibleBackend)
};

}  // namespace Compatible

using CompBackend = Compatible::CompatibleBackend;

#endif  // COMPATIBLEBACKEND_H
