#include <gtest/gtest.h>

#define private public  // hack complier
#define protected public

#include "../deb_installer/manager/AddPackageThread.h"

#undef private
#undef protected

#include <stub.h>
#include <QApt/DebFile>
#include <QDir>
#include <QDebug>
using namespace QApt;

TEST(AddPackageThread_Test, AddPackageThread_UT_001)
{
    QSet<QByteArray> md5;
    md5 << "24b0ce68d7af97ede709f3b723e686af";
    AddPackageThread *addPkgThread = new AddPackageThread(md5);

    QStringList dependsList;
    dependsList << "package1" << "package";
    addPkgThread->setPackages(dependsList);

    ASSERT_EQ(addPkgThread->m_packages.size(), 2);
}

TEST(AddPackageThread_Test, AddPackageThread_UT_002)
{
    QSet<QByteArray> md5;
    md5 << "24b0ce68d7af97ede709f3b723e686af";
    AddPackageThread *addPkgThread = new AddPackageThread(md5);

    md5.clear();
    md5 << "test";
    addPkgThread->setAppendPackagesMd5(md5);

    ASSERT_TRUE(addPkgThread->m_appendedPackagesMd5.contains("test"));
}

bool isValid()
{
    return true;
}
TEST(AddPackageThread_Test, AddPackageThread_UT_003)
{
    qDebug() << "ut0003";
    Stub stub;

    QSet<QByteArray> md5;
    md5 << "24b0ce68d7af97ede709f3b723e686af";
    AddPackageThread *addPkgThread = new AddPackageThread(md5);

    QStringList dependsList;
    dependsList << "package1" << "package";
    addPkgThread->setPackages(dependsList);

    stub.set(ADDR(DebFile, isValid), isValid);

    qDebug() << "ut0003" << "stub";
    addPkgThread->checkInvalid();

    qDebug() << "ut0003" << "stub" << "ASSERT";
    ASSERT_EQ(addPkgThread->m_validPackageCount, 2);
    qDebug() << "ut0003" << "stub" << "ASSERT END";
}
QByteArray md5sum()
{
    return "";
}

QString packagename()
{
    return "name";
}

TEST(AddPackageThread_Test, AddPackageThread_UT_004)
{
    Stub stub;
    QSet<QByteArray> md5;
    md5 << "24b0ce68d7af97ede709f3b723e686af";
    AddPackageThread *addPkgThread = new AddPackageThread(md5);

    QStringList dependsList;
    dependsList << "package1" << "package";
    addPkgThread->setPackages(dependsList);

    stub.set(ADDR(DebFile, isValid), isValid);
    stub.set(ADDR(DebFile, md5Sum), md5sum);
    stub.set(ADDR(DebFile, packageName), packagename);

    addPkgThread->run();

    ASSERT_EQ(addPkgThread->m_validPackageCount, 2);
}
bool apt_mkdir(const QString &dirName)
{
    Q_UNUSED(dirName);
    return true;
}

bool apt_exits()
{
    return true;
}
TEST(AddPackageThread_Test, AddPackageThread_UT_005)
{
    Stub stub;
    QSet<QByteArray> md5;
    md5 << "24b0ce68d7af97ede709f3b723e686af";
    AddPackageThread *addPkgThread = new AddPackageThread(md5);

    QStringList dependsList;
    dependsList << "package1" << "package";
    addPkgThread->setPackages(dependsList);

    stub.set(ADDR(DebFile, isValid), isValid);
    stub.set(ADDR(DebFile, md5Sum), md5sum);
    stub.set(ADDR(DebFile, packageName), packagename);

    stub.set((bool(QDir::*)(const QString &)const)ADDR(QDir, mkdir), apt_mkdir);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((bool(QDir::*)()const)ADDR(QDir, exists), apt_exits);
    ASSERT_STREQ(addPkgThread->SymbolicLink("test", "test1").toLocal8Bit(), (QString("test")).toLocal8Bit());
}


TEST(AddPackageThread_Test, AddPackageThread_UT_006)
{
    Stub stub;
    QSet<QByteArray> md5;
    md5 << "24b0ce68d7af97ede709f3b723e686af";
    AddPackageThread *addPkgThread = new AddPackageThread(md5);

    QStringList dependsList;
    dependsList << "package1" << "package";
    addPkgThread->setPackages(dependsList);

    stub.set(ADDR(DebFile, isValid), isValid);
    stub.set(ADDR(DebFile, md5Sum), md5sum);
    stub.set(ADDR(DebFile, packageName), packagename);

    stub.set((bool(QDir::*)(const QString &)const)ADDR(QDir, mkdir), apt_mkdir);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((bool(QDir::*)()const)ADDR(QDir, exists), apt_exits);
    ASSERT_TRUE(addPkgThread->mkTempDir());
}

TEST(AddPackageThread_Test, AddPackageThread_UT_007)
{
    Stub stub;
    QSet<QByteArray> md5;
    md5 << "24b0ce68d7af97ede709f3b723e686af";
    AddPackageThread *addPkgThread = new AddPackageThread(md5);

    QStringList dependsList;
    dependsList << "package1" << "package";
    addPkgThread->setPackages(dependsList);

    stub.set(ADDR(DebFile, isValid), isValid);
    stub.set(ADDR(DebFile, md5Sum), md5sum);
    stub.set(ADDR(DebFile, packageName), packagename);

    stub.set((bool(QDir::*)(const QString &)const)ADDR(QDir, mkdir), apt_mkdir);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((bool(QDir::*)()const)ADDR(QDir, exists), apt_exits);

    ASSERT_STREQ(addPkgThread->link("test", "test1").toLocal8Bit(), (QString("test")).toLocal8Bit());
}

