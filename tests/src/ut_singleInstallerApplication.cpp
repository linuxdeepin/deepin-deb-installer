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

#include "../deb-installer/singleInstallerApplication.h"
#include "../deb-installer/utils/utils.h"
#include "../deb-installer/view/pages/debinstaller.h"

#include <stub.h>
#include <ut_Head.h>

#include <QDebug>
#include <QCommandLineParser>
#include <QScopedPointer>

#include <gtest/gtest.h>

void stud_singleaAppProcess(const QCoreApplication &app)
{
    Q_UNUSED(app);
}

class SingleInstallerApplication_UT : public UT_HEAD
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
        int a = 1;
        int &argc = a;
        char argv[] = "abc";
        char argv1[] = "123";
        char *pArgv[] = {argv, argv1};
        singleInstaller = new SingleInstallerApplication(argc, pArgv);
        usleep(100 * 1000);
        qDebug() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
    }
    SingleInstallerApplication *singleInstaller;
};

TEST_F(SingleInstallerApplication_UT, UT_SingleInstallerApplication_parseCmdLine)
{
    Stub stub;
    stub.set((void (QCommandLineParser::*)(const QCoreApplication &))ADDR(QCommandLineParser, process), stud_singleaAppProcess);
    singleInstaller->m_selectedFiles.append(" ");
    singleInstaller->parseCmdLine();
    EXPECT_TRUE(singleInstaller->m_selectedFiles.isEmpty());
}

TEST_F(SingleInstallerApplication_UT, UT_SingleInstallerApplication_activateWindow)
{
    singleInstaller->m_selectedFiles.append(" ");
    singleInstaller->activateWindow();

    EXPECT_TRUE(singleInstaller->m_qspMainWnd.get()->isVisible());
}

TEST_F(SingleInstallerApplication_UT, UT_SingleInstallerApplication_InstallerDeb)
{
    QStringList debPathList;
    singleInstaller->InstallerDeb(debPathList);
    EXPECT_EQ(0, debPathList.size());
}

TEST_F(SingleInstallerApplication_UT, UT_SingleInstallerApplication_InstallerDeb_01)
{
    QStringList debPathList;
    debPathList.append("test.deb");
    singleInstaller->InstallerDeb(debPathList);
    EXPECT_EQ(1, debPathList.size());
}
