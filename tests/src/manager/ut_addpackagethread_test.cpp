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
    delete addPkgThread;
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
    delete addPkgThread;
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
    delete addPkgThread;
}
QByteArray md5sum()
{
    return "";
}

QString packagename()
{
    return "name";
}

bool ut_dealInvalidPackage()
{
    return true;
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
    stub.set(ADDR(AddPackageThread, dealInvalidPackage), ut_dealInvalidPackage);

    addPkgThread->run();

    ASSERT_EQ(addPkgThread->m_validPackageCount, 2);
    addPkgThread->terminate();
    delete addPkgThread;
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
    delete addPkgThread;
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
    delete addPkgThread;
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
    delete addPkgThread;
}

bool thread_stub_is_open()
{
    qDebug() << "stb——is_open";
    return false;
}

void thread_stub_open(const std::string &__s, std::ios_base::openmode __mode)
{
    Q_UNUSED(__s);
    Q_UNUSED(__mode);
    qDebug() << "stb——open";
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

bool add_stub_is_open_true()
{
    qDebug() << "stb——is_open";
    return true;
}

bool add_stub_is_open_false()
{
    qDebug() << "stb——is_open";
    return false;
}

void add_stub_open(const std::string &__s, std::ios_base::openmode __mode)
{
    Q_UNUSED(__s);
    Q_UNUSED(__mode);
    qDebug() << "stb——open";
}

void add_stub_close()
{

}
bool add_stub_permission_true(QFile::Permissions permissions)
{
    Q_UNUSED(permissions);
    return true;
}

bool add_stub_permission_false(QFile::Permissions permissions)
{
    Q_UNUSED(permissions);
    return false;
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
    stub.set((void (std::fstream::*)(const std::string & __s, std::ios_base::openmode __mode))ADDR(std::fstream, open), add_stub_open);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, is_open), add_stub_is_open_true);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, close), add_stub_close);

    addPkgThread->dealInvalidPackage("");
    delete addPkgThread;
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

    stub.set((void (std::fstream::*)(const std::string & __s, std::ios_base::openmode __mode))ADDR(std::fstream, open), add_stub_open);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, is_open), add_stub_is_open_false);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, close), add_stub_close);
    stub.set(ADDR(QFileInfo, permission), add_stub_permission_true);

    addPkgThread->dealInvalidPackage("");
    delete addPkgThread;
}

TEST(AddPackageThread_Test, UT_AddPackageThread_dealInvalidPackage_noPermission)
{
    Stub stub;
    QSet<QByteArray> md5;
    md5 << "24b0ce68d7af97ede709f3b723e686af";
    AddPackageThread *addPkgThread = new AddPackageThread(md5);

    QStringList dependsList;
    dependsList << "package1" << "package";
    addPkgThread->setPackages(dependsList);

    stub.set((void (std::fstream::*)(const std::string & __s, std::ios_base::openmode __mode))ADDR(std::fstream, open), add_stub_open);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, is_open), add_stub_is_open_false);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, close), add_stub_close);
    stub.set(ADDR(QFileInfo, permission), add_stub_permission_false);

    addPkgThread->dealInvalidPackage("");
    delete addPkgThread;
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

    stub.set(ADDR(QFileInfo, absoluteFilePath), thread_stub_absoluteFilePath);

    stub.set(ADDR(AddPackageThread, SymbolicLink), thread_stub_SymbolicLink);

    ASSERT_STREQ("", addPkgThread->dealPackagePath("test").toUtf8());
    delete addPkgThread;
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

    stub.set(ADDR(QFileInfo, absoluteFilePath), thread_stub_absoluteFilePath);

    stub.set(ADDR(AddPackageThread, SymbolicLink), thread_stub_SymbolicLink);

    ASSERT_STREQ("", addPkgThread->dealPackagePath(" ").toUtf8());
    delete addPkgThread;
}
