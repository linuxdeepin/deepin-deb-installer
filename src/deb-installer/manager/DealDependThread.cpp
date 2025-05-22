// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DealDependThread.h"
#include "model/deblistmodel.h"
#include "utils/hierarchicalverify.h"
#include "utils/ddlog.h"

DealDependThread::DealDependThread(QObject *parent)
{
    Q_UNUSED(parent);
    qCDebug(appLog) << "DealDependThread created";
    proc = new QProcess(this);
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &DealDependThread::slotInstallFinished);
    connect(proc, &QProcess::readyReadStandardOutput, this, &DealDependThread::slotReadOutput);
}

DealDependThread::~DealDependThread()
{
    delete proc;
}

void DealDependThread::setDependsList(const QStringList &dependList, int index)
{
    m_index = index;
    m_dependsList = dependList;
}

void DealDependThread::setBrokenDepend(const QString &dependName)
{
    m_brokenDepend = dependName;
}

void DealDependThread::slotReadOutput()
{
    QString tmp = proc->readAllStandardOutput().data();

    if (tmp.contains("StartInstallDeepinwine")) {
        emit signalDependResult(DebListModel::AuthConfirm, m_index, m_brokenDepend);
        return;
    }
    if (tmp.contains("Not authorized")) {
        bDependsStatusErr = true;
        qCWarning(appLog) << "Wine dependency install not authorized";
        emit signalDependResult(DebListModel::CancelAuth, m_index, m_brokenDepend);
    }

    // 检测是否包含分级验签错误信息
    if (HierarchicalVerify::instance()->checkTransactionError(m_brokenDepend, tmp)) {
        bVerifyStatusErr = true;
        qCWarning(appLog) << "Hierarchical verify failed for wine dependency. Output:" << tmp;
        // 结束后统一发送信号
    }
}

void DealDependThread::run()
{
    qCDebug(appLog) << "Starting dependency thread run";
    proc->setProcessChannelMode(QProcess::MergedChannels);
    msleep(100);

    bDependsStatusErr = false;
    bVerifyStatusErr = false;

    emit signalDependResult(DebListModel::AuthBefore, m_index, m_brokenDepend);
    proc->start("pkexec",
                QStringList() << "deepin-deb-installer-dependsInstall"
                              << "--install_wine" << m_dependsList);

    qCDebug(appLog) << "starting command: pkexec deepin-deb-installer-dependsInstall --install_wine" << m_dependsList;
    emit signalEnableCloseButton(false);
}

void DealDependThread::slotInstallFinished(int num = -1)
{
    if (bDependsStatusErr) {
        emit signalDependResult(DebListModel::AnalysisErr, m_index, m_brokenDepend);
        bDependsStatusErr = false;
        qCDebug(appLog) << "Dependency install failed. Dependencies:" << m_dependsList;
        return;
    }

    if (bVerifyStatusErr) {
        emit signalDependResult(DebListModel::VerifyDependsErr, m_index, m_brokenDepend);
        bVerifyStatusErr = false;
        qCDebug(appLog) << "Hierarchical verify failed for wine dependency. Exit code:" << num << "Dependencies:" << m_dependsList;
        return;
    }

    if (num == 0) {
        qCDebug(appLog) << "Dependency install succeeded. Exit code:" << num << "Dependencies:" << m_dependsList;
        emit signalDependResult(DebListModel::AuthDependsSuccess, m_index, m_brokenDepend);
    } else {
        qCWarning(appLog) << "Dependency install failed. Exit code:" << num << "Dependencies:" << m_dependsList;
        emit signalDependResult(DebListModel::AuthDependsErr, m_index, m_brokenDepend);
    }
    emit signalEnableCloseButton(true);

    qCDebug(appLog) << "Dependency install process finished. Exit code:" << num << "Dependencies:" << m_dependsList;
}
