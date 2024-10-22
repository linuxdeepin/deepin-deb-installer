// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UAB_DBUS_PACKAGE_MANAGER_H
#define UAB_DBUS_PACKAGE_MANAGER_H

#include <QObject>
#include <QScopedPointer>
#include <QDBusInterface>

#include "uab_package.h"

namespace Uab {

// DBus interface: org.deepin.linglong.PackageManager
class UabDBusPackageManager : public QObject
{
    Q_OBJECT
public:
    static UabDBusPackageManager *instance();

    [[nodiscard]] bool isValid();

    [[nodiscard]] bool installFormFile(const UabPackage::Ptr &installPtr);
    [[nodiscard]] bool uninstall(const UabPackage::Ptr &uninstallPtr);

    Q_SIGNAL void progressChanged(int progress, const QString &message);
    Q_SIGNAL void packageFinished(bool success);

private:
    // from linyaps task.h  linglong::service::InstallTask::Status
    enum Status {
        Queued,
        preInstall,
        installRuntime,
        installBase,
        installApplication,
        postInstall,
        Success,
        Failed,
        Canceled  // manualy cancel
    };

    UabDBusPackageManager();
    ~UabDBusPackageManager() override = default;

    void ensureInterface();
    [[nodiscard]] bool isRunning() const;

    Q_SLOT void onTaskChanged(const QString &recTaskID, const QString &percentage, const QString &message, int status);

    // linglnog dbus interface result
    struct DBusResult
    {
        QString taskId;
        int code{0};
        QString message;
    };
    DBusResult m_currentTask;

    QScopedPointer<QDBusInterface> m_interface;

    Q_DISABLE_COPY(UabDBusPackageManager)
};

}  // namespace Uab

#endif  // UAB_DBUS_PACKAGE_MANAGER_H
