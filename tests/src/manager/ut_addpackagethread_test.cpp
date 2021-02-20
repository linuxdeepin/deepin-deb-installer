#include <gtest/gtest.h>

#include "../deb_installer/manager/AddPackageThread.h"


#include <stub.h>
#include <QApt/DebFile>
#include <QDir>
#include <QDebug>
#include <fstream>
#include <QFileInfo>
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

bool ut_mkTempDir()
{
    return false;
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
    stub.set(ADDR(AddPackageThread, mkTempDir), ut_mkTempDir);
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
    stub.set((bool(QDir::*)()const)ADDR(QDir, exists), apt_exits);

    ASSERT_STREQ(addPkgThread->link("test", "test1").toLocal8Bit(), (QString("test")).toLocal8Bit());
}

bool thread_stub_is_open()
{
    qDebug()<<"stb——is_open";
    return false;
}

void thread_stub_open(const std::string& __s,std::ios_base::openmode __mode)
{
    Q_UNUSED(__s);
    Q_UNUSED(__mode);
    qDebug()<<"stb——open";
}

void thread_stub_close()
{

}

QString thread_stub_SymbolicLink(QString previousName, QString packageName)
{
    Q_UNUSED(previousName);
    Q_UNUSED(packageName);

    return "";
}

QString thread_stub_absoluteFilePath()
{
    return "";
}

bool thread_stub_permission_true(QFile::Permissions permissions)
{
    Q_UNUSED(permissions);
    return true;
}

bool thread_stub_permission_false(QFile::Permissions permissions)
{
    Q_UNUSED(permissions);
    return false;
}

bool thread_stub_checkLocalFile_true(QString fileName)
{
    Q_UNUSED(fileName);
    return true;
}

bool thread_stub_checkLocalFile_false(QString fileName)
{
    Q_UNUSED(fileName);
    return false;
}

QString thread_stub_dealPackagePath(QString packagePath)
{
    Q_UNUSED(packagePath);
    return "";
}

TEST(AddPackageThread_Test, UT_AddPackageThread_checkLocalFile_true)
{
    Stub stub;
    QSet<QByteArray> md5;
    md5 << "24b0ce68d7af97ede709f3b723e686af";
    AddPackageThread *addPkgThread = new AddPackageThread(md5);

    QStringList dependsList;
    dependsList << "package1" << "package";
    addPkgThread->setPackages(dependsList);

    stub.set((void (std::fstream::*)(const std::string& __s,std::ios_base::openmode __mode))ADDR(std::fstream, open), thread_stub_open);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, is_open), thread_stub_is_open);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, close), thread_stub_close);

    stub.set(ADDR(QFileInfo,permission), thread_stub_permission_true);


    ASSERT_FALSE( addPkgThread->checkLocalFile("/1"));
}

TEST(AddPackageThread_Test, UT_AddPackageThread_checkLocalFile_false)
{
    Stub stub;
    QSet<QByteArray> md5;
    md5 << "24b0ce68d7af97ede709f3b723e686af";
    AddPackageThread *addPkgThread = new AddPackageThread(md5);

    QStringList dependsList;
    dependsList << "package1" << "package";
    addPkgThread->setPackages(dependsList);

    stub.set((void (std::fstream::*)(const std::string& __s,std::ios_base::openmode __mode))ADDR(std::fstream, open), thread_stub_open);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, is_open), thread_stub_is_open);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, close), thread_stub_close);

    stub.set(ADDR(QFileInfo,permission), thread_stub_permission_false);


    ASSERT_TRUE( addPkgThread->checkLocalFile("/1"));
}

TEST(AddPackageThread_Test, UT_AddPackageThread_dealInvalidPackage)
{
    Stub stub;
    QSet<QByteArray> md5;
    md5 << "24b0ce68d7af97ede709f3b723e686af";
    AddPackageThread *addPkgThread = new AddPackageThread(md5);

    QStringList dependsList;
    dependsList << "package1" << "package";
    addPkgThread->setPackages(dependsList);

    stub.set(ADDR(AddPackageThread,checkLocalFile), thread_stub_checkLocalFile_true);
    addPkgThread->dealInvalidPackage("");
}

TEST(AddPackageThread_Test, UT_AddPackageThread_dealInvalidPackage_false)
{
    Stub stub;
    QSet<QByteArray> md5;
    md5 << "24b0ce68d7af97ede709f3b723e686af";
    AddPackageThread *addPkgThread = new AddPackageThread(md5);

    QStringList dependsList;
    dependsList << "package1" << "package";
    addPkgThread->setPackages(dependsList);

    stub.set(ADDR(AddPackageThread,checkLocalFile), thread_stub_checkLocalFile_false);
    addPkgThread->dealInvalidPackage("");
}

TEST(AddPackageThread_Test, UT_AddPackageThread_dealPackagePath_absoluteFilePath)
{
    Stub stub;
    QSet<QByteArray> md5;
    md5 << "24b0ce68d7af97ede709f3b723e686af";
    AddPackageThread *addPkgThread = new AddPackageThread(md5);

    QStringList dependsList;
    dependsList << "package1" << "package";
    addPkgThread->setPackages(dependsList);

    stub.set(ADDR(QFileInfo, absoluteFilePath),thread_stub_absoluteFilePath);

    stub.set(ADDR(AddPackageThread, SymbolicLink),thread_stub_SymbolicLink);

    ASSERT_STREQ("", addPkgThread->dealPackagePath("test").toUtf8());
}

TEST(AddPackageThread_Test, UT_AddPackageThread_dealPackagePath_SymbolicLink)
{
    Stub stub;
    QSet<QByteArray> md5;
    md5 << "24b0ce68d7af97ede709f3b723e686af";
    AddPackageThread *addPkgThread = new AddPackageThread(md5);

    QStringList dependsList;
    dependsList << "package1" << "package";
    addPkgThread->setPackages(dependsList);

    stub.set(ADDR(QFileInfo, absoluteFilePath),thread_stub_absoluteFilePath);

    stub.set(ADDR(AddPackageThread, SymbolicLink),thread_stub_SymbolicLink);

    ASSERT_STREQ("", addPkgThread->dealPackagePath(" ").toUtf8());
}
