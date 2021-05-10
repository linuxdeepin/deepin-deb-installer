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

#ifndef INSTALLER_H
#define INSTALLER_H

#include <QObject>

#include <QApt/Backend>
#include <QApt/DebFile>

class PackagesManager;
class Package;

class PackageInstaller: public QObject
{
    Q_OBJECT
public:
    PackageInstaller(QApt::Backend *b);

    virtual ~PackageInstaller();

    void appendPackage(Package *packages);

    void installPackage();

    void uninstallPackage();

signals:
    void signal_startInstall();

    void signal_installError(int, QString);

    void signal_installProgress(int progress);

    void signal_installDetailStatus(QString);

    void signal_installFinished(QApt::ExitStatus);

    void signal_uninstallFinished(QApt::ExitStatus);

    void signal_packageNotInstall();

private:
    bool isDpkgRunning();

    void dealBreakPackage();

    void dealAvailablePackage();

    void dealInstallablePackage();

    void installAvailableDepends();


private:
    Package *m_packages         = nullptr;

    QApt::Backend *m_backend    = nullptr;
    QApt::Transaction *m_pTrans = nullptr;
};

#endif // INSTALLER_H
