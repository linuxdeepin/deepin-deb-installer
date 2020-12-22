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

#include "../deb_installer/view/pages/multipleinstallpage.h"
#include "../deb_installer/model/deblistmodel.h"
#include "../deb_installer/manager/packagesmanager.h"

#include <stub.h>
#include <ut_Head.h>

#include <QDebug>
#include <QStandardItem>

#include <gtest/gtest.h>
using namespace QApt;

bool stud_init()
{
    return true;
}

int stud_getInstallFileSize()
{
    return 10;
}

class MultipleInstallPage_UT : public UT_HEAD
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
        stub.set(ADDR(Backend, init), stud_init);
        debListModel = new DebListModel();
        usleep(100 * 1000);
        multiplepage = new MultipleInstallPage(debListModel);
        usleep(100 * 1000);
        qDebug() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete multiplepage;
        delete debListModel;
    }
    MultipleInstallPage *multiplepage;
    DebListModel *debListModel;
};

TEST_F(MultipleInstallPage_UT, total_UT)
{
    Stub stub;
    stub.set(ADDR(DebListModel, getInstallFileSize), stud_getInstallFileSize);
    multiplepage->onWorkerFinshed();
    multiplepage->onOutputAvailable("test");
    multiplepage->onProgressChanged(100);
    multiplepage->onAutoScrollInstallList(-1);
    multiplepage->onAutoScrollInstallList(2);
    multiplepage->onRequestRemoveItemClicked(debListModel->index(0));
    EXPECT_EQ(debListModel->isWorkerPrepare(), true);
    multiplepage->showInfo();
    EXPECT_EQ(multiplepage->m_upDown, false);
    multiplepage->hiddenCancelButton();
    multiplepage->setEnableButton(true);
    multiplepage->refreshModel();
    multiplepage->hideInfo();
    multiplepage->afterGetAutherFalse();

    multiplepage->DealDependResult(2, "test");
    multiplepage->DealDependResult(3, "test");
    multiplepage->DealDependResult(4, "test");
    multiplepage->DealDependResult(5, "test");
    multiplepage->DealDependResult(6, "test");
    multiplepage->DealDependResult(1, "test");
}
