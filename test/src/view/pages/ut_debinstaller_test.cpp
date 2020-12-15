/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:     linxun <linxun@uniontech.com>
* Maintainer:  linxun <linxun@uniontech.com>
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

#include "../deb_installer/view/pages/debinstaller.h"
#include "../deb_installer/view/widgets/TitleBarFocusMonitor.h"
#include "../deb_installer/model/deblistmodel.h"
#include "../deb_installer/view/pages/multipleinstallpage.h"
#include "../deb_installer/view/pages/singleinstallpage.h"
#include "../deb_installer/manager/packagesmanager.h"
#include "../deb_installer/view/widgets/infocontrolbutton.h"

#include <stub.h>
#include <ut_Head.h>

#include <DTitlebar>

#include <QDebug>
#include <QAction>
#include <QFileInfo>
#include <QtConcurrent>
#include <QFuture>

#include <gtest/gtest.h>

using namespace QApt;

bool stud_isRunning()
{
    return true;
};

void stud_disableCloseAndExit()
{
}

void stud_enableCloseAndExit()
{
}

void stud_stopMonitor()
{
}

bool stud_checkSuffix(QString filePath)
{
    Q_UNUSED(filePath);
    return true;
}

void stud_appendPackage(QStringList package)
{
    Q_UNUSED(package);
}

void stud_refreshModel()
{
}

void stud_uninstallCurrentPackage()
{
}

QString stud_packageName()
{
    return QString("test.deb");
};

QString stud_version()
{
    return QString("1.0.0");
};

QString stud_longDescription()
{
    return QString("This is test");
};

const QList<QString> stud_preparedPackages()
{
    return QList<QString> {"test.deb"};
}

const QList<QString> stud1_preparedPackages()
{
    return QList<QString> {QStringList() << "test.deb"
                                         << "test1.deb"};
}

void stud_afterGetAutherFalse()
{
}

void stud_setEnableButton(bool bEnable)
{
    Q_UNUSED(bEnable);
}

void stud_DealDependResult(int iAuthRes, QString dependName)
{
    Q_UNUSED(iAuthRes);
    Q_UNUSED(dependName);
}

bool stud_isFile()
{
    return true;
}

bool stud_run()
{
    return true;
}

bool stud_reloadCache()
{
    return true;
}

void stud_removePackage(const int idx)
{
    Q_UNUSED(idx);
}

void stud_reset()
{
}

void stud_appendNoThread(QStringList packages, int allPackageSize)
{
    Q_UNUSED(packages);
    Q_UNUSED(allPackageSize);
}
class Debinstaller_UT : public UT_HEAD
{
public:
    //添加日志
    static void SetUpTestCase()
    {
        qDebug() << "SetUpTestCase" << endl;
    }
    static void TearDownTestCase()
    {
        qDebug() << "TearDownTestCase" << endl;
    }
    void SetUp() //TEST跑之前会执行SetUp
    {
        Stub stub;
        stub.set(ADDR(Backend, init), stud_run);
        stub.set(ADDR(PackagesManager, appendNoThread), stud_appendNoThread);
        deb = new DebInstaller();
        qDebug() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
    }
    DebInstaller *deb;
};

TEST_F(Debinstaller_UT, total_UT)
{
    Stub stub;
    stub.set(ADDR(DebInstaller, disableCloseAndExit), stud_disableCloseAndExit);
    stub.set(ADDR(DebInstaller, enableCloseAndExit), stud_enableCloseAndExit);
    stub.set(ADDR(DebInstaller, checkSuffix), stud_checkSuffix);
    stub.set(ADDR(TitleBarFocusMonitor, isRunning), stud_isRunning);
    stub.set(ADDR(TitleBarFocusMonitor, stopMonitor), stud_stopMonitor);
    stub.set(ADDR(DebListModel, appendPackage), stud_appendPackage);
    stub.set(ADDR(DebListModel, preparedPackages), stud_preparedPackages);
    stub.set(ADDR(DebListModel, removePackage), stud_removePackage);
    stub.set(ADDR(DebListModel, reset), stud_reset);
    stub.set(ADDR(MultipleInstallPage, refreshModel), stud_refreshModel);
    stub.set(ADDR(SingleInstallPage, uninstallCurrentPackage), stud_uninstallCurrentPackage);
    stub.set(ADDR(SingleInstallPage, afterGetAutherFalse), stud_afterGetAutherFalse);
    stub.set(ADDR(SingleInstallPage, setEnableButton), stud_setEnableButton);
    stub.set(ADDR(SingleInstallPage, DealDependResult), stud_DealDependResult);
    stub.set(ADDR(DebFile, packageName), stud_packageName);
    stub.set(ADDR(DebFile, version), stud_version);
    stub.set(ADDR(DebFile, longDescription), stud_longDescription);
    stub.set(ADDR(Backend, reloadCache), stud_reloadCache);

    deb->stopMonitorTitleBarFocus();
    deb->enableCloseButton(false);
    deb->enableCloseButton(true);
    deb->handleFocusPolicy();
    deb->onStartInstallRequested();

    deb->m_fileListModel->m_workerStatus_temp = DebListModel::WorkerProcessing;
    deb->onPackagesSelected(QStringList() << "test.deb"
                                          << "test1.deb");
    deb->onNewAppOpen(2222, QStringList() << "deb-installer"
                                          << "test.deb");
    deb->m_fileListModel->m_workerStatus_temp = DebListModel::WorkerFinished;
    deb->onPackagesSelected(QStringList() << "test.deb"
                                          << "test1.deb");

    deb->refreshMulti();
    deb->showInvalidePackageMessage();
    deb->showPkgExistMessage();
    deb->showUninstallConfirmPage();
    deb->onUninstallAccepted();
    deb->onUninstallCancel();
    deb->onAuthing(false);
    deb->reset();

    deb->removePackage(1);
    deb->appendPackageStart();
    deb->appendFinished();
    deb->single2Multi();
    deb->changeDragFlag();
    deb->m_dragflag = 2;
    deb->DealDependResult(DebListModel::AuthDependsSuccess, "test");
    deb->setEnableButton(true);
    deb->setEnableButton(false);
    deb->showHiddenButton();
    deb->m_dragflag = 1;
    deb->packageAppending = true;
    deb->DealDependResult(DebListModel::AuthBefore, "test");
    deb->setEnableButton(true);
    deb->setEnableButton(false);
    deb->showHiddenButton();
    EXPECT_EQ(deb->backToSinglePage(), nullptr);
}

TEST_F(Debinstaller_UT, onPackagesSelected_UT)
{
    Stub stub;
    stub.set(ADDR(DebListModel, preparedPackages), stud1_preparedPackages);
    stub.set(ADDR(DebFile, packageName), stud_packageName);
    stub.set(ADDR(DebFile, version), stud_version);
    stub.set(ADDR(DebFile, longDescription), stud_longDescription);
    stub.set(ADDR(MultipleInstallPage, refreshModel), stud_refreshModel);

    deb->removePackage(1);
}

TEST_F(Debinstaller_UT, checkSuffix_UT)
{
    Stub stub;

    stub.set(ADDR(QFileInfo, isFile), stud_isFile);
    EXPECT_EQ(deb->checkSuffix("test.deb"), true);
    EXPECT_EQ(deb->checkSuffix("test"), false);
}
