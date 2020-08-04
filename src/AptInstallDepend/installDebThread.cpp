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

void InstallDebThread::setParam(QStringList tParam)
{
    m_listParam = tParam;
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
    if (m_listParam.size() > 0) {
        if (m_listParam[0] == "InstallDeepinWine") {
            qDebug() << "StartInstallDeepinwine";

            m_proc->start("sudo", QStringList() << "apt-get"
                          << "install"
                          << "deepin-wine"
                          << "deepin-wine-helper"
                          << "--fix-missing"
                          << "-y");
            m_proc->waitForFinished(-1);
            m_proc->close();
        } else if (m_listParam[0] == "InstallConfig") {
            if (m_listParam.size() <= 1)
                return;

            qDebug() << "StartInstallAptConfig";

            m_proc->start("sudo", QStringList() << "-S" <<  "dpkg" << "-i" << m_listParam[1]);
            m_proc->waitForFinished(1500);

            char c_input[20];
            while (fgets(c_input, 10, stdin)) {
                QString str = c_input;
                str.remove(QChar('\\'), Qt::CaseInsensitive);
                str.remove(QChar('"'), Qt::CaseInsensitive);

                m_proc->write(str.toLatin1().data());

                m_proc->waitForFinished(1500);

                QProcess::ProcessState tmp = m_proc->state();
                if (tmp == QProcess::ProcessState::NotRunning) {
                    m_proc->close();
                    break;
                }
            }

            m_proc->close();
        }
    }
}
