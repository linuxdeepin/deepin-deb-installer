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

#include "../deb-installer/manager/AddPackageThread.h"

#include <stub.h>
#include <QApt/DebFile>
#include <QDir>
#include <QDebug>
#include <fstream>
#include <QFileInfo>
using namespace QApt;

class ut_addPackageThread_Test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp()
    {
        md5 << "24b0ce68d7af97ede709f3b723e686af";
        m_addPkgThread = new AddPackageThread(md5);
    }
    void TearDown()
    {
        delete m_addPkgThread;
    }

    AddPackageThread *m_addPkgThread = nullptr;
    Stub stub;
    QSet<QByteArray> md5;
};

TEST_F(ut_addPackageThread_Test, UT_AddPackageThread_setPackage)
{
    QStringList dependsList;
    dependsList << "package1" << "package";
    m_addPkgThread->setPackages(dependsList);

    ASSERT_EQ(m_addPkgThread->m_packages.size(), 2);
}

TEST_F(ut_addPackageThread_Test, UT_AddPackageThread_setAppendPackagesMd5)
{
    md5.clear();
    md5 << "test";
    m_addPkgThread->setAppendPackagesMd5(md5);

    ASSERT_TRUE(m_addPkgThread->m_appendedPackagesMd5.contains("test"));
}

bool isValid()
{
    return true;
}
TEST_F(ut_addPackageThread_Test, UT_AddPackageThread_checkInvalid)
{
    QStringList dependsList;
    dependsList << "package1" << "package";
    m_addPkgThread->setPackages(dependsList);

    stub.set(ADDR(DebFile, isValid), isValid);

    m_addPkgThread->checkInvalid();

    ASSERT_EQ(m_addPkgThread->m_validPackageCount, 2);
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

TEST_F(ut_addPackageThread_Test, UT_AddPackageThread_run)
{
    QStringList dependsList;
    dependsList << "package1" << "package";
    m_addPkgThread->setPackages(dependsList);

    stub.set(ADDR(DebFile, isValid), isValid);
    stub.set(ADDR(DebFile, md5Sum), md5sum);
    stub.set(ADDR(DebFile, packageName), packagename);
    stub.set(ADDR(AddPackageThread, dealInvalidPackage), ut_dealInvalidPackage);

    m_addPkgThread->run();

    ASSERT_EQ(m_addPkgThread->m_validPackageCount, 2);
    m_addPkgThread->terminate();
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

TEST_F(ut_addPackageThread_Test, UT_AddPackageThread_SymbolicLink)
{
    QStringList dependsList;
    dependsList << "package1" << "package";
    m_addPkgThread->setPackages(dependsList);

    stub.set(ADDR(DebFile, isValid), isValid);
    stub.set(ADDR(DebFile, md5Sum), md5sum);
    stub.set(ADDR(DebFile, packageName), packagename);

    stub.set((bool(QDir::*)(const QString &)const)ADDR(QDir, mkdir), apt_mkdir);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((bool(QDir::*)()const)ADDR(QDir, exists), apt_exits);
    stub.set(ADDR(AddPackageThread, mkTempDir), ut_mkTempDir);
    ASSERT_STREQ(m_addPkgThread->SymbolicLink("test", "test1").toLocal8Bit(), (QString("test")).toLocal8Bit());
}

TEST_F(ut_addPackageThread_Test, UT_AddPackageThread_mkTempDir)
{
    QStringList dependsList;
    dependsList << "package1" << "package";
    m_addPkgThread->setPackages(dependsList);

    stub.set(ADDR(DebFile, isValid), isValid);
    stub.set(ADDR(DebFile, md5Sum), md5sum);
    stub.set(ADDR(DebFile, packageName), packagename);

    stub.set((bool(QDir::*)(const QString &)const)ADDR(QDir, mkdir), apt_mkdir);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((bool(QDir::*)()const)ADDR(QDir, exists), apt_exits);
    ASSERT_TRUE(m_addPkgThread->mkTempDir());
}

TEST_F(ut_addPackageThread_Test, UT_AddPackageThread_link)
{
    QStringList dependsList;
    dependsList << "package1" << "package";
    m_addPkgThread->setPackages(dependsList);

    stub.set(ADDR(DebFile, isValid), isValid);
    stub.set(ADDR(DebFile, md5Sum), md5sum);
    stub.set(ADDR(DebFile, packageName), packagename);

    stub.set((bool(QDir::*)(const QString &)const)ADDR(QDir, mkdir), apt_mkdir);
    stub.set((bool(QDir::*)()const)ADDR(QDir, exists), apt_exits);
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

TEST_F(ut_addPackageThread_Test, UT_AddPackageThread_dealInvalidPackage)
{
    QStringList dependsList;
    dependsList << "package1" << "package";
    m_addPkgThread->setPackages(dependsList);
    stub.set((void (std::fstream::*)(const std::string & __s, std::ios_base::openmode __mode))ADDR(std::fstream, open), add_stub_open);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, is_open), add_stub_is_open_true);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, close), add_stub_close);

    m_addPkgThread->dealInvalidPackage("");
}

TEST_F(ut_addPackageThread_Test, UT_AddPackageThread_dealInvalidPackage_false)
{
    QStringList dependsList;
    dependsList << "package1" << "package";
    m_addPkgThread->setPackages(dependsList);

    stub.set((void (std::fstream::*)(const std::string & __s, std::ios_base::openmode __mode))ADDR(std::fstream, open), add_stub_open);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, is_open), add_stub_is_open_false);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, close), add_stub_close);
    stub.set(ADDR(QFileInfo, permission), add_stub_permission_true);

    m_addPkgThread->dealInvalidPackage("");
}

TEST_F(ut_addPackageThread_Test, UT_AddPackageThread_dealInvalidPackage_noPermission)
{
    QStringList dependsList;
    dependsList << "package1" << "package";
    m_addPkgThread->setPackages(dependsList);

    stub.set((void (std::fstream::*)(const std::string & __s, std::ios_base::openmode __mode))ADDR(std::fstream, open), add_stub_open);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, is_open), add_stub_is_open_false);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, close), add_stub_close);
    stub.set(ADDR(QFileInfo, permission), add_stub_permission_false);

    m_addPkgThread->dealInvalidPackage("");
}

TEST_F(ut_addPackageThread_Test, UT_AddPackageThread_dealPackagePath_absoluteFilePath)
{
    QStringList dependsList;
    dependsList << "package1" << "package";
    m_addPkgThread->setPackages(dependsList);

    stub.set(ADDR(QFileInfo, absoluteFilePath), thread_stub_absoluteFilePath);

    stub.set(ADDR(AddPackageThread, SymbolicLink), thread_stub_SymbolicLink);

    ASSERT_STREQ("", m_addPkgThread->dealPackagePath("test").toUtf8());
}

TEST_F(ut_addPackageThread_Test, UT_AddPackageThread_dealPackagePath_SymbolicLink)
{
    QStringList dependsList;
    dependsList << "package1" << "package";
    m_addPkgThread->setPackages(dependsList);

    stub.set(ADDR(QFileInfo, absoluteFilePath), thread_stub_absoluteFilePath);

    stub.set(ADDR(AddPackageThread, SymbolicLink), thread_stub_SymbolicLink);

    ASSERT_STREQ("", m_addPkgThread->dealPackagePath(" ").toUtf8());
}

TEST_F(ut_addPackageThread_Test, UT_AddPackageThread_dealPackagePath_SymbolicLink_1)
{
    QStringList dependsList;
    dependsList << "package1" << "package";
    m_addPkgThread->setPackages(dependsList);

    stub.set(ADDR(QFileInfo, absoluteFilePath), thread_stub_absoluteFilePath);

    stub.set(ADDR(AddPackageThread, SymbolicLink), thread_stub_SymbolicLink);
    stub.set(ADDR(DebFile, packageName), packagename);

    ASSERT_STREQ("", m_addPkgThread->dealPackagePath("/ 1").toUtf8());
}

TEST_F(ut_addPackageThread_Test, UT_AddPackageThread_dealPackagePath_SymbolicLink_2)
{
    stub.set(ADDR(AddPackageThread, mkTempDir), add_stub_is_open_true);
    m_addPkgThread->SymbolicLink("0","1");
}

TEST_F(ut_addPackageThread_Test, UT_AddPackageThread_dealPackagePath_mkTempDir)
{
    stub.set((bool(QDir::*)(const QString &)const)ADDR(QDir, mkdir), ut_mkTempDir);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((bool(QDir::*)()const)ADDR(QDir, exists), ut_mkTempDir);

    m_addPkgThread->mkTempDir();
}

TEST_F(ut_addPackageThread_Test, UT_AddPackageThread_dealPackagePath_link)
{
    stub.set((bool(QFile::*)(const QString &))ADDR(QFile, link), apt_exits);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((bool(QDir::*)()const)ADDR(QDir, exists), ut_mkTempDir);

    QFile file;
    m_addPkgThread->link("","");
}
