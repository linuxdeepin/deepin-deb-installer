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

#include "../deb_installer/manager/PackageDependsStatus.h"
#include "../deb_installer/manager/packagesmanager.h"

#include <stub.h>
#include <QFuture>
#include <QDir>
#include <fstream>
#include <QFileInfo>
#include <QApt/DependencyInfo>
#include <QList>

#include <gtest/gtest.h>

using namespace QApt;

QString deb_arch_all()
{
    return "all";
}

QString deb_arch_i386()
{
    return "i386";
}

QStringList backend_architectures()
{
    return {"i386", "amd64"};
}

bool backend_init()
{
    return true;
}

bool deb_isValid()
{
    return true;
}

QByteArray deb_md5Sum()
{
    return "0010";
}

int deb_installSize()
{
    return 0;
}

QString deb_packageName()
{
    return "";
}

QString deb_longDescription()
{
    return "longDescription";
}

QString deb_version()
{
    return "version";
}


QList<DependencyItem> deb_conflicts()
{
    DependencyInfo info("packageName", "0.0", RelationType::Equals, Depends);
    QList<DependencyInfo> dependencyItem;
    dependencyItem << info;
    QList<DependencyItem> conflicts;
    conflicts << dependencyItem;

    return conflicts;
}

QList<DependencyItem> deb_conflicts_null()
{
    return {};

}

Package *packageWithArch(QString, QString, QString)
{
    return nullptr;
}

PackageList backend_availablePackages()
{
    return {};
}

QLatin1String package_name()
{
    return QLatin1String("name");
}

QString package_version()
{
    return "version";
}

QString package_architecture()
{
    return "i386";
}

QList<DependencyItem> conflicts()
{
    qDebug() << "conflicts";
    DependencyInfo info("packageName", "0.0", RelationType::Equals, Conflicts);
    QList<DependencyInfo> dependencyItem;
    dependencyItem << info;
    QList<DependencyItem> conflicts;
    conflicts << dependencyItem;

    return conflicts;
}

QList<DependencyItem> package_conflicts()
{
    DependencyInfo info("packageName", "0.0", RelationType::Equals, Conflicts);
    QList<DependencyInfo> dependencyItem;
    dependencyItem << info;
    QList<DependencyItem> conflicts;
    conflicts << dependencyItem;

    return conflicts;
}

bool stub_is_open_true()
{
    qDebug() << "stb——is_open";
    return true;
}

bool stub_is_open_false()
{
    qDebug() << "stb——is_open";
    return false;
}

void stub_open(const std::string &__s, std::ios_base::openmode __mode)
{
    Q_UNUSED(__s);
    Q_UNUSED(__mode);
    qDebug() << "stb——open";
}

void stub_close()
{

}

PackageDependsStatus stub_getPackageDependsStatus(const int )
{
    return PackageDependsStatus::ok();
}



QString stub_SymbolicLink(QString previousName, QString packageName)
{
    Q_UNUSED(previousName);
    Q_UNUSED(packageName);

    return "";
}

QString stub_absoluteFilePath()
{
    return "";
}

bool stub_permission_true(QFile::Permissions permissions)
{
    Q_UNUSED(permissions);
    return true;
}

bool stub_permission_false(QFile::Permissions permissions)
{
    Q_UNUSED(permissions);
    return false;
}


QString stub_dealPackagePath(QString packagePath)
{
    Q_UNUSED(packagePath);
    return "";
}

bool stub_dealInvalidPackage(QString )
{
    return true;
}

bool deb_isValid_false()
{
    return true;
}


Package *package_package(const QString &name)
{
    Q_UNUSED(name);
    return nullptr;
}

bool package_isInstalled()
{
    return true;
}


QString package_installedVersion()
{
    return "";
}

int package_compareVersion()
{
    return 0;
}


Package *backend_package(const QString &name)
{
    return nullptr;
}

class ut_packagesManager_test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() override;
    void TearDown() override;

    PackagesManager *m_packageManager = nullptr;
    Stub stub;
};

void ut_packagesManager_test::SetUp()
{
    stub.set(ADDR(DebFile, architecture), deb_arch_all);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(DebFile, isValid), deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);
//    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set((Package * (Backend::*)(const QString &) const)ADDR(Backend, package), package_package);
    m_packageManager = new PackagesManager();
    usleep(10 * 1000);
}

void ut_packagesManager_test::TearDown()
{
    delete m_packageManager;
}

TEST_F(ut_packagesManager_test, PackageManager_UT_isBackendReady)
{
    ASSERT_TRUE(m_packageManager->isBackendReady());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_dealPackagePath_SymbolicLink)
{
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, SymbolicLink), stub_SymbolicLink);
    ASSERT_STREQ("", m_packageManager->dealPackagePath("/ ").toUtf8());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_dealPackagePath_AbsolutePath)
{
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(QFileInfo, absoluteFilePath), stub_absoluteFilePath);
    stub.set(ADDR(PackagesManager, SymbolicLink), stub_SymbolicLink);
    ASSERT_STREQ("", m_packageManager->dealPackagePath(" ").toUtf8());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_dealInvalidPackage_true)
{
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set((void (std::fstream::*)(const std::string & __s, std::ios_base::openmode __mode))ADDR(std::fstream, open), stub_open);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, is_open), stub_is_open_true);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, close), stub_close);

    m_packageManager->dealInvalidPackage("/1");
}

TEST_F(ut_packagesManager_test, PackageManager_UT_dealInvalidPackage_false)
{
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set((void (std::fstream::*)(const std::string & __s, std::ios_base::openmode __mode))ADDR(std::fstream, open), stub_open);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, is_open), stub_is_open_false);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, close), stub_close);

    m_packageManager->dealInvalidPackage("/1");
}

TEST_F(ut_packagesManager_test, PackageManager_UT_dealInvalidPackage_NoPermission)
{
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set((void (std::fstream::*)(const std::string & __s, std::ios_base::openmode __mode))ADDR(std::fstream, open), stub_open);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, is_open), stub_is_open_false);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, close), stub_close);
    stub.set(ADDR(QFileInfo, permission), stub_permission_false);
    m_packageManager->dealInvalidPackage("/1");
}

TEST_F(ut_packagesManager_test, PackageManager_UT_appendPackage)
{
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    m_packageManager->appendPackage({"/1"});

    ASSERT_FALSE(m_packageManager->m_packageMd5.isEmpty());

}

TEST_F(ut_packagesManager_test, PackageManager_UT_appendPackage_invalid)
{
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    m_packageManager->appendPackage({"/1"});

    ASSERT_FALSE(m_packageManager->m_packageMd5.isEmpty());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_appendPackage_openFailed)
{
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    m_packageManager->appendPackage({"/1"});

    ASSERT_FALSE(m_packageManager->m_packageMd5.isEmpty());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_refreshPage)
{
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);

    m_packageManager->m_preparedPackages.clear();
    m_packageManager->m_preparedPackages.append("/1");
    m_packageManager->refreshPage(2);
    m_packageManager->m_preparedPackages.append("/2");
    m_packageManager->refreshPage(2);
    m_packageManager->m_preparedPackages.append("/3");
    m_packageManager->refreshPage(2);

}

TEST_F(ut_packagesManager_test, PackageManager_UT_isArchError)
{
    m_packageManager->appendPackage({"/1"});
    m_packageManager->m_preparedPackages.append("0");
    ASSERT_FALSE(m_packageManager->isArchError(0));
}

TEST_F(ut_packagesManager_test, PackageManager_UT_isArchError_1)
{
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    m_packageManager->appendPackage({"/1"});
    m_packageManager->m_preparedPackages.append("0");
    ASSERT_FALSE(m_packageManager->isArchError(0));
}

TEST_F(ut_packagesManager_test, PackageManager_UT_packageConflictStat)
{

    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);

    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts_null);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    m_packageManager->appendPackage({"/1"});

    ConflictResult cr = m_packageManager->packageConflictStat(0);
    ASSERT_TRUE(cr.is_ok());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_isConflictSatisfy)
{
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);

//    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    m_packageManager->appendPackage({"/1"});
    ConflictResult cr = m_packageManager->isConflictSatisfy("i386", conflicts());
    ASSERT_TRUE(cr.is_ok());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_isConflictSatisfy_01)
{
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
//    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    m_packageManager->appendPackage({"/1"});
    ConflictResult cr = m_packageManager->isConflictSatisfy("i386", conflicts());
    ASSERT_TRUE(cr.is_ok());

}

TEST_F(ut_packagesManager_test, PackageManager_UT_isInstalledConflict)
{

    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(Package, isInstalled), package_isInstalled);
    stub.set(ADDR(Package, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    ConflictResult cr = m_packageManager->isInstalledConflict("package name", "packageversion", "i386");
    ASSERT_TRUE(cr.is_ok());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_isConflictSatisfy_0001)
{
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    Package *package = nullptr;
    stub.set(ADDR(Package, name), package_name);
    stub.set(ADDR(Package, version), package_version);
    stub.set(ADDR(Package, architecture), package_architecture);
    stub.set(ADDR(Package, conflicts), package_conflicts);


    ConflictResult cr = m_packageManager->isConflictSatisfy("i386", package);
    ASSERT_TRUE(cr.is_ok());
    delete package;
}

TEST_F(ut_packagesManager_test, PackageManager_UT_packageInstallStatus)
{
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);

    m_packageManager->appendPackage({"/1"});
    ASSERT_EQ(m_packageManager->packageInstallStatus(0), 0);
}
TEST_F(ut_packagesManager_test, PackageManager_UT_DealDependResult)
{
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);

    m_packageManager->m_dependInstallMark.append("test success");
    m_packageManager->DealDependResult(4, 0, "");
    m_packageManager->DealDependResult(2, 0, "");
    m_packageManager->DealDependResult(5, 0, "");
}
//TEST_F(ut_packagesManager_test,)
//{

//}
//TEST_F(ut_packagesManager_test,)
//{

//}
//TEST_F(ut_packagesManager_test,)
//{

//}


QList<DependencyItem> deb_depends()
{
    DependencyInfo info("packageName", "0.0", RelationType::Equals, Depends);
    QList<DependencyInfo> dependencyItem;
    dependencyItem << info;
    QList<DependencyItem> conflicts;
    conflicts << dependencyItem;

    return conflicts;
}

bool isInstalled()
{
    return true;
}

bool ut_isArchError(int index)
{
    Q_UNUSED(index);
    return true;
}

bool ut_isArchError_false(int index)
{
    Q_UNUSED(index);
    return false;
}


TEST(PackageManager_UT, PackageManager_UT_getPackageDependsStatus)
{
    Stub stub;

    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package * (Backend::*)(const QString &) const)ADDR(Backend, package), package_package);
    stub.set(ADDR(DebFile, isValid), deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);

    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, isArchError), ut_isArchError_false);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->appendPackage({"/"});
    qInfo() << package_conflicts().size();


    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(Package, isInstalled), isInstalled);
    PackageDependsStatus pd = p->getPackageDependsStatus(0);

    ASSERT_TRUE(pd.isBreak());

    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_getPackageDependsStatus_01)
{
    Stub stub;

    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package * (Backend::*)(const QString &) const)ADDR(Backend, package), package_package);
    stub.set(ADDR(DebFile, isValid), deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);

    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, isArchError), ut_isArchError);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->appendPackage({"/"});


    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(Package, isInstalled), isInstalled);
    PackageDependsStatus pd = p->getPackageDependsStatus(0);

    ASSERT_EQ(pd.status, 5);

    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_packageInstalledVersion)
{
    Stub stub;

    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package * (Backend::*)(const QString &) const)ADDR(Backend, package), package_package);
    stub.set(ADDR(DebFile, isValid), deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);

    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

//    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->appendPackage({"/"});

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);

    p->m_packageInstallStatus[p->m_packageMd5[0]] = 0;
    QString version = p->packageInstalledVersion(0);

    ASSERT_TRUE(version.isEmpty());
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_packageAvailableDepends)
{
    Stub stub;

    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package * (Backend::*)(const QString &) const)ADDR(Backend, package), package_package);
    stub.set(ADDR(DebFile, isValid), deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);

    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->appendPackage({"/"});

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);

    QStringList ads = p->packageAvailableDepends(0);

    ASSERT_TRUE(ads.isEmpty());
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_specialPackage)
{
    Stub stub;

    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(DebFile, isValid), deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);

    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    PackagesManager *p = new PackagesManager();

    p->appendPackage({"/"});
    usleep(10 * 1000);

    QMap<QString, QString> sp = p->specialPackage();

    ASSERT_FALSE(sp.empty());
    delete p;
}

QStringList package_requiredByList()
{
    QStringList rbl;
    rbl << "package1" << "package2" << "package3" << "package4";
    return rbl;
}
TEST(PackageManager_UT, PackageManager_UT_packageReverseDependsList)
{
    Stub stub;

    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package * (Backend::*)(const QString &) const)ADDR(Backend, package), package_package);
    stub.set(ADDR(DebFile, isValid), deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);

    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->appendPackage({"/"});

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);

    stub.set(ADDR(Package, requiredByList), package_requiredByList);
//    stub.set(ADDR(Package, compareVersion),package_compareVersion);

    QStringList ads = p->packageReverseDependsList("", "");

    ASSERT_TRUE(ads.isEmpty());
    delete p;
}

bool backend_reloadCache()
{
    return true;
}

TEST(PackageManager_UT, PackageManager_UT_reset)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(Backend, reloadCache), backend_reloadCache);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package * (Backend::*)(const QString &) const)ADDR(Backend, package), package_package);
    stub.set(ADDR(DebFile, isValid), deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);

    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->appendPackage({"/"});

    p->reset();

    ASSERT_TRUE(p->m_appendedPackagesMd5.isEmpty());
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_resetPackageDependsStatus)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(Backend, reloadCache), backend_reloadCache);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package * (Backend::*)(const QString &) const)ADDR(Backend, package), package_package);
    stub.set(ADDR(DebFile, isValid), deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);

    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->appendPackage({"/"});

    p->resetPackageDependsStatus(0);

    ASSERT_TRUE(p->m_packageMd5DependsStatus.isEmpty());

    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_removePackage)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(Backend, reloadCache), backend_reloadCache);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package * (Backend::*)(const QString &) const)ADDR(Backend, package), package_package);
    stub.set(ADDR(DebFile, isValid), deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);

    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->appendPackage({"/"});

    p->removePackage(0);

    ASSERT_TRUE(p->m_appendedPackagesMd5.isEmpty());

    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_removePackage_removeMulti)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(Backend, reloadCache), backend_reloadCache);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package * (Backend::*)(const QString &) const)ADDR(Backend, package), package_package);
    stub.set(ADDR(DebFile, isValid), deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);

    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->m_preparedPackages.append("/0");
    p->m_preparedPackages.append("/1");
    p->m_preparedPackages.append("/2");
    p->m_packageMd5.append("0");
    p->m_packageMd5.append("0");
    p->m_packageMd5.append("0");

    p->removePackage(0);

    ASSERT_EQ(p->m_preparedPackages.size(), 2);

    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_removePackage_removeTwo)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(Backend, reloadCache), backend_reloadCache);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package * (Backend::*)(const QString &) const)ADDR(Backend, package), package_package);
    stub.set(ADDR(DebFile, isValid), deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);

    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);

    p->m_preparedPackages.append("/1");
    p->m_preparedPackages.append("/2");
    p->m_packageMd5.append("0");
    p->m_packageMd5.append("0");

    p->removePackage(0);

    ASSERT_EQ(p->m_preparedPackages.size(), 1);

    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_rmTempDir)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);

    stub.set(ADDR(Backend, init), backend_init);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->rmTempDir();
    ASSERT_STREQ(p->SymbolicLink("test", "test").toLocal8Bit(), (QString("/tmp/LinkTemp/") + QString("test")).toLocal8Bit());
    delete p;
}
TEST(PackageManager_UT, PackageManager_UT_020)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);

    stub.set(ADDR(Backend, init), backend_init);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    ASSERT_TRUE(p->rmTempDir());
    delete p;
}

bool pm_mkdir(const QString &dirName)
{
    Q_UNUSED(dirName);
    return true;
}

bool pm_exits()
{
    return true;
}
TEST(PackageManager_UT, PackageManager_UT_mkTempDir)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);

    stub.set(ADDR(Backend, init), backend_init);

    stub.set((bool(QDir::*)(const QString &)const)ADDR(QDir, mkdir), pm_mkdir);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((bool(QDir::*)()const)ADDR(QDir, exists), pm_exits);
    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    ASSERT_TRUE(p->mkTempDir());
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_link)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);

    stub.set(ADDR(Backend, init), backend_init);

    PackagesManager *p = new PackagesManager();

    usleep(10 * 1000);
    ASSERT_STREQ(p->link("test", "test1").toLocal8Bit(), (QString("test")).toLocal8Bit());
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_packageWithArch)
{
    Stub stub;

    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package * (Backend::*)(const QString &) const)ADDR(Backend, package), package_package);
    stub.set(ADDR(DebFile, isValid), deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);

    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->appendPackage({"/"});
    p->appendPackage(QStringList() << "test");

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);

    stub.set(ADDR(Package, requiredByList), package_requiredByList);
//    stub.set(ADDR(Package, compareVersion),package_compareVersion);

    ASSERT_FALSE(p->packageWithArch("package", "arch"));

    p->deleteLater();
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_appendPackageFinished)
{
    Stub stub;
    stub.set(ADDR(Backend, init), backend_init);
    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->appendPackageFinished();
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_isArchMatches_01)
{
    ASSERT_TRUE(isArchMatches("all", "", 0));

}

TEST(PackageManager_UT, PackageManager_UT_isArchMatches_02)
{
    ASSERT_FALSE(isArchMatches("amd64", "", 0));

}

TEST(PackageManager_UT, PackageManager_UT_resolvMultiArchAnnotation)
{
    ASSERT_STREQ(resolvMultiArchAnnotation("all", "", InvalidMultiArchType).toLocal8Bit(), "");
}

TEST(PackageManager_UT, PackageManager_UT_resolvMultiArchAnnotation_01)
{
    ASSERT_STREQ(resolvMultiArchAnnotation("testAnnotation", "", InvalidMultiArchType).toLocal8Bit(), ":testAnnotation");
}

TEST(PackageManager_UT, PackageManager_UT_resolvMultiArchAnnotation_02)
{
    ASSERT_STREQ(resolvMultiArchAnnotation(":i386", "", InvalidMultiArchType).toLocal8Bit(), ":i386");
}

TEST(PackageManager_UT, PackageManager_UT_dependencyVersionMatch)
{
    ASSERT_TRUE(dependencyVersionMatch(0, Equals));
}

TEST(PackageManager_UT, PackageManager_UT_dependencyVersionMatch_01)
{
    ASSERT_TRUE(dependencyVersionMatch(0, LessOrEqual));
}

TEST(PackageManager_UT, PackageManager_UT_dependencyVersionMatch_02)
{
    ASSERT_TRUE(dependencyVersionMatch(-1, LessThan));
}

TEST(PackageManager_UT, PackageManager_UT_dependencyVersionMatch_03)
{
    ASSERT_TRUE(dependencyVersionMatch(1, GreaterThan));
}

TEST(PackageManager_UT, PackageManager_UT_dependencyVersionMatch_04)
{
    ASSERT_TRUE(dependencyVersionMatch(1, NotEqual));
}

TEST(PackageManager_UT, PackageManager_UT_dependencyVersionMatch_05)
{
    ASSERT_TRUE(dependencyVersionMatch(1, GreaterOrEqual));
}

TEST(PackageManager_UT, PackageManager_UT_dependencyVersionMatch_06)
{
    ASSERT_TRUE(dependencyVersionMatch(1, NoOperand));
}


TEST(PackageManager_UT, PackageManager_UT_backend)
{

    Stub stub;
    stub.set(ADDR(Backend, init), backend_init);
    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    ASSERT_TRUE(p->backend());

    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_package)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_all);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(DebFile, isValid), deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->appendPackage({"/1"});

    ASSERT_STREQ(p->package(0).toLocal8Bit(), "");
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_checkDependsPackageStatus)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set((Package * (Backend::*)(const QString &) const)ADDR(Backend, package), package_package);
    stub.set(ADDR(DebFile, isValid), deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    QSet<QString> set;
    p->checkDependsPackageStatus(set, "", conflicts());
    delete p;
}
