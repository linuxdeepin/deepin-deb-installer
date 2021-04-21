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

#include "../deb-installer/view/pages/debinstaller.h"
#include "../deb-installer/model/deblistmodel.h"
#include "../deb-installer/view/pages/multipleinstallpage.h"
#include "../deb-installer/view/pages/singleinstallpage.h"
#include "../deb-installer/manager/packagesmanager.h"
#include "../deb-installer/view/widgets/infocontrolbutton.h"

#include <stub.h>
#include <ut_Head.h>

#include <DTitlebar>

#include <QDebug>
#include <QAction>
#include <QFileInfo>
#include <QtConcurrent>
#include <QFuture>
#include <QUrl>

#include <gtest/gtest.h>

using namespace QApt;

void stud_removePackage(const int idx)
{
    Q_UNUSED(idx);
}

void stud_installDebs()
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
        usleep(1000 * 1000);
        qDebug() << "SetUp" << endl;

    }
    void TearDown() //TEST跑完之后会执行TearDown
    {

        delete deb;
    }
    DebInstaller *deb;

};
void stub_enableCloseAndExit()
{
    return;
}

TEST_F(Debinstaller_UT, total_UT)
{
    Stub stub;
    stub.set(ADDR(DebInstaller, checkSuffix), stud_checkSuffix);
    stub.set(ADDR(DebListModel, slotAppendPackage), stud_appendPackage);
    stub.set(ADDR(DebListModel, preparedPackages), stud_preparedPackages);
    stub.set(ADDR(DebListModel, slotRemovePackage), stud_removePackage);
    stub.set(ADDR(DebListModel, reset), stud_reset);
    stub.set(ADDR(DebListModel, initPrepareStatus), stud_reset);
    stub.set(ADDR(DebListModel, installDebs), stud_installDebs);
    stub.set(ADDR(MultipleInstallPage, refreshModel), stud_refreshModel);
    stub.set(ADDR(MultipleInstallPage, setEnableButton), stud_setEnableButton);
    stub.set(ADDR(MultipleInstallPage, DealDependResult), stud_DealDependResult);
    stub.set(ADDR(MultipleInstallPage, afterGetAutherFalse), stud_afterGetAutherFalse);
    stub.set(ADDR(SingleInstallPage, slotUninstallCurrentPackage), stud_uninstallCurrentPackage);
    stub.set(ADDR(SingleInstallPage, afterGetAutherFalse), stud_afterGetAutherFalse);
    stub.set(ADDR(SingleInstallPage, setEnableButton), stud_setEnableButton);
    stub.set(ADDR(SingleInstallPage, DealDependResult), stud_DealDependResult);
    stub.set(ADDR(DebFile, packageName), stud_packageName);
    stub.set(ADDR(DebFile, version), stud_version);
    stub.set(ADDR(DebFile, longDescription), stud_longDescription);
    stub.set(ADDR(Backend, reloadCache), stud_reloadCache);
    stub.set(ADDR(DebInstaller, enableCloseAndExit), stub_enableCloseAndExit);
    stub.set(ADDR(DebInstaller, disableCloseAndExit), stub_enableCloseAndExit);


    deb->slotEnableCloseButton(false);
    deb->slotEnableCloseButton(true);

    deb->m_fileListModel->m_workerStatus_temp = DebListModel::WorkerProcessing;
    deb->slotPackagesSelected(QStringList() << "test.deb"
                            << "test1.deb");
    deb->m_fileListModel->m_workerStatus_temp = DebListModel::WorkerFinished;
    deb->slotPackagesSelected(QStringList() << "test.deb"
                            << "test1.deb");

    deb->refreshMulti();
    deb->slotShowInvalidePackageMessage();
    deb->slotShowPkgExistMessage();
    deb->slotShowUninstallConfirmPage();
    deb->slotUninstallAccepted();
    deb->slotUninstallCancel();
    deb->slotSetAuthingStatus(false);
    deb->slotReset();

    deb->slotRemovePackage(1);
    deb->appendPackageStart();
    deb->appendFinished();
    deb->single2Multi();
    deb->slotChangeDragFlag();
    deb->m_dragflag = 2;
    deb->slotDealDependResult(DebListModel::AuthDependsSuccess, "test");
    deb->slotSetEnableButton(true);
    deb->slotSetEnableButton(false);
    deb->slotShowHiddenButton();
    deb->m_dragflag = 1;
    deb->slotDealDependResult(DebListModel::AuthBefore, "test");
    deb->slotSetEnableButton(true);
    deb->slotSetEnableButton(false);
    deb->slotShowHiddenButton();
    deb->m_packageAppending = true;
    deb->slotSetEnableButton(true);
    deb->slotShowPkgRemovedMessage("00");
    EXPECT_EQ(deb->backToSinglePage(), nullptr);

}

TEST_F(Debinstaller_UT, checkSuffix_UT)
{
    Stub stub;
    stub.set(ADDR(QFileInfo, isFile), stud_isFile);
    EXPECT_EQ(deb->checkSuffix("test.deb"), true);
    EXPECT_EQ(deb->checkSuffix("test"), false);
}

TEST_F(Debinstaller_UT, keyPressEvent_UT)
{
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QCoreApplication::sendEvent(deb, &keyPressEvent);
}

TEST_F(Debinstaller_UT, dragEnterEvent_UT)
{
    QMimeData *mimeData = new QMimeData;
    QDragEnterEvent enterEvent(QPoint(0, 0), Qt::MoveAction, mimeData, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(deb, &enterEvent);
    delete  mimeData;
}

TEST_F(Debinstaller_UT, dropEvent_UT)
{
    QByteArray csvData = "test";
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("text/csv", csvData);
    QDropEvent dropEvent(QPoint(0, 0), Qt::MoveAction, mimeData, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(deb, &dropEvent);
    delete mimeData;
}
