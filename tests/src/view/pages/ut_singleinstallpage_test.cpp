/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     linxun <linxun@uniontech.com>
*
* Maintainer: linxun <linxun@uniontech.com>
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

#include "../deb-installer/view/pages/singleinstallpage.h"
#include "../deb-installer/model/deblistmodel.h"
#include "../deb-installer/manager/packagesmanager.h"
#include "../deb-installer/manager/PackageDependsStatus.h"
#include "../deb-installer/view/widgets/workerprogress.h"

#include <stub.h>
#include <ut_Head.h>

#include <QDebug>

#include <gtest/gtest.h>
using namespace QApt;

void stud_installPackages()
{
    return;
}

bool stud_singleinit()
{
    return true;
}

QString stud_singlepackageName()
{
    return "test";
}

QString stud_singleversion()
{
    return "1.0.0";
}

QString stud_singlelongDescription()
{
    return "this is test";
}

QString stud_singlearchitecture()
{
    return "x86";
}

QString stud_singlefilePath()
{
    return "/home";
}

QString stud_singleshortDescription()
{
    return "this is a short test";
}

PackageDependsStatus stud_singlegetPackageDependsStatus(const int index)
{
    Q_UNUSED(index);
    PackageDependsStatus m_status;
    m_status.status = 1;
    return m_status;
}

int stud_singlepackageInstallStatus(const int index)
{
    Q_UNUSED(index);
    return 1;
}

void stud_singleuninstallPackage(const int idx)
{
    Q_UNUSED(idx);
}

int stud_successtoInt(bool *ok = nullptr)
{
    Q_UNUSED(ok);
    return 2;
}
int stud_failedtoInt(bool *ok = nullptr)
{
    Q_UNUSED(ok);
    return 3;
}
bool stud_recheckPackagePath(QString )
{
    return true;
}

PackageDependsStatus stud_getPackageDependsStatus(const int )
{
    return PackageDependsStatus::_break("package");
}


class SingleInstallpage_UT : public UT_HEAD
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
        qDebug() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete page;
        delete model;
    }
    SingleInstallPage *page;
    DebListModel *model;
};

QVariant stu_data()
{
    return DebListModel::DependsOk;
}

TEST_F(SingleInstallpage_UT, total_UT)
{
    Stub stub;
    stub.set(ADDR(Backend, init), stud_singleinit);
    stub.set(ADDR(DebFile, packageName), stud_singlepackageName);
    stub.set(ADDR(DebFile, version), stud_singleversion);
    stub.set(ADDR(DebFile, longDescription), stud_singlelongDescription);
    stub.set(ADDR(DebFile, filePath), stud_singlefilePath);
    stub.set(ADDR(DebFile, architecture), stud_singlearchitecture);
    stub.set(ADDR(DebFile, shortDescription), stud_singleshortDescription);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stud_singlegetPackageDependsStatus);
    stub.set(ADDR(PackagesManager, packageInstallStatus), stud_singlepackageInstallStatus);
    stub.set(ADDR(DebListModel, slotInstallPackages), stud_installPackages);
    stub.set(ADDR(DebListModel, slotUninstallPackage), stud_singleuninstallPackage);
    stub.set(ADDR(DebListModel, recheckPackagePath), stud_recheckPackagePath);

    model = new DebListModel();
    model->m_packagesManager->m_preparedPackages.append("test");
    model->m_packagesManager->m_preparedPackages.append("test1");
    page = new SingleInstallPage(model);
    page->slotReinstall();

    EXPECT_EQ(page->initLabelWidth(11), 260);
    EXPECT_EQ(page->initLabelWidth(12), 255);
    EXPECT_EQ(page->initLabelWidth(13), 250);
    EXPECT_EQ(page->initLabelWidth(14), 250);
    EXPECT_EQ(page->initLabelWidth(15), 240);
    EXPECT_EQ(page->initLabelWidth(16), 240);
    EXPECT_EQ(page->initLabelWidth(18), 230);
    EXPECT_EQ(page->initLabelWidth(20), 220);
    EXPECT_EQ(page->initLabelWidth(22), 220);

    page->slotInstall();
    page->slotUninstallCurrentPackage();
    page->slotShowInfomation();
    page->slotHideInfomation();
    page->slotShowInfo();
    page->slotOutputAvailable("test");
    page->m_upDown = false;
    page->slotWorkerFinished();

    page->slotWorkerProgressChanged(100);
    page->m_progress->setValue(60);
    page->slotWorkerProgressChanged(50);

    page->setEnableButton(false);
    page->setEnableButton(true);

    page->afterGetAutherFalse();
    page->setAuthConfirm("test");
    page->setAuthBefore();
    page->setCancelAuthOrAuthDependsErr();
    stub.set(ADDR(QModelIndex, data), stu_data);
    page->setCancelAuthOrAuthDependsErr();
    page->DealDependResult(1, "test");
    page->m_operate = SingleInstallPage::Install;
    page->setEnableButton(true);
    page->setAuthBefore();
    page->afterGetAutherFalse();
    page->m_operate = SingleInstallPage::Reinstall;
    page->setAuthBefore();
    page->afterGetAutherFalse();
}

TEST_F(SingleInstallpage_UT, onWorkFinishedFailed_UT)
{
    Stub stub;
    stub.set(ADDR(Backend, init), stud_singleinit);
    stub.set(ADDR(DebFile, packageName), stud_singlepackageName);
    stub.set(ADDR(DebFile, version), stud_singleversion);
    stub.set(ADDR(DebFile, longDescription), stud_singlelongDescription);
    stub.set(ADDR(DebFile, filePath), stud_singlefilePath);
    stub.set(ADDR(DebFile, architecture), stud_singlearchitecture);
    stub.set(ADDR(DebFile, shortDescription), stud_singleshortDescription);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stud_singlegetPackageDependsStatus);
    stub.set(ADDR(PackagesManager, packageInstallStatus), stud_singlepackageInstallStatus);
    stub.set(ADDR(DebListModel, slotInstallPackages), stud_installPackages);
    stub.set(ADDR(DebListModel, slotUninstallPackage), stud_singleuninstallPackage);
    stub.set(ADDR(DebListModel, recheckPackagePath), stud_recheckPackagePath);

    model = new DebListModel();
    usleep(100 * 1000);
    model->m_packagesManager->m_preparedPackages.append("test");
    model->m_packagesManager->m_preparedPackages.append("test1");
    page = new SingleInstallPage(model);
    usleep(100 * 1000);
    stub.set(ADDR(QVariant, toInt), stud_failedtoInt);
    page->m_operate = SingleInstallPage::Uninstall;
    page->slotWorkerFinished();
    page->m_operate = SingleInstallPage::Install;
    page->slotWorkerFinished();
}

TEST_F(SingleInstallpage_UT, onWorkFinishedSuccees_UT)
{
    Stub stub;
    stub.set(ADDR(Backend, init), stud_singleinit);
    stub.set(ADDR(DebFile, packageName), stud_singlepackageName);
    stub.set(ADDR(DebFile, version), stud_singleversion);
    stub.set(ADDR(DebFile, longDescription), stud_singlelongDescription);
    stub.set(ADDR(DebFile, filePath), stud_singlefilePath);
    stub.set(ADDR(DebFile, architecture), stud_singlearchitecture);
    stub.set(ADDR(DebFile, shortDescription), stud_singleshortDescription);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stud_singlegetPackageDependsStatus);
    stub.set(ADDR(PackagesManager, packageInstallStatus), stud_singlepackageInstallStatus);
    stub.set(ADDR(DebListModel, slotInstallPackages), stud_installPackages);
    stub.set(ADDR(DebListModel, slotUninstallPackage), stud_singleuninstallPackage);
    stub.set(ADDR(QModelIndex, data), stu_data);
    stub.set(ADDR(DebListModel, recheckPackagePath), stud_recheckPackagePath);
    model = new DebListModel();
    usleep(100 * 1000);
    model->m_packagesManager->m_preparedPackages.append("test");
    model->m_packagesManager->m_preparedPackages.append("test1");
    page = new SingleInstallPage(model);
    usleep(100 * 1000);
    page->showPackageInfo();

    stub.set(ADDR(QVariant, toInt), stud_successtoInt);
    page->slotWorkerFinished();
    page->m_operate = SingleInstallPage::Install;
    page->slotWorkerFinished();
    page->m_operate = SingleInstallPage::Uninstall;
    page->setAuthBefore();
    page->DealDependResult(3, "test");
    page->DealDependResult(2, "test");
    page->DealDependResult(5, "test");
    page->DealDependResult(4, "test");
    page->DealDependResult(6, "test");
    page->DealDependResult(0, "test");
}

TEST_F(SingleInstallpage_UT, initTabOrder_UT)
{
    Stub stub;
    stub.set(ADDR(Backend, init), stud_singleinit);
    stub.set(ADDR(DebFile, packageName), stud_singlepackageName);
    stub.set(ADDR(DebFile, version), stud_singleversion);
    stub.set(ADDR(DebFile, longDescription), stud_singlelongDescription);
    stub.set(ADDR(DebFile, filePath), stud_singlefilePath);
    stub.set(ADDR(DebFile, architecture), stud_singlearchitecture);
    stub.set(ADDR(DebFile, shortDescription), stud_singleshortDescription);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stud_singlegetPackageDependsStatus);
    stub.set(ADDR(PackagesManager, packageInstallStatus), stud_singlepackageInstallStatus);
    stub.set(ADDR(DebListModel, slotInstallPackages), stud_installPackages);
    stub.set(ADDR(DebListModel, slotUninstallPackage), stud_singleuninstallPackage);
    stub.set(ADDR(QModelIndex, data), stu_data);
    stub.set(ADDR(DebListModel, recheckPackagePath), stud_recheckPackagePath);
    model = new DebListModel();
    usleep(100 * 1000);
    model->m_packagesManager->m_preparedPackages.append("test");
    model->m_packagesManager->m_preparedPackages.append("test1");
    page = new SingleInstallPage(model);
    usleep(100 * 1000);

    page->m_installButton->setVisible(true);
    page->m_backButton->setVisible(true);
    page->m_uninstallButton->setVisible(true);
    page->initTabOrder();
    page->slotHideDependsInfo();
    page->slotShowDependsInfo();
}

TEST_F(SingleInstallpage_UT, paintEvent_UT)
{
    Stub stub;
    stub.set(ADDR(Backend, init), stud_singleinit);
    stub.set(ADDR(DebFile, packageName), stud_singlepackageName);
    stub.set(ADDR(DebFile, version), stud_singleversion);
    stub.set(ADDR(DebFile, longDescription), stud_singlelongDescription);
    stub.set(ADDR(DebFile, filePath), stud_singlefilePath);
    stub.set(ADDR(DebFile, architecture), stud_singlearchitecture);
    stub.set(ADDR(DebFile, shortDescription), stud_singleshortDescription);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stud_singlegetPackageDependsStatus);
    stub.set(ADDR(PackagesManager, packageInstallStatus), stud_singlepackageInstallStatus);
    stub.set(ADDR(DebListModel, slotInstallPackages), stud_installPackages);
    stub.set(ADDR(DebListModel, slotUninstallPackage), stud_singleuninstallPackage);
    stub.set(ADDR(QModelIndex, data), stu_data);
    stub.set(ADDR(DebListModel, recheckPackagePath), stud_recheckPackagePath);
    model = new DebListModel();
    usleep(100 * 1000);
    model->m_packagesManager->m_preparedPackages.append("test");
    model->m_packagesManager->m_preparedPackages.append("test1");
    page = new SingleInstallPage(model);
    usleep(100 * 1000);
    QPaintEvent paint(QRect(page->rect()));
    page->paintEvent(&paint);
}

TEST_F(SingleInstallpage_UT, slotDependPackages_UT)
{
    Stub stub;
    stub.set(ADDR(Backend, init), stud_singleinit);
    stub.set(ADDR(DebFile, packageName), stud_singlepackageName);
    stub.set(ADDR(DebFile, version), stud_singleversion);
    stub.set(ADDR(DebFile, longDescription), stud_singlelongDescription);
    stub.set(ADDR(DebFile, filePath), stud_singlefilePath);
    stub.set(ADDR(DebFile, architecture), stud_singlearchitecture);
    stub.set(ADDR(DebFile, shortDescription), stud_singleshortDescription);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stud_singlegetPackageDependsStatus);
    stub.set(ADDR(PackagesManager, packageInstallStatus), stud_singlepackageInstallStatus);
    stub.set(ADDR(DebListModel, slotInstallPackages), stud_installPackages);
    stub.set(ADDR(DebListModel, slotUninstallPackage), stud_singleuninstallPackage);
    stub.set(ADDR(QModelIndex, data), stu_data);
    stub.set(ADDR(DebListModel, recheckPackagePath), stud_recheckPackagePath);
    model = new DebListModel();
    usleep(100 * 1000);
    model->m_packagesManager->m_preparedPackages.append("test");
    model->m_packagesManager->m_preparedPackages.append("test1");
    page = new SingleInstallPage(model);
    usleep(100 * 1000);
    //    QMap<QByteArray, QPair<QList<DependInfo>, QList<DependInfo>>> dependPackages;
    DependInfo info;
    info.packageName = "libopencl1";
    info.version = "1.0";
    QList<DependInfo> list;
    list.append(info);
    QPair<QList<DependInfo>, QList<DependInfo>> pair;
    pair.first.append(list);
    pair.second.append(list);
    //    dependPackages.insert("deb", pair);
    page->slotDependPackages(pair, false);
}
