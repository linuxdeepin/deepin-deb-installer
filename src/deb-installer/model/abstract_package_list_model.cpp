// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstract_package_list_model.h"
#include "utils/ddlog.h"

/**
   @class AbstractPackageListModel
   @brief Interface for the package data model and package installation control.
 */
AbstractPackageListModel::AbstractPackageListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    qCDebug(appLog) << "Constructing AbstractPackageListModel";
}

AbstractPackageListModel::WorkerStatus AbstractPackageListModel::getWorkerStatus() const
{
    qCDebug(appLog) << "Getting worker status:" << m_workerStatus;
    return m_workerStatus;
}

/**
 * @brief Set the worker `status` and emit the corresponding signal.
 */
void AbstractPackageListModel::setWorkerStatus(WorkerStatus status)
{
    qCDebug(appLog) << "Worker status changed from" << m_workerStatus << "to" << status;
    m_workerStatus = status;

    switch (status) {
        case WorkerProcessing:
            qCDebug(appLog) << "Worker status is Processing, emitting signalWorkerStart.";
            Q_FALLTHROUGH();
        case WorkerUnInstall:
            qCDebug(appLog) << "Worker status is UnInstall, emitting signalWorkerStart.";
            Q_EMIT signalWorkerStart();
            break;
        case WorkerFinished:
            qCDebug(appLog) << "Worker status is Finished, emitting signalWorkerFinished.";
            Q_EMIT signalWorkerFinished();
            break;
        default:
            qCDebug(appLog) << "Worker status is other, do nothing.";
            break;
    }
}
