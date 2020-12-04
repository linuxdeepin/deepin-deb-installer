#include <gtest/gtest.h>

#define private public  // hack complier
#define protected public

#include "../deb_installer/manager/DealDependThread.h"

#undef private
#undef protected

#include <stub.h>
#include <QProcess>
#include <QIODevice>



TEST(DealDependThread_Test, DealDependThread_UT_001)
{
    DealDependThread *dThread = new DealDependThread();

    QStringList dependsList;
    dependsList <<"package1"<<"package";
    dThread->setDependsList(dependsList, 0);

    ASSERT_EQ(dThread->m_dependsList.size(),2);
}

TEST(DealDependThread_Test, DealDependThread_UT_002)
{
    DealDependThread *dThread = new DealDependThread();


    dThread->setBrokenDepend("package");

    ASSERT_STREQ(dThread->m_brokenDepend.toLocal8Bit(),"package");
}

void proc_start(const QString &program, const QStringList &arguments,QIODevice::OpenModeFlag mode)
{
    Q_UNUSED(program);
    Q_UNUSED(arguments);
    Q_UNUSED(mode);
    return;
}

TEST(DealDependThread_Test, DealDependThread_UT_003)
{
    DealDependThread *dThread = new DealDependThread();

    Stub stub;

    //(int(A::*)(int))ADDR(A,foo)
    //stub.set((int(A::*)(double))ADDR(A,foo), foo_stub_double);
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start), proc_start);
    dThread->terminate();
}

TEST(DealDependThread_Test, DealDependThread_UT_004)
{
    DealDependThread *dThread = new DealDependThread();

    dThread->bDependsStatusErr = true;
    dThread->onFinished(-1);

    ASSERT_FALSE(dThread->bDependsStatusErr);

}

TEST(DealDependThread_Test, DealDependThread_UT_005)
{
    DealDependThread *dThread = new DealDependThread();

    emit dThread->proc->finished(0);

    ASSERT_FALSE(dThread->bDependsStatusErr);

}
