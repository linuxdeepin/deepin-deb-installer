// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eventlogutils.h"
#include "ddlog.h"
#include <QLibrary>
#include <QDir>
#include <QLibraryInfo>
#include <QJsonDocument>

Eventlogutils *Eventlogutils::m_pInstance = nullptr;
Eventlogutils *Eventlogutils::GetInstance()
{
    qCDebug(appLog) << "Getting Eventlogutils instance";
    if (m_pInstance == nullptr) {
        qCDebug(appLog) << "Creating new Eventlogutils instance";
        m_pInstance = new Eventlogutils();
    }
    return m_pInstance;
}

void Eventlogutils::writeLogs(QJsonObject &data)
{
    qCDebug(appLog) << "Writing logs:" << data;
    if (!writeEventLogFunc) {
        qCWarning(appLog) << "writeEventLogFunc is not available, cannot write logs";
        return;
    }

    writeEventLogFunc(QJsonDocument(data).toJson(QJsonDocument::Compact).toStdString());
    qCDebug(appLog) << "Logs written successfully";
}

Eventlogutils::Eventlogutils()
{
    qCDebug(appLog) << "Constructing Eventlogutils";
    QLibrary library("libdeepin-event-log.so");
    initFunc = reinterpret_cast<bool (*)(const std::string &, bool)>(library.resolve("Initialize"));
    writeEventLogFunc = reinterpret_cast<void (*)(const std::string &)>(library.resolve("WriteEventLog"));

    if (!initFunc) {
        qCWarning(appLog) << "Failed to resolve Initialize function from libdeepin-event-log.so";
        return;
    }
    if (!writeEventLogFunc) {
        qCWarning(appLog) << "Failed to resolve WriteEventLog function from libdeepin-event-log.so";
    }

    qCDebug(appLog) << "Initializing event log with package name: deepin-deb-installer";
    initFunc("deepin-deb-installer", true);
    qCDebug(appLog) << "Eventlogutils constructed";
}
