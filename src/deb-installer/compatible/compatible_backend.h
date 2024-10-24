// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPATIBLEBACKEND_H
#define COMPATIBLEBACKEND_H

#include <QObject>

#include "compatible_defines.h"

class QProcess;

namespace Compatible {

// Backend for comaptible mode package manage.
class CompatibleBackend : public QObject
{
    Q_OBJECT

public:
    static CompatibleBackend *instance();

    void initBackend(bool async = true);
    [[nodiscard]] bool compatibleValid() const;
    Q_SIGNAL void compatibleInitFinished();

    [[nodiscard]] bool compatibleExists() const;
    bool recheckCompatibleExists();

    [[nodiscard]] QList<QPair<QString, QString>> osNameList() const;
    [[nodiscard]] CompPkgInfo::Ptr containsPackage(const QString &packageName);

    // update backend database after controller process finished.
    void packageInstalled(const CompPkgInfo::Ptr &appendPtr);
    void packageRemoved(const CompPkgInfo::Ptr &removePtr);

private:
    explicit CompatibleBackend(QObject *parent = nullptr);
    ~CompatibleBackend() override = default;

    void initFinished(const QList<RootfsInfo::Ptr> &rootfsList, const QHash<QString, CompPkgInfo::Ptr> &packages);
    static void backendProcess(CompatibleBackend *backend);
    [[nodiscard]] static QList<RootfsInfo::Ptr> parseRootfsFromRawOutput(const QByteArray &output);
    [[nodiscard]] static QHash<QString, CompPkgInfo::Ptr> parseAppListFromRawOutput(const QByteArray &output);

    bool m_init{false};
    bool m_compatibleExists{false};
    QList<RootfsInfo::Ptr> m_rootfsList;          // rootfs list
    QHash<QString, CompPkgInfo::Ptr> m_packages;  // all packages, every package is unique

    Q_DISABLE_COPY(CompatibleBackend)
};

}  // namespace Compatible

using CompBackend = Compatible::CompatibleBackend;

#endif  // COMPATIBLEBACKEND_H
