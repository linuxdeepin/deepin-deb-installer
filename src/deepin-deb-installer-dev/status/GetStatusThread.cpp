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

#include "GetStatusThread.h"
#include "package/Package.h"

#include <QMetaType>

#include <QFile>

GetStatusThread::GetStatusThread(PackageStatus *packageStatus)
{
    qRegisterMetaType<DependsStatus>("DependsStatus");//注册PortConfig类型
    qRegisterMetaType<InstallStatus>("InstallStatus");
    m_pPackageStatus = packageStatus;
}

void GetStatusThread::setPackage(int index, QString packagePath)
{
    m_index  = index;
    m_packagePath = packagePath;
}

void GetStatusThread::run()
{
    QFile debFile(m_packagePath);

    if (!debFile.exists()) {
        qWarning() << "GetStatusThread" << "run" << "getPackageDeoebdsStatus" << "文件不存在";
    }
    DependsStatus DependsStatus = m_pPackageStatus->getPackageDependsStatus(m_packagePath);

    emit signal_dependsStatus(m_index, DependsStatus);
    InstallStatus installStatus = m_pPackageStatus->getPackageInstallStatus(m_packagePath);
    emit signal_installStatus(m_index, installStatus);
}
