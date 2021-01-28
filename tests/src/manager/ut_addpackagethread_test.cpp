#include <gtest/gtest.h>

#include "../deb_installer/manager/AddPackageThread.h"


#include <stub.h>
#include <QApt/DebFile>
#include <QDir>
#include <QDebug>
using namespace QApt;

TEST(AddPackageThread_Test, UT_AddPackageThread_setPackage)
{
    QSet<QByteArray> md5;
    md5 << "24b0ce68d7af97ede709f3b723e686af";
    AddPackageThread *addPkgThread = new AddPackageThread(md5);

    QStringList dependsList;
    dependsList << "package1" << "package";
    addPkgThread->setPackages(dependsList);

    ASSERT_EQ(addPkgThread->m_packages.size(), 2);
}

TEST(AddPackageThread_Test, UT_AddPackageThread_setAppendPackagesMd5)
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
TEST(AddPackageThread_Test, UT_AddPackageThread_checkInvalid)
{
    Stub stub;

    QSet<QByteArray> md5;
    md5 << "24b0ce68d7af97ede709f3b723e686af";
    AddPackageThread *addPkgThread = new AddPackageThread(md5);

    QStringList dependsList;
    dependsList << "package1" << "package";
    addPkgThread->setPackages(dependsList);

    stub.set(ADDR(DebFile, isValid), isValid);

    addPkgThread->checkInvalid();

    ASSERT_EQ(addPkgThread->m_validPackageCount, 2);
}
QByteArray md5sum()
{
    return "";
}

QString packagename()
{
    return "name";
}

TEST(AddPackageThread_Test, UT_AddPackageThread_run)
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
TEST(AddPackageThread_Test, UT_AddPackageThread_SymbolicLink)
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


TEST(AddPackageThread_Test, UT_AddPackageThread_mkTempDir)
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

TEST(AddPackageThread_Test, UT_AddPackageThread_link)
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

