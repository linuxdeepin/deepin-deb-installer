// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uab_dbus_package_manager.h"

#include <QDBusReply>
#include <QDBusUnixFileDescriptor>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QDebug>

namespace Uab {

const QString kllDBusService = "org.deepin.linglong.PackageManager";
const QString kllDBusPath = "/org/deepin/linglong/PackageManager";
const QString kllDBusInterface = "org.deepin.linglong.PackageManager1";
// method
const QString kllDBusInstallFromFile = "InstallFromFile";
const QString kllDBusInstall = "Install";
const QString kllDBusUninstall = "Uninstall";
const QString kllDBusUpdate = "Update";
const QString kllDBusSearch = "Search";
const QString kllDBusCancelTask = "CancelTask";
// signal
const QString kllDBusSignalTaskChanged = "TaskChanged";

// uninstall param keys
const QString kllParamPackage = "package";
const QString kllParamId = "id";
const QString kllParamVersion = "version";
const QString kllParamChannel = "channel";
const QString kllParamModule = "packageManager1PackageModule";

// result tag
const QString kllRetCode = "code";
const QString kllRetTaskId = "taskID";
const QString kllRetMessage = "message";

UabDBusPackageManager::UabDBusPackageManager() {}

void UabDBusPackageManager::ensureInterface()
{
    if (!m_interface.isNull()) {
        return;
    }

    QDBusConnection connection = QDBusConnection::systemBus();
    connection.connect(kllDBusService,
                       kllDBusPath,
                       kllDBusInterface,
                       kllDBusSignalTaskChanged,
                       this,
                       SLOT(onTaskChanged(const QString &, const QString &, const QString &, int)));

    m_interface.reset(new QDBusInterface(kllDBusService, kllDBusPath, kllDBusInterface, connection));
    if (!m_interface->isValid()) {
        qInfo() << qPrintable("LinglongPM dbus create fails:") << m_interface->lastError().message();
    }
}

bool UabDBusPackageManager::isRunning() const
{
    return !m_currentTask.taskId.isEmpty();
}

void UabDBusPackageManager::onTaskChanged(const QString &recTaskID, const QString &percentage, const QString &message, int status)
{
    qInfo() << QString("From LinglingPM, [task]: %1 [percentage]: %2 [message]: %3 [status]: %4")
                   .arg(recTaskID)
                   .arg(percentage)
                   .arg(message)
                   .arg(status);

    if (recTaskID != m_currentTask.taskId) {
        return;
    }

    Q_EMIT progressChanged(percentage.toInt(), message);

    switch (status) {
        case UabDBusPackageManager::Success:
            m_currentTask.taskId.clear();
            Q_EMIT packageFinished(true);
            break;
        case UabDBusPackageManager::Failed:
            m_currentTask.taskId.clear();
            Q_EMIT packageFinished(false);
            break;
        case UabDBusPackageManager::Canceled:
            m_currentTask.taskId.clear();
            break;
        default:
            break;
    }
}

UabDBusPackageManager *UabDBusPackageManager::instance()
{
    static UabDBusPackageManager ins;
    return &ins;
}

bool UabDBusPackageManager::isValid()
{
    ensureInterface();
    return m_interface->isValid();
}

/**
   @brief Install package \a installPtr ,
   @return True if call Linglong package manager install package file start. otherwise false.
 */
bool UabDBusPackageManager::installFormFile(const UabPackage::Ptr &installPtr)
{
    if (!installPtr || !installPtr->isValid() || isRunning()) {
        return false;
    }
    ensureInterface();

    QFile file(installPtr->info()->filePath);
    if (!file.open(QFile::ReadOnly | QFile::ExistingOnly)) {
        return false;
    }

    const QDBusUnixFileDescriptor dbusFd(file.handle());
    QDBusReply<QVariantMap> reply =
        m_interface->call(kllDBusInstallFromFile, QVariant::fromValue(dbusFd), QFileInfo(file.fileName()).suffix());
    if (reply.error().isValid()) {
        qWarning() << qPrintable("call LinglongPM dbus fails:") << reply.error().message();
        return false;
    }

    const QVariantMap data = reply.value();
    const int code = data.value(kllRetCode).toInt();
    if (Uab::UabSuccess != code) {
        qWarning() << QString("LinglingPM return error: [%1] %2").arg(code).arg(data.value(kllRetMessage).toString());
        return false;
    }

    m_currentTask.taskId = data.value(kllRetTaskId).toString();
    m_currentTask.code = code;
    m_currentTask.message = data.value(kllRetMessage).toString();

    qInfo() << QString("Requset LinglingPM install: %1/%2 [task]: %3 [message]: %4")
                   .arg(installPtr->info()->id)
                   .arg(installPtr->info()->version)
                   .arg(m_currentTask.taskId)
                   .arg(m_currentTask.message);

    Q_EMIT progressChanged(0, m_currentTask.message);
    return true;
}

bool UabDBusPackageManager::uninstall(const UabPackage::Ptr &uninstallPtr)
{
    if (!uninstallPtr || !uninstallPtr->isValid() || isRunning()) {
        return false;
    }
    ensureInterface();

    QVariantMap pkgParams;
    pkgParams[kllParamId] = uninstallPtr->info()->id;
    pkgParams[kllParamVersion] = uninstallPtr->info()->version;
    pkgParams[kllParamChannel] = uninstallPtr->info()->channel;
    pkgParams[kllParamModule] = uninstallPtr->info()->module;
    QVariantMap params;
    params[kllParamPackage] = pkgParams;

    QDBusReply<QVariantMap> reply = m_interface->call(kllDBusUninstall, params);
    if (reply.error().isValid()) {
        qWarning() << qPrintable("call LinglongPM dbus fails:") << reply.error().message();
        return false;
    }

    const QVariantMap data = reply.value();
    const int code = data.value(kllRetCode).toInt();
    if (Uab::UabSuccess != code) {
        qWarning() << QString("LinglingPM return error: [%1] %2").arg(code).arg(data.value(kllRetMessage).toString());
        return false;
    }

    m_currentTask.taskId = data.value(kllRetTaskId).toString();
    m_currentTask.code = code;
    m_currentTask.message = data.value(kllRetMessage).toString();

    qInfo() << QString("Requset LinglingPM uninstall: %1/%2 [message]: %3")
                   .arg(uninstallPtr->info()->id)
                   .arg(uninstallPtr->info()->version)
                   .arg(m_currentTask.message);

    QTimer::singleShot(0, this, [this]() {
        Q_EMIT progressChanged(100, m_currentTask.message);
        Q_EMIT packageFinished(true);
    });
    // uninstall not need receive task info, remove imdealtly and fast
    return true;
}

}  // namespace Uab
