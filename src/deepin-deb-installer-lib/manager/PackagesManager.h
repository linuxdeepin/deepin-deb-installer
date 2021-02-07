/*
* Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
*
* Author:     cuizhen <cuizhen@uniontech.com>
* Maintainer: cuizhen <cuizhen@uniontech.com>
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

#ifndef PackagesManager_H
#define PackagesManager_H
#include "result.h"
#include "status/PackageStatus.h"

#include <QObject>
#include <QFuture>

#include <QApt/Transaction>

class PackageSigntureStatus;
class Package;
class PackageInstaller;

typedef Result<QString> ConflictResult;

class GetStatusThread;
class PackagesManager : public QObject
{
    friend class installer;
    Q_OBJECT
public:
    PackagesManager();

    virtual ~PackagesManager();

public:

    bool checkPackageValid(int index = 0);

    bool checkPackageSignture(int index = 0);

    bool checkPackageDependsStatus(int index = 0);

    int  checkInstallStatus(int index = 0);

public:
    void appendPackages(QStringList packages);

    void removePackage(int index);

signals:
    void signal_backendError();

    void signal_packageInvalid(int);

    void signal_signatureError(int, int);

    void signal_dependStatusError(int, int);

    void signal_addPackageSuccess(int);

    void signal_removePackageSuccess(int);

    void signal_packageAlreadyExits(int);

    void signal_invalidIndex(int);

    void signal_packageNotInstalled(int);

signals:
    void signal_startInstallPackages();

    void signal_installProgress(int progress);

    void signal_installDetailStatus(QString);

    void signal_installFinished();

    void signal_uninstallFinished();

    void signal_installErrorOccured(int, QString);

public:
    void install();

    void uninstall(int index);

private slots:
    void slot_installFinished(QApt::ExitStatus);

    void slot_uninstallFinished(QApt::ExitStatus);

    void slot_getDependsStatus(int, DependsStatus);

    void slot_getInstallStatus(int, InstallStatus);
private:
    QList<Package *> m_packages;

    Package *searchByIndex(int index = 0);

    QSet<QByteArray> m_packagesMd5;

    PackageStatus      *m_pPackageStatus;
    PackageInstaller   *m_pPackageInstaller;

    GetStatusThread    *m_pGetStatusThread;

    bool                m_appendFinished = false;

private:
    void getPackageInfo(QString pkg, int index = 0);

    bool checkPackageSuffix(QString packagePath);

    void initConnection();

};

#endif // PackagesManager_H
