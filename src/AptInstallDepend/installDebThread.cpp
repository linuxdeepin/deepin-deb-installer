/*
* Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
*
* Author:     liupeng <liupeng@uniontech.com>
* Maintainer: liupeng <liupeng@uniontech.com>
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
#include "installDebThread.h"
#include <QDebug>

InstallDebThread::InstallDebThread()
{
    m_proc = new QProcess;
    connect(m_proc, SIGNAL(finished(int)), this, SLOT(onFinished(int)));
    connect(m_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(on_readoutput()));
}

InstallDebThread::~InstallDebThread()
{
    if (m_proc != nullptr) {
        delete m_proc;
        m_proc = nullptr;
    }
}

void InstallDebThread::on_readoutput()
{
    QString tmp = m_proc->readAllStandardOutput().data();
    qDebug() << tmp;
}

void InstallDebThread::onFinished(int num)
{
    m_resultFlag = num;
}

void InstallDebThread::run()
{
    m_proc->start("sudo", QStringList() << "apt-get"
                                        << "install"
                                        << "deepin-wine"
                                        << "deepin-wine-helper"
                                        << "--fix-missing"
                                        << "-y");
    m_proc->waitForFinished(-1);
    m_proc->close();
}
