/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "DealDependThread.h"
#include "model/deblistmodel.h"

DealDependThread::DealDependThread(QObject *parent)
{
    Q_UNUSED(parent);
    proc = new QProcess(this);
    connect(proc, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this, &DealDependThread::slotInstallFinished);
    connect(proc, &QProcess::readyReadStandardOutput, this, &DealDependThread::slotReadOutput);
}

DealDependThread::~DealDependThread()
{
    delete proc;
}

void DealDependThread::setDependsList(QStringList dependList, int index)
{
    m_index = index;
    m_dependsList = dependList;
}

void DealDependThread::setBrokenDepend(QString dependName)
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
        qWarning()<<"install Wine dependency Not authorized";
        emit signalDependResult(DebListModel::CancelAuth, m_index, m_brokenDepend);
    }
}

void DealDependThread::run()
{
    proc->setProcessChannelMode(QProcess::MergedChannels);
    msleep(100);

    emit signalDependResult(DebListModel::AuthBefore, m_index, m_brokenDepend);
    proc->start("pkexec", QStringList() << "deepin-deb-installer-dependsInstall"  << "InstallDeepinWine" << m_dependsList);
    emit signalEnableCloseButton(false);
}

void DealDependThread::slotInstallFinished(int num = -1)
{
    if (num == 0) {
        if (bDependsStatusErr) {
            emit signalDependResult(DebListModel::AnalysisErr, m_index, m_brokenDepend);
            bDependsStatusErr = false;
            return;
        }
        emit signalDependResult(DebListModel::AuthDependsSuccess, m_index, m_brokenDepend);
    } else {
        if (bDependsStatusErr) {
            emit signalDependResult(DebListModel::AnalysisErr, m_index, m_brokenDepend);
            bDependsStatusErr = false;
            return;
        }
        qWarning() << m_dependsList << "install error" << num;
        emit signalDependResult(DebListModel::AuthDependsErr, m_index, m_brokenDepend);
    }
    emit signalEnableCloseButton(true);
}
