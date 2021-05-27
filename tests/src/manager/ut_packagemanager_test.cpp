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

#include "../deb-installer/manager/PackageDependsStatus.h"
#include "../deb-installer/manager/packagesmanager.h"

#include <stub.h>
#include <QFuture>
#include <QDir>
#include <fstream>
#include <QFileInfo>
#include <QApt/DependencyInfo>
#include <QList>

#include <gtest/gtest.h>
typedef Result<QString> ConflictResult;
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


Package *backend_package(const QString &)
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
    stub.set(ADDR(PackagesManager, rmTempDir), stub_is_open_false);
    delete m_packageManager;
}

TEST_F(ut_packagesManager_test, PackageManager_UT_isBackendReady)
{
    ASSERT_TRUE(m_packageManager->isBackendReady());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_dealPackagePath_SymbolicLink)
{
    ASSERT_STREQ("/tmp/LinkTemp/1", m_packageManager->SymbolicLink("0","1").toUtf8());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_dealPackagePath_SymbolicLink_01)
{
    stub.set(ADDR(PackagesManager, mkTempDir), stub_is_open_false);
//    ASSERT_STREQ("0", m_packageManager->SymbolicLink("0","1").toUtf8());
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
void stub_appendNoThread(QStringList , int )
{

}

void stub_start(const QString &program, const QStringList &arguments, QIODevice::OpenModeFlag mode)
{
    Q_UNUSED(program);
    Q_UNUSED(arguments);
    Q_UNUSED(mode);
    return;
}
void stub_qthread_start(QThread::Priority = QThread::InheritPriority)
{
    return;
}
TEST_F(ut_packagesManager_test, PackageManager_UT_appendPackage_multi)
{
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    stub.set(ADDR(PackagesManager, appendNoThread), stub_appendNoThread);
    stub.set(ADDR(QThread,start), stub_qthread_start);
    m_packageManager->appendPackage({"/1", "/2"});

    ASSERT_TRUE(m_packageManager->m_packageMd5.isEmpty());
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
    m_packageManager->m_preparedPackages.insert(0,"/1");
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
    ASSERT_FALSE(cr.is_ok());
    delete package;
}
const ConflictResult stub_isInstalledConflict(const QString &, const QString &,
                                         const QString &)
{
    return ConflictResult::err("");
}
TEST_F(ut_packagesManager_test, PackageManager_UT_isConflictSatisfy_0002)
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

    stub.set(ADDR(PackagesManager,isInstalledConflict),stub_isInstalledConflict);

    ConflictResult cr = m_packageManager->isConflictSatisfy("i386", package);
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
    m_packageManager->slotDealDependResult(4, 0, "");
    m_packageManager->slotDealDependResult(2, 0, "");
    m_packageManager->slotDealDependResult(5, 0, "");
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

TEST_F(ut_packagesManager_test, PackageManager_UT_getPackageDependsStatus)
{
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, isArchError), ut_isArchError_false);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    usleep(10 * 1000);
    m_packageManager->appendPackage({"/"});
    qInfo() << package_conflicts().size();


    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(Package, isInstalled), isInstalled);
    PackageDependsStatus pd = m_packageManager->getPackageDependsStatus(0);

    ASSERT_TRUE(pd.isBreak());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_getPackageDependsStatus_01)
{
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, isArchError), ut_isArchError);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    usleep(10 * 1000);
    m_packageManager->appendPackage({"/"});

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(Package, isInstalled), isInstalled);
    PackageDependsStatus pd = m_packageManager->getPackageDependsStatus(0);

    ASSERT_EQ(pd.status, 5);
}

bool stub_isBlackApplication_false(QString)
{
    return false;
}

bool stub_isBlackApplication_true(QString)
{
    return true;
}
TEST_F(ut_packagesManager_test, PackageManager_UT_getPackageDependsStatus_02)
{
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, isArchError), ut_isArchError);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    stub.set(ADDR(PackagesManager, isBlackApplication), stub_isBlackApplication_false);

    usleep(10 * 1000);
    m_packageManager->appendPackage({"/"});
    m_packageManager->m_packageMd5DependsStatus.clear();

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(Package, isInstalled), isInstalled);
    PackageDependsStatus pd = m_packageManager->getPackageDependsStatus(0);

    ASSERT_EQ(pd.status, 2);
}
TEST_F(ut_packagesManager_test, PackageManager_UT_getPackageDependsStatus_03)
{
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, isArchError), ut_isArchError);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    stub.set(ADDR(PackagesManager, isBlackApplication), stub_isBlackApplication_true);

    usleep(10 * 1000);
    m_packageManager->appendPackage({"/"});
    m_packageManager->m_packageMd5DependsStatus.clear();

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(Package, isInstalled), isInstalled);
    PackageDependsStatus pd = m_packageManager->getPackageDependsStatus(0);

    ASSERT_EQ(pd.status, 6);
}

const ConflictResult stub_isConflictSatisfy(const QString &, const QList<QApt::DependencyItem> &)
{
    return ConflictResult::err("");
}

TEST_F(ut_packagesManager_test, PackageManager_UT_packageInstalledVersion)
{
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    usleep(10 * 1000);
    m_packageManager->appendPackage({"/"});

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);

    m_packageManager->m_packageInstallStatus[m_packageManager->m_packageMd5[0]] = 0;
    QString version = m_packageManager->packageInstalledVersion(0);

    ASSERT_TRUE(version.isEmpty());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_packageAvailableDepends)
{
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    usleep(10 * 1000);
    m_packageManager->appendPackage({"/"});

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);

    QStringList ads = m_packageManager->packageAvailableDepends(0);

    ASSERT_TRUE(ads.isEmpty());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_specialPackage)
{
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    m_packageManager->appendPackage({"/"});
    usleep(10 * 1000);

    QMap<QString, QString> sp = m_packageManager->specialPackage();

    ASSERT_FALSE(sp.empty());
}

QStringList package_requiredByList()
{
    QStringList rbl;
    rbl << "package1" << "package2" << "package3" << "package4";
    return rbl;
}
TEST_F(ut_packagesManager_test, PackageManager_UT_packageReverseDependsList)
{
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    usleep(10 * 1000);
    m_packageManager->appendPackage({"/"});

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);

    stub.set(ADDR(Package, requiredByList), package_requiredByList);
//    stub.set(ADDR(Package, compareVersion),package_compareVersion);

    QStringList ads = m_packageManager->packageReverseDependsList("", "");

    ASSERT_TRUE(ads.isEmpty());
}

bool backend_reloadCache()
{
    return true;
}

TEST_F(ut_packagesManager_test, PackageManager_UT_reset)
{
    stub.set(ADDR(Backend, reloadCache), backend_reloadCache);
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    usleep(10 * 1000);
    m_packageManager->appendPackage({"/"});

    m_packageManager->reset();

    ASSERT_TRUE(m_packageManager->m_appendedPackagesMd5.isEmpty());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_resetPackageDependsStatus)
{
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    usleep(10 * 1000);
    m_packageManager->appendPackage({"/"});

    m_packageManager->resetPackageDependsStatus(0);

    ASSERT_TRUE(m_packageManager->m_packageMd5DependsStatus.isEmpty());
}

bool stub_reloadCache()
{
    return false;
}
TEST_F(ut_packagesManager_test, PackageManager_UT_resetPackageDependsStatus_01)
{
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    stub.set(ADDR(Backend, reloadCache),stub_reloadCache);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);


    usleep(10 * 1000);
    m_packageManager->appendPackage({"/"});
    m_packageManager->m_packageMd5[0] = "1";
    PackageDependsStatus st = PackageDependsStatus::ok();
    m_packageManager->m_packageMd5DependsStatus.insert("1",st);
    m_packageManager->resetPackageDependsStatus(0);

    ASSERT_TRUE(m_packageManager->m_packageMd5DependsStatus.isEmpty());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_removePackage)
{
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    usleep(10 * 1000);
    m_packageManager->appendPackage({"/"});

    m_packageManager->removePackage(0);

    ASSERT_TRUE(m_packageManager->m_appendedPackagesMd5.isEmpty());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_removePackage_01)
{
    m_packageManager->m_preparedPackages.append("1");

    m_packageManager->removePackage(-1);

    ASSERT_EQ(m_packageManager->m_preparedPackages.size(),1);
}

TEST_F(ut_packagesManager_test, PackageManager_UT_removePackage_02)
{
    m_packageManager->m_preparedPackages.append("1");

    m_packageManager->m_packageMd5.insert(0,"1");
    m_packageManager->m_dependInstallMark.append("1");

    m_packageManager->removePackage(0);


    ASSERT_EQ(m_packageManager->m_preparedPackages.size(),0);
}

TEST_F(ut_packagesManager_test, PackageManager_UT_removePackage_removeMulti)
{
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);

    usleep(10 * 1000);
    m_packageManager->m_preparedPackages.append("/0");
    m_packageManager->m_preparedPackages.append("/1");
    m_packageManager->m_preparedPackages.append("/2");
    m_packageManager->m_packageMd5.append("0");
    m_packageManager->m_packageMd5.append("0");
    m_packageManager->m_packageMd5.append("0");

    m_packageManager->removePackage(0);

    ASSERT_EQ(m_packageManager->m_preparedPackages.size(), 2);
}

TEST_F(ut_packagesManager_test, PackageManager_UT_removePackage_removeTwo)
{
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);

    usleep(10 * 1000);

    m_packageManager->m_preparedPackages.append("/1");
    m_packageManager->m_preparedPackages.append("/2");
    m_packageManager->m_packageMd5.append("0");
    m_packageManager->m_packageMd5.append("0");

    m_packageManager->removePackage(0);

    ASSERT_EQ(m_packageManager->m_preparedPackages.size(), 1);
}

TEST_F(ut_packagesManager_test, PackageManager_UT_rmTempDir)
{

    usleep(10 * 1000);
    m_packageManager->rmTempDir();
    ASSERT_STREQ(m_packageManager->SymbolicLink("test", "test").toLocal8Bit(), (QString("/tmp/LinkTemp/") + QString("test")).toLocal8Bit());
}
TEST_F(ut_packagesManager_test, PackageManager_UT_020)
{

    usleep(10 * 1000);
    ASSERT_TRUE(m_packageManager->rmTempDir());
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
TEST_F(ut_packagesManager_test, PackageManager_UT_mkTempDir)
{

    stub.set((bool(QDir::*)(const QString &)const)ADDR(QDir, mkdir), pm_mkdir);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((bool(QDir::*)()const)ADDR(QDir, exists), pm_exits);

    ASSERT_TRUE(m_packageManager->mkTempDir());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_link)
{
    m_packageManager->link("test", "test1").toLocal8Bit(), (QString("test")).toLocal8Bit();
}


TEST_F(ut_packagesManager_test, PackageManager_UT_packageWithArch)
{
//    stub.set(ADDR(DebFile, architecture), deb_arch_i386);

    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    usleep(10 * 1000);
    m_packageManager->appendPackage({"/"});
    m_packageManager->appendPackage(QStringList() << "test");

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);

    stub.set(ADDR(Package, requiredByList), package_requiredByList);
//    stub.set(ADDR(Package, compareVersion),package_compareVersion);

    ASSERT_FALSE(m_packageManager->packageWithArch("package", "arch"));

    m_packageManager->deleteLater();
}

TEST_F(ut_packagesManager_test, PackageManager_UT_appendPackageFinished)
{
    usleep(10 * 1000);
    m_packageManager->slotAppendPackageFinished();
}

TEST_F(ut_packagesManager_test, PackageManager_UT_isArchMatches_01)
{
    ASSERT_TRUE(m_packageManager->isArchMatches("all", "", 0));
}

TEST_F(ut_packagesManager_test, PackageManager_UT_isArchMatches_02)
{
    ASSERT_FALSE(m_packageManager->isArchMatches("amd64", "", 0));
}

TEST_F(ut_packagesManager_test, PackageManager_UT_resolvMultiArchAnnotation)
{
    ASSERT_STREQ(m_packageManager->resolvMultiArchAnnotation("all", "", InvalidMultiArchType).toLocal8Bit(), "");
}

TEST_F(ut_packagesManager_test, PackageManager_UT_resolvMultiArchAnnotation_any)
{
    ASSERT_STREQ(m_packageManager->resolvMultiArchAnnotation("any", "", InvalidMultiArchType).toLocal8Bit(), "");
}

TEST_F(ut_packagesManager_test, PackageManager_UT_resolvMultiArchAnnotation_MultiArchForeign)
{
    ASSERT_STREQ(m_packageManager->resolvMultiArchAnnotation("any", "", MultiArchForeign).toLocal8Bit(), "");
}

TEST_F(ut_packagesManager_test, PackageManager_UT_resolvMultiArchAnnotation_01)
{
    ASSERT_STREQ(m_packageManager->resolvMultiArchAnnotation("testAnnotation", "", InvalidMultiArchType).toLocal8Bit(), ":testAnnotation");
}

TEST_F(ut_packagesManager_test, PackageManager_UT_resolvMultiArchAnnotation_02)
{
    ASSERT_STREQ(m_packageManager->resolvMultiArchAnnotation(":i386", "", InvalidMultiArchType).toLocal8Bit(), ":i386");
}

TEST_F(ut_packagesManager_test, PackageManager_UT_dependencyVersionMatch)
{
    ASSERT_TRUE(m_packageManager->dependencyVersionMatch(0, Equals));
}

TEST_F(ut_packagesManager_test, PackageManager_UT_dependencyVersionMatch_01)
{
    ASSERT_TRUE(m_packageManager->dependencyVersionMatch(0, LessOrEqual));
}

TEST_F(ut_packagesManager_test, PackageManager_UT_dependencyVersionMatch_02)
{
    ASSERT_TRUE(m_packageManager->dependencyVersionMatch(-1, LessThan));
}

TEST_F(ut_packagesManager_test, PackageManager_UT_dependencyVersionMatch_03)
{
    ASSERT_TRUE(m_packageManager->dependencyVersionMatch(1, GreaterThan));
}

TEST_F(ut_packagesManager_test, PackageManager_UT_dependencyVersionMatch_04)
{
    ASSERT_TRUE(m_packageManager->dependencyVersionMatch(1, NotEqual));
}

TEST_F(ut_packagesManager_test, PackageManager_UT_dependencyVersionMatch_05)
{
    ASSERT_TRUE(m_packageManager->dependencyVersionMatch(1, GreaterOrEqual));
}

TEST_F(ut_packagesManager_test, PackageManager_UT_dependencyVersionMatch_06)
{
    ASSERT_TRUE(m_packageManager->dependencyVersionMatch(1, NoOperand));
}

TEST_F(ut_packagesManager_test, PackageManager_UT_backend)
{
    usleep(10 * 1000);
    ASSERT_TRUE(m_packageManager->backend());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_package)
{
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    usleep(10 * 1000);
    m_packageManager->appendPackage({"/1"});

    ASSERT_STREQ(m_packageManager->package(0).toLocal8Bit(), "");
}

TEST_F(ut_packagesManager_test, PackageManager_UT_checkDependsPackageStatus)
{
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    usleep(10 * 1000);
    QSet<QString> set;
    m_packageManager->checkDependsPackageStatus(set, "", conflicts());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_getPackageMd5)
{
    m_packageManager->m_packageMd5.append({"1","2"});
    m_packageManager->getPackageMd5(0);
}

TEST_F(ut_packagesManager_test, PackageManager_UT_getBlackApplications)
{
    stub.set((bool(QFile::*)()const)ADDR(QFile, exists), pm_exits);
    m_packageManager->getBlackApplications();
}

TEST_F(ut_packagesManager_test, PackageManager_UT_isBlackApplication)
{
    m_packageManager->m_blackApplicationList.append("black");


    ASSERT_TRUE(m_packageManager->isBlackApplication("black"));
}
