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
}

TEST(DealDependThread_Test, DealDependThread_UT_setBrokenDepend)
{
    DealDependThread *dThread = new DealDependThread();


    dThread->setBrokenDepend("package");

    ASSERT_STREQ(dThread->m_brokenDepend.toLocal8Bit(), "package");
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
}

TEST(DealDependThread_Test, DealDependThread_UT_onFinished)
{
    DealDependThread *dThread = new DealDependThread();

    dThread->bDependsStatusErr = false;
    dThread->onFinished(-1);

    ASSERT_FALSE(dThread->bDependsStatusErr);

}

TEST(DealDependThread_Test, DealDependThread_UT_finished)
{
    DealDependThread *dThread = new DealDependThread();

    emit dThread->proc->finished(0);

    ASSERT_FALSE(dThread->bDependsStatusErr);

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
    //    dThread->start();
    //    dThread->terminate();
}
