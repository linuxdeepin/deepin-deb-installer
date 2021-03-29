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

#ifndef GETSTATUSTHREAD_H
#define GETSTATUSTHREAD_H

#include "PackageStatus.h"

#include <QObject>
#include <QThread>


class Package;
class GetStatusThread : public QThread
{
    Q_OBJECT
public:

    GetStatusThread(PackageStatus *);

    void run();

    void setPackage(int index, QString packagePath);

signals:

    void signal_dependsStatus(int, DependsStatus);
    void signal_installStatus(int, InstallStatus);
private:
    int m_index = -1;
    PackageStatus *m_pPackageStatus;
    QString m_packagePath;
};

#endif // GETSTATUSTHREAD_H
