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
#include <gtest/gtest.h>

#include "../deb_installer/manager/DealDependThread.h"

#include <stub.h>
#include <QProcess>
#include <QIODevice>

class ut_dealDependThread_Test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp()
    {
        m_dThread = new DealDependThread();
    }
    void TearDown()
    {
        delete m_dThread;
    }

    DealDependThread *m_dThread = nullptr;
    Stub stub;
};

TEST_F(ut_dealDependThread_Test, DealDependThread_UT_setDependsList)
{
    QStringList dependsList;
    dependsList << "package1" << "package";
    m_dThread->setDependsList(dependsList, 0);

    ASSERT_EQ(m_dThread->m_dependsList.size(), 2);
}

TEST_F(ut_dealDependThread_Test, DealDependThread_UT_setBrokenDepend)
{
    m_dThread->setBrokenDepend("package");

    ASSERT_STREQ(m_dThread->m_brokenDepend.toLocal8Bit(), "package");
}

void proc_start(const QString &program, const QStringList &arguments, QIODevice::OpenModeFlag mode)
{
    Q_UNUSED(program);
    Q_UNUSED(arguments);
    Q_UNUSED(mode);
    return;
}

TEST_F(ut_dealDependThread_Test, DealDependThread_UT_start)
{
    //(int(A::*)(int))ADDR(A,foo)
    //stub.set((int(A::*)(double))ADDR(A,foo), foo_stub_double);
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), proc_start);
    m_dThread->start();
    m_dThread->terminate();
    m_dThread->wait();
}

TEST_F(ut_dealDependThread_Test, DealDependThread_UT_onFinished)
{
    m_dThread->bDependsStatusErr = false;
    m_dThread->onFinished(-1);

    ASSERT_FALSE(m_dThread->bDependsStatusErr);
}

TEST_F(ut_dealDependThread_Test, DealDependThread_UT_finished)
{
    emit m_dThread->proc->finished(0);

    ASSERT_FALSE(m_dThread->bDependsStatusErr);
}

QByteArray readAllStandardOutput_success()
{
    return "Not authorized";
}

TEST_F(ut_dealDependThread_Test, DealDependThread_UT_on_readoutput)
{
    stub.set(ADDR(QProcess, readAllStandardOutput), readAllStandardOutput_success);
    m_dThread->on_readoutput();
}
