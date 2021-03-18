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



TEST(DealDependThread_Test, DealDependThread_UT_setDependsList)
{
    DealDependThread *dThread = new DealDependThread();

    QStringList dependsList;
    dependsList << "package1" << "package";
    dThread->setDependsList(dependsList, 0);

    ASSERT_EQ(dThread->m_dependsList.size(), 2);
    delete dThread;
}

TEST(DealDependThread_Test, DealDependThread_UT_setBrokenDepend)
{
    DealDependThread *dThread = new DealDependThread();


    dThread->setBrokenDepend("package");

    ASSERT_STREQ(dThread->m_brokenDepend.toLocal8Bit(), "package");

    delete dThread;
}

void proc_start(const QString &program, const QStringList &arguments, QIODevice::OpenModeFlag mode)
{
    Q_UNUSED(program);
    Q_UNUSED(arguments);
    Q_UNUSED(mode);
    return;
}

TEST(DealDependThread_Test, DealDependThread_UT_start)
{
    DealDependThread *dThread = new DealDependThread();

    Stub stub;

    //(int(A::*)(int))ADDR(A,foo)
    //stub.set((int(A::*)(double))ADDR(A,foo), foo_stub_double);
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), proc_start);

    dThread->start();
    dThread->terminate();
    dThread->wait();

    delete dThread;
}

TEST(DealDependThread_Test, DealDependThread_UT_onFinished)
{
    DealDependThread *dThread = new DealDependThread();

    dThread->bDependsStatusErr = false;
    dThread->onFinished(-1);

    ASSERT_FALSE(dThread->bDependsStatusErr);
    delete dThread;

}

TEST(DealDependThread_Test, DealDependThread_UT_finished)
{
    DealDependThread *dThread = new DealDependThread();

    emit dThread->proc->finished(0);

    ASSERT_FALSE(dThread->bDependsStatusErr);
    delete dThread;

}

QByteArray readAllStandardOutput_success()
{
    return "Not authorized";
}

TEST(DealDependThread_Test, DealDependThread_UT_on_readoutput)
{
    DealDependThread *dThread = new DealDependThread();
    Stub stub;
    stub.set(ADDR(QProcess, readAllStandardOutput), readAllStandardOutput_success);
    dThread->on_readoutput();

    delete dThread;

}
