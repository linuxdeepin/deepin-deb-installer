/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
*
* Author:     cuizhen <cuizhen@uniontech.com>
* Maintainer:  cuizhen <cuizhen@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "DealDependThread.h"
#include "model/deblistmodel.h"

DealDependThread::DealDependThread(QObject *parent)
{
    Q_UNUSED(parent);
    proc = new QProcess(this);
    connect(proc, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this, &DealDependThread::onFinished);
    connect(proc, &QProcess::readyReadStandardOutput, this, &DealDependThread::on_readoutput);
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
void DealDependThread::on_readoutput()
{
    QString tmp = proc->readAllStandardOutput().data();
    qDebug() << "安装依赖输出数据:" << tmp;

    if (tmp.contains("StartInstallDeepinwine")) {
        emit DependResult(DebListModel::AuthConfirm, m_index, m_brokenDepend);
        return;
    }

    if (tmp.contains("Not authorized")) {
        bDependsStatusErr = true;
        emit DependResult(DebListModel::CancelAuth, m_index, m_brokenDepend);
    }
}

void DealDependThread::onFinished(int num = -1)
{
    if (bDependsStatusErr) {
        bDependsStatusErr = false;
        return;
    }

    if (num == 0) {
        if (bDependsStatusErr) {
            emit DependResult(DebListModel::AnalysisErr, m_index, m_brokenDepend);
            bDependsStatusErr = false;
            return;
        }
        qDebug() << m_dependsList << "安装成功";
        emit DependResult(DebListModel::AuthDependsSuccess, m_index, m_brokenDepend);
    } else {
        if (bDependsStatusErr) {
            emit DependResult(DebListModel::AnalysisErr, m_index, m_brokenDepend);
            bDependsStatusErr = false;
            return;
        }
        qDebug() << m_dependsList << "install error" << num;
        emit DependResult(DebListModel::AuthDependsErr, m_index, m_brokenDepend);
    }
    emit enableCloseButton(true);
}

void DealDependThread::run()
{
    proc->setProcessChannelMode(QProcess::MergedChannels);
    msleep(100);

    emit DependResult(DebListModel::AuthBefore, m_index, m_brokenDepend);
    qDebug() << "run m_dependList" << m_dependsList;
    proc->start("pkexec", QStringList() << "deepin-deb-installer-dependsInstall"  << "InstallDeepinWine" << m_dependsList);
    emit enableCloseButton(false);
}
