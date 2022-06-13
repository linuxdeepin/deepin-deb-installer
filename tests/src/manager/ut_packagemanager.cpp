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
#include "../deb-installer/manager/DealDependThread.h"
#include "../deb-installer/manager/AddPackageThread.h"
#include "../deb-installer/model/deblistmodel.h"

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

Package *stub_packageWithArch(QString, QString, QString)
{
    return nullptr;
}

Package *stub_avaialbe_packageWithArch(QString, QString, QString)
{
    Backend *bac = nullptr;
    pkgCache::PkgIterator packageIter;
//    Package *package = new Package(bac,packageIter);
     QScopedPointer<Package> package(new Package(bac, packageIter));
    return package.get();
}

PackageList backend_availablePackages()
{
    return {};
}

QLatin1String package_name()
{
    return QLatin1String("");
}

QString deb_package_name()
{
    return QString("deepin-elf-verify");
}
QString deb_package_name1()
{
    return QString("deepin-wine");
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

QString package_installedVersion1()
{
    return "deb";
}

int package_compareVersion()
{
    return 0;
}

int package_compareVersion1()
{
    return -1;
}

int package_compareVersion2()
{
    return 1;
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

    m_packageManager = new PackagesManager();
    usleep(10 * 1000);
}

void ut_packagesManager_test::TearDown()
{
    stub.set(ADDR(PackagesManager, rmTempDir), stub_is_open_false);

    for (auto pkg : m_packageManager->m_backendFuture.result()->availablePackages()) {
        delete pkg;
        pkg =  nullptr;
    }

    delete m_packageManager;
}

Package *packagesManager_package(const QString &)
{
    qDebug()<<"not null";
    Backend *bac = nullptr;
    pkgCache::PkgIterator packageIter;
    QScopedPointer<Package> package(new Package(bac, packageIter));
    return package.get();
}

TEST_F(ut_packagesManager_test, PackageManager_UT_isBackendReady)
{
    ASSERT_TRUE(m_packageManager->isBackendReady());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_dealPackagePath_SymbolicLink_01)
{
    stub.set(ADDR(PackagesManager, mkTempDir), stub_is_open_false);
    EXPECT_EQ("0", m_packageManager->SymbolicLink("0", "1"));
}

TEST_F(ut_packagesManager_test, PackageManager_UT_dealPackagePath_AbsolutePath)
{
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(QFileInfo, absoluteFilePath), stub_absoluteFilePath);
    stub.set(ADDR(PackagesManager, SymbolicLink), stub_SymbolicLink);
    ASSERT_STREQ("", m_packageManager->dealPackagePath(" ").toUtf8());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_dealPackagePath_space)
{
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(QFileInfo, absoluteFilePath), stub_absoluteFilePath);
    stub.set(ADDR(PackagesManager, SymbolicLink), stub_SymbolicLink);
    ASSERT_STREQ("", m_packageManager->dealPackagePath("/ ").toUtf8());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_dealInvalidPackage_true)
{
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set((void (std::fstream::*)(const std::string & __s, std::ios_base::openmode __mode))ADDR(std::fstream, open), stub_open);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, is_open), stub_is_open_true);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, close), stub_close);

    EXPECT_FALSE(m_packageManager->dealInvalidPackage("/1"));
}

void stub_qthread_start(QThread::Priority = QThread::InheritPriority)
{
    return;
}

TEST_F(ut_packagesManager_test, PackageManager_UT_dealInvalidPackage_false)
{
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set((void (std::fstream::*)(const std::string & __s, std::ios_base::openmode __mode))ADDR(std::fstream, open), stub_open);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, is_open), stub_is_open_false);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, close), stub_close);

    EXPECT_FALSE(m_packageManager->dealInvalidPackage("/1"));
}

TEST_F(ut_packagesManager_test, PackageManager_UT_dealInvalidPackage_NoPermission)
{
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set((void (std::fstream::*)(const std::string & __s, std::ios_base::openmode __mode))ADDR(std::fstream, open), stub_open);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, is_open), stub_is_open_false);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, close), stub_close);
    stub.set(ADDR(QFileInfo, permission), stub_permission_false);
    EXPECT_FALSE(m_packageManager->dealInvalidPackage("/1"));
}

TEST_F(ut_packagesManager_test, PackageManager_UT_appendPackage)
{
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    m_packageManager->appendPackage({"/1"});

    ASSERT_FALSE(m_packageManager->m_packageMd5.isEmpty());
    stub.set(ADDR(QThread, start), stub_qthread_start);
    m_packageManager->appendPackage(QStringList() << "/1"
                                                  << "/2");
    ASSERT_EQ(1, m_packageManager->m_pAddPackageThread->m_packages.size());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_appendPackage_invalid)
{
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    m_packageManager->appendPackage({"/1"});

    ASSERT_FALSE(m_packageManager->m_packageMd5.isEmpty());
    stub.set(ADDR(QThread, start), stub_qthread_start);
    m_packageManager->appendPackage(QStringList() << "/1"
                                                  << "/2");
    ASSERT_EQ(1, m_packageManager->m_pAddPackageThread->m_packages.size());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_appendPackage_openFailed)
{
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    m_packageManager->appendPackage({"/1"});

    ASSERT_FALSE(m_packageManager->m_packageMd5.isEmpty());
    stub.set(ADDR(QThread, start), stub_qthread_start);
    m_packageManager->appendPackage(QStringList() << "/1"
                                                  << "/2");
    ASSERT_EQ(1, m_packageManager->m_pAddPackageThread->m_packages.size());
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
    EXPECT_EQ(3, m_packageManager->m_preparedPackages.size());
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

    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);

    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts_null);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    m_packageManager->appendPackage({"/1"});

    ConflictResult cr = m_packageManager->packageConflictStat(0);
    ASSERT_TRUE(cr.is_ok());
    ASSERT_EQ(1, m_packageManager->m_preparedPackages.size());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_packageConflictStat_invalid)
{

    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);

    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts_null);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    m_packageManager->appendPackage({"/1"});

    ConflictResult cr = m_packageManager->packageConflictStat(-1);
    ASSERT_FALSE(cr.is_ok());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_isConflictSatisfy)
{
    stub.set((QApt::Package *(QApt::Backend::*)(const QString & name) const)ADDR(Backend, package), package_package);
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);

    m_packageManager->appendPackage({"/1"});
    ConflictResult cr = m_packageManager->isConflictSatisfy("i386", conflicts());
    ASSERT_TRUE(cr.is_ok());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_isConflictSatisfy_01)
{
    stub.set((QApt::Package *(QApt::Backend::*)(const QString & name) const)ADDR(Backend, package), package_package);

    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    m_packageManager->appendPackage({"/1"});
    ConflictResult cr = m_packageManager->isConflictSatisfy("i386", conflicts());
    ASSERT_TRUE(cr.is_ok());
}


MultiArchType ut_packagesManager_multiArchType()
{
    return MultiArchSame;
}

int ut_packagesManager_compareVersion(const QString &, const QString &){
    return 0;
}

bool ut_packagesManager_dependencyVersionMatch(const int, const RelationType)
{
    return false;
}

bool ut_packagesManager_dependencyVersionMatch1(const int, const RelationType)
{
    return true;
}


TEST_F(ut_packagesManager_test, PackageManager_UT_isConflictSatisfy_02)
{
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    stub.set(ADDR(PackagesManager, dependencyVersionMatch), stub_dealInvalidPackage);

    stub.set((QApt::Package *(QApt::Backend::*)(const QString & name) const)ADDR(Backend, package), packagesManager_package);
    stub.set(ADDR(Package, isInstalled), package_isInstalled);
    stub.set(ADDR(Package, name), package_name);
    stub.set(ADDR(Package, version), package_version);
    stub.set(ADDR(Package, architecture), package_architecture);
    stub.set(ADDR(Package, conflicts), package_conflicts);
    stub.set(ADDR(Package, multiArchType), ut_packagesManager_multiArchType);
    stub.set(ADDR(Package, multiArchTypeString), package_version);
    stub.set(ADDR(Package, installedVersion), package_version);
    stub.set(ADDR(Package, compareVersion), ut_packagesManager_compareVersion);
    stub.set(ADDR(Package, availableVersion), package_version);

    m_packageManager->appendPackage({"/1"});
    ConflictResult cr = m_packageManager->isConflictSatisfy("i386", conflicts());
    ASSERT_FALSE(cr.is_ok());
}

bool stub_isArchMatches(QString , const QString &, const int ){
    return false;
}

TEST_F(ut_packagesManager_test, PackageManager_UT_isConflictSatisfy_03)
{
    stub.set((QApt::Package *(QApt::Backend::*)(const QString & name) const)ADDR(Backend, package), packagesManager_package);
    stub.set(ADDR(Package, isInstalled), package_isInstalled);
    stub.set(ADDR(Package, name), package_name);
    stub.set(ADDR(Package, version), package_version);
    stub.set(ADDR(Package, architecture), package_architecture);
    stub.set(ADDR(Package, conflicts), package_conflicts);
    stub.set(ADDR(Package, multiArchType), ut_packagesManager_multiArchType);
    stub.set(ADDR(Package, multiArchTypeString), package_version);
    stub.set(ADDR(Package, installedVersion), package_version);
    stub.set(ADDR(Package, compareVersion), ut_packagesManager_compareVersion);
    stub.set(ADDR(Package, availableVersion), package_version);
    stub.set(ADDR(PackagesManager, isArchMatches), stub_isArchMatches);

    usleep(50 *1000);
    ConflictResult cr = m_packageManager->isConflictSatisfy("i386", conflicts());
    ASSERT_FALSE(cr.is_ok());
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

PackageList stub_availablePackages_nullptr()
{
    PackageList paclist;

    Package *package =  nullptr;
    paclist.append(package);
    return paclist;
}

QString deb_isInstalledConflict_packageName()
{
    return "";
}

QString deb_package_version()
{
    return "1";
}

TEST_F(ut_packagesManager_test, PackageManager_UT_isInstalledConflict_001)
{
    qDebug()<<"PackageManager_UT_isInstalledConflict_001";
    usleep(10 *1000);
    stub.set((QApt::Package *(QApt::Backend::*)(const QString & name) const)ADDR(Backend, package), package_package);
    stub.set(ADDR(Backend, availablePackages),stub_availablePackages_nullptr);

    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(Package, isInstalled), package_isInstalled);
    stub.set(ADDR(Package, conflicts), deb_conflicts);
    stub.set(ADDR(Package, name), package_name);
    stub.set(ADDR(DependencyInfo, packageName), deb_isInstalledConflict_packageName);
    stub.set(ADDR(DependencyInfo, packageVersion), deb_package_version);
    stub.set(ADDR(DependencyInfo, multiArchAnnotation), deb_package_version);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    qDebug()<<"PackageManager_UT_isInstalledConflict_001 打桩完成";

    ConflictResult cr = m_packageManager->isInstalledConflict("package name", "packageversion", "i386");
    ASSERT_TRUE(cr.is_ok());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_isInstalledConflict_002)
{
    stub.set((QApt::Package *(QApt::Backend::*)(const QString & name) const)ADDR(Backend, package), package_package);
    stub.set(ADDR(Backend, availablePackages),stub_availablePackages_nullptr);

    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(Package, isInstalled), package_isInstalled);
    stub.set(ADDR(Package, conflicts), deb_conflicts);
    stub.set(ADDR(Package, name), package_name);
    stub.set(ADDR(DependencyInfo, packageName), deb_isInstalledConflict_packageName);
    stub.set(ADDR(DependencyInfo, packageVersion), deb_package_version);
    stub.set(ADDR(DependencyInfo, multiArchAnnotation), package_installedVersion);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    ConflictResult cr = m_packageManager->isInstalledConflict("package name", "packageversion", "i386");
    ASSERT_TRUE(cr.is_ok());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_isConflictSatisfy_0001)
{
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
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

TEST_F(ut_packagesManager_test, PackageManager_UT_isConflictSatisfy_0002)
{
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    Package *pkg = nullptr;

    stub.set(ADDR(Package, name), package_name);
    stub.set(ADDR(Package, version), package_version);
    stub.set(ADDR(Package, architecture), package_architecture);
    stub.set(ADDR(Package, conflicts), package_conflicts);

    ConflictResult cr = m_packageManager->isConflictSatisfy("i386", pkg);
    ASSERT_FALSE(cr.is_ok());
}
const ConflictResult stub_isInstalledConflict(const QString &, const QString &,
                                         const QString &)
{
    return ConflictResult::err("1");
}

const ConflictResult stub_isInstalledConflict_ok(const QString &, const QString &,
                                         const QString &)
{
    return ConflictResult::ok("1");
}


TEST_F(ut_packagesManager_test, PackageManager_UT_isConflictSatisfy_0003)
{
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    Backend *bac = nullptr;
    pkgCache::PkgIterator packageIter;
    Package package(bac,packageIter);
    stub.set(ADDR(Package, name), package_name);
    stub.set(ADDR(Package, version), package_version);
    stub.set(ADDR(Package, architecture), package_architecture);
    stub.set(ADDR(Package, conflicts), package_conflicts);

    stub.set(ADDR(PackagesManager,isInstalledConflict),stub_isInstalledConflict);

    ConflictResult cr = m_packageManager->isConflictSatisfy("i386", &package);
    ASSERT_FALSE(cr.is_ok());
}

const ConflictResult stub_isConflictSatisfy(const QString &, const QList<QApt::DependencyItem> &)
{
    return ConflictResult::ok("1");
}

const ConflictResult stub_isConflictSatisfy_error(const QString &, const QList<QApt::DependencyItem> &)
{
    return ConflictResult::err("1");
}

TEST_F(ut_packagesManager_test, PackageManager_UT_isConflictSatisfy_0004)
{
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    Backend *bac = nullptr;
    pkgCache::PkgIterator packageIter;
    Package package(bac,packageIter);
    stub.set(ADDR(Package, name), package_name);
    stub.set(ADDR(Package, version), package_version);
    stub.set(ADDR(Package, architecture), package_architecture);
    stub.set(ADDR(Package, conflicts), package_conflicts);

    stub.set(ADDR(PackagesManager,isInstalledConflict),stub_isInstalledConflict_ok);
    stub.set((const ConflictResult (PackagesManager::*)(const QString &, const QList<QApt::DependencyItem> &))ADDR(PackagesManager, isConflictSatisfy), stub_isConflictSatisfy);

    ConflictResult cr = m_packageManager->isConflictSatisfy("i386", &package);
    ASSERT_TRUE(cr.is_ok());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_packageInstallStatus)
{
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);

    m_packageManager->appendPackage({"/1"});
    ASSERT_EQ(m_packageManager->packageInstallStatus(0), 0);
}

TEST_F(ut_packagesManager_test, PackageManager_UT_packageInstallStatus_01)
{
    stub.set(ADDR(PackagesManager, packageWithArch), stub_avaialbe_packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);

    m_packageManager->m_packageMd5.insert(0, "deb");

    m_packageManager->appendPackage({"/1"});
    ASSERT_EQ(m_packageManager->packageInstallStatus(0), 0);
    ASSERT_EQ(DebListModel::NotInstalled, m_packageManager->m_packageInstallStatus[m_packageManager->m_packageMd5.value(0)]);
}

TEST_F(ut_packagesManager_test, PackageManager_UT_packageInstallStatus_02)
{
    stub.set(ADDR(PackagesManager, packageWithArch), stub_avaialbe_packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    stub.set(ADDR(Package, installedVersion), deb_package_version);
    stub.set(ADDR(Package, compareVersion), package_compareVersion1);
    m_packageManager->m_packageMd5.insert(0, "deb");
    m_packageManager->appendPackage({"/1"});
    ASSERT_EQ(m_packageManager->packageInstallStatus(0), 3);
    ASSERT_EQ(DebListModel::InstalledLaterVersion, m_packageManager->m_packageInstallStatus[m_packageManager->m_packageMd5.value(0)]);
}

TEST_F(ut_packagesManager_test, PackageManager_UT_packageInstallStatus_03)
{
    stub.set(ADDR(PackagesManager, packageWithArch), stub_avaialbe_packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    stub.set(ADDR(Package, installedVersion), deb_package_version);
    stub.set(ADDR(Package, compareVersion), package_compareVersion2);
    m_packageManager->m_packageMd5.insert(0, "deb");
    m_packageManager->appendPackage({"/1"});
    ASSERT_EQ(m_packageManager->packageInstallStatus(0), 2);
    ASSERT_EQ(DebListModel::InstalledEarlierVersion, m_packageManager->m_packageInstallStatus[m_packageManager->m_packageMd5.value(0)]);
}

TEST_F(ut_packagesManager_test, PackageManager_UT_packageInstallStatus_04)
{
    stub.set(ADDR(PackagesManager, packageWithArch), stub_avaialbe_packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    stub.set(ADDR(Package, installedVersion), deb_package_version);
    stub.set(ADDR(Package, compareVersion), package_compareVersion2);

    m_packageManager->m_preparedPackages.append("0");
    m_packageManager->m_packageMd5.insert(0,"0");
    m_packageManager->m_packageInstallStatus.insert("0",1);

    ASSERT_EQ(m_packageManager->packageInstallStatus(0), 1);
    ASSERT_EQ(DebListModel::InstalledSameVersion, m_packageManager->m_packageInstallStatus[m_packageManager->m_packageMd5.value(0)]);
}

bool ut_isArchError_false(int index)
{
    Q_UNUSED(index);
    return false;
}

bool stub_isInstalled()
{
    return true;
}

QList<DependencyItem> deb_depends()
{
    DependencyInfo info("packageName", "0.0", RelationType::Equals, Depends);
    QList<DependencyInfo> dependencyItem;
    dependencyItem << info;
    QList<DependencyItem> conflicts;
    conflicts << dependencyItem;

    return conflicts;
}

TEST_F(ut_packagesManager_test, PackageManager_UT_DealDependResult)
{
    Stub stub;
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set((QApt::Package * (QApt::Backend::*)(const QString &name) const) ADDR(Backend, package), package_package);
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, isArchError), ut_isArchError_false);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    usleep(10 * 1000);
    m_packageManager->appendPackage({"/"});
    qInfo() << package_conflicts().size();

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(Package, isInstalled), stub_isInstalled);

    m_packageManager->m_dependInstallMark.append("deb");
    m_packageManager->m_packageMd5.append("deb");
    m_packageManager->m_packageMd5DependsStatus.insert("deb", PackageDependsStatus::ok());

    m_packageManager->m_dependInstallMark.append("test success");
    m_packageManager->slotDealDependResult(4, 0, "");
    EXPECT_EQ(DebListModel::DependsOk, m_packageManager->m_packageMd5DependsStatus[m_packageManager->m_dependInstallMark[0]].status);
    m_packageManager->slotDealDependResult(2, 0, "");
    EXPECT_EQ(DebListModel::DependsAuthCancel, m_packageManager->m_packageMd5DependsStatus[m_packageManager->m_dependInstallMark[0]].status);
    m_packageManager->slotDealDependResult(5, 0, "");
    EXPECT_EQ(DebListModel::DependsBreak, m_packageManager->m_packageMd5DependsStatus[m_packageManager->m_dependInstallMark[0]].status);
    m_packageManager->installWineDepends = true;
    m_packageManager->slotDealDependResult(5, 0, "");
    EXPECT_EQ(DebListModel::DependsBreak, m_packageManager->m_packageMd5DependsStatus[m_packageManager->m_dependInstallMark[0]].status);
    m_packageManager->m_preparedPackages.append({"1", "2"});
    m_packageManager->slotDealDependResult(5, 0, "");
    EXPECT_FALSE(m_packageManager->installWineDepends);
    EXPECT_EQ(DebListModel::DependsBreak, m_packageManager->m_packageMd5DependsStatus[m_packageManager->m_dependInstallMark[0]].status);
}

bool ut_isArchError(int index)
{
    Q_UNUSED(index);
    return true;
}

TEST_F(ut_packagesManager_test, PackageManager_UT_getPackageDependsStatus)
{
    stub.set((QApt::Package *(QApt::Backend::*)(const QString & name) const)ADDR(Backend, package), package_package);
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
    stub.set(ADDR(PackagesManager, isArchError), ut_isArchError_false);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    usleep(10 * 1000);
    m_packageManager->appendPackage({"/"});
    qInfo() << package_conflicts().size();


    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(Package, isInstalled), stub_isInstalled);

    m_packageManager->m_dependInstallMark.append("deb");
    m_packageManager->m_packageMd5.append("deb");
    PackageDependsStatus pd = m_packageManager->getPackageDependsStatus(0);

    ASSERT_TRUE(pd.isBreak());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_getPackageDependsStatus_01)
{
    stub.set((QApt::Package *(QApt::Backend::*)(const QString & name) const)ADDR(Backend, package), package_package);
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
    stub.set(ADDR(PackagesManager, isArchError), ut_isArchError);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    usleep(10 * 1000);
    m_packageManager->appendPackage({"/"});

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(Package, isInstalled), stub_isInstalled);

    m_packageManager->m_dependInstallMark.append("deb");
    m_packageManager->m_packageMd5.append("deb1");

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
    stub.set((QApt::Package *(QApt::Backend::*)(const QString & name) const)ADDR(Backend, package), package_package);
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
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
    stub.set(ADDR(Package, isInstalled), stub_isInstalled);
    PackageDependsStatus pd = m_packageManager->getPackageDependsStatus(0);

    ASSERT_EQ(pd.status, 2);
}
TEST_F(ut_packagesManager_test, PackageManager_UT_getPackageDependsStatus_03)
{
    stub.set((QApt::Package *(QApt::Backend::*)(const QString & name) const)ADDR(Backend, package), package_package);
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
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
    stub.set(ADDR(Package, isInstalled), stub_isInstalled);
    PackageDependsStatus pd = m_packageManager->getPackageDependsStatus(0);

//    ASSERT_EQ(pd.status, 6);
}

TEST_F(ut_packagesManager_test, PackageManager_UT_getPackageDependsStatus_04)
{

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(Package, isInstalled), stub_isInstalled);
    PackageDependsStatus pd = m_packageManager->getPackageDependsStatus(-1);

    ASSERT_EQ(pd.status, 2);
}

TEST_F(ut_packagesManager_test, PackageManager_UT_getPackageDependsStatus_05)
{
    stub.set((QApt::Package *(QApt::Backend::*)(const QString & name) const)ADDR(Backend, package), package_package);
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
    stub.set(ADDR(PackagesManager, isArchError), ut_isArchError_false);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    stub.set(ADDR(PackagesManager, isBlackApplication), stub_isBlackApplication_false);

    usleep(10 * 1000);
    m_packageManager->appendPackage({"/"});
    m_packageManager->m_packageMd5DependsStatus.clear();

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(Package, isInstalled), stub_isInstalled);


    stub.set((const ConflictResult (PackagesManager::*)(const QString &, const QList<QApt::DependencyItem> &))
             ADDR(PackagesManager, isConflictSatisfy), stub_isConflictSatisfy_error);

    PackageDependsStatus pd = m_packageManager->getPackageDependsStatus(0);

    ASSERT_EQ(pd.status, 2);
}

const PackageDependsStatus stub_checkDependsPackageStatus(QSet<QString> &, const QString &,
                                                     const QList<QApt::DependencyItem> &)
{
    return PackageDependsStatus::_break("1");
}


const PackageDependsStatus stub_checkDependsPackageStatus_ok(QSet<QString> &, const QString &,
                                                     const QList<QApt::DependencyItem> &)
{
    return PackageDependsStatus::ok();
}

const PackageDependsStatus stub_checkDependsPackageStatus_DI(QSet<QString> &,
                                                                      const QString &,
                                                                      const DependencyInfo &)
{
    return PackageDependsStatus::_break("1");
}

TEST_F(ut_packagesManager_test, PackageManager_UT_getPackageDependsStatus_06)
{
    stub.set(ADDR(DependencyInfo, packageName), deb_package_name);
    stub.set(ADDR(DependencyInfo, packageVersion), deb_package_name);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    stub.set(ADDR(DebFile, depends), deb_depends);

    stub.set(ADDR(Package, name), package_name);
    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(Package, isInstalled), stub_isInstalled);
    stub.set(ADDR(Package, architecture), package_architecture);
    stub.set(ADDR(Package, version), package_architecture);
    stub.set(ADDR(Package, multiArchType), ut_packagesManager_multiArchType);
    stub.set(ADDR(Package, conflicts), package_conflicts);
    stub.set(ADDR(Package, depends), deb_conflicts_null);

    stub.set((QApt::Package *(QApt::Backend::*)(const QString & name) const)ADDR(Backend, package), package_package);


    stub.set(ADDR(PackagesManager, packageWithArch), stub_avaialbe_packageWithArch);
    stub.set(ADDR(PackagesManager, isArchError), ut_isArchError_false);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    stub.set(ADDR(PackagesManager, isBlackApplication), stub_isBlackApplication_false);

    stub.set((const ConflictResult (PackagesManager::*)(const QString &, const QList<QApt::DependencyItem> &))
             ADDR(PackagesManager, isConflictSatisfy), stub_isConflictSatisfy);

    stub.set((const PackageDependsStatus (PackagesManager::*)(QSet<QString> &, const QString &,
                                                              const QList<QApt::DependencyItem> &))
             ADDR(PackagesManager, checkDependsPackageStatus), stub_checkDependsPackageStatus);

    stub.set((const PackageDependsStatus (PackagesManager::*)(QSet<QString> &,
                                                              const QString &,
                                                              const DependencyInfo &))
             ADDR(PackagesManager, checkDependsPackageStatus), stub_checkDependsPackageStatus_DI);


    m_packageManager->appendPackage({"/"});
    m_packageManager->m_packageMd5DependsStatus.clear();

    PackageDependsStatus pd = m_packageManager->getPackageDependsStatus(0);

    ASSERT_EQ(pd.status, 2);
}

void stub_run()
{

}
bool stub_isRunning()
{
    return false;
}

void stub_setDependsList(QStringList , int )
{

}

void stub_setBrokenDepend(QString)
{

}

TEST_F(ut_packagesManager_test, PackageManager_UT_getPackageDependsStatus_07)
{
    stub.set(ADDR(DependencyInfo, packageName), deb_package_name1);
    stub.set(ADDR(DependencyInfo, packageVersion), deb_package_name);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    stub.set(ADDR(DebFile, depends), deb_depends);

    stub.set(ADDR(Package, name), package_name);
    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(Package, isInstalled), stub_isInstalled);
    stub.set(ADDR(Package, architecture), package_architecture);
    stub.set((QApt::Package *(QApt::Backend::*)(const QString & name) const)ADDR(Backend, package), package_package);
    stub.set(ADDR(Package, multiArchType), ut_packagesManager_multiArchType);
    stub.set(ADDR(Package, conflicts), package_conflicts);

    stub.set(ADDR(PackagesManager, packageWithArch), stub_avaialbe_packageWithArch);
    stub.set(ADDR(PackagesManager, isArchError), ut_isArchError_false);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);
    stub.set(ADDR(PackagesManager, isBlackApplication), stub_isBlackApplication_false);

    stub.set((const ConflictResult (PackagesManager::*)(const QString &, const QList<QApt::DependencyItem> &))
             ADDR(PackagesManager, isConflictSatisfy), stub_isConflictSatisfy);

    stub.set((const PackageDependsStatus (PackagesManager::*)(QSet<QString> &,
                                                              const QString &,
                                                              const DependencyInfo &))
    ADDR(PackagesManager, checkDependsPackageStatus), stub_checkDependsPackageStatus_DI);

    stub.set(ADDR(QThread, isRunning), stub_isRunning);

//    typedef void (*fptr)(DealDependThread*);
//    fptr run    = (fptr)((void(DealDependThread::*)(   ))&DealDependThread::run);

    stub.set(ADDR(DealDependThread, setDependsList), stub_setDependsList);
    stub.set(ADDR(DealDependThread, setBrokenDepend), stub_setBrokenDepend);

    m_packageManager->m_preparedPackages.insert(0,"0");
    m_packageManager->m_packageMd5.insert(0,"0");
    m_packageManager->m_dependInstallMark.insert(0,"0");
    m_packageManager->m_packageMd5DependsStatus.clear();

    PackageDependsStatus pd = m_packageManager->getPackageDependsStatus(0);

    ASSERT_EQ(pd.status, 2);
}

TEST_F(ut_packagesManager_test, PackageManager_UT_packageInstalledVersion)
{
    stub.set((QApt::Package *(QApt::Backend::*)(const QString & name) const)ADDR(Backend, package), package_package);

    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), stub_avaialbe_packageWithArch);
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
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
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
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
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
    rbl << "package1" << "package2" ;
    return rbl;
}

QStringList package_recommandList()
{
    QStringList rbl;
    rbl <<  "package3";
    return rbl;
}

QStringList package_suggestList()
{
    QStringList rbl;
    rbl << "package4";
    return rbl;
}


TEST_F(ut_packagesManager_test, PackageManager_UT_packageReverseDependsList)
{
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
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

TEST_F(ut_packagesManager_test, PackageManager_UT_packageReverseDependsList_1)
{
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(Package, requiredByList), package_requiredByList);
    stub.set(ADDR(Package, recommendsList), package_requiredByList);
    stub.set(ADDR(Package, suggestsList), package_requiredByList);
    stub.set(ADDR(Package, isInstalled), stub_isInstalled);


    stub.set(ADDR(PackagesManager, packageWithArch), stub_avaialbe_packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    usleep(10 * 1000);
    m_packageManager->appendPackage({"/"});

//    stub.set(ADDR(Package, compareVersion),package_compareVersion);

    QStringList ads = m_packageManager->packageReverseDependsList("", "");

    ASSERT_FALSE(ads.isEmpty());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_packageReverseDependsList_2)
{
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(Package, requiredByList), package_requiredByList);
    stub.set(ADDR(Package, recommendsList), package_requiredByList);
    stub.set(ADDR(Package, suggestsList), package_requiredByList);
    stub.set(ADDR(Package, isInstalled), stub_isRunning);


    stub.set(ADDR(PackagesManager, packageWithArch), stub_avaialbe_packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    usleep(10 * 1000);
    m_packageManager->appendPackage({"/"});

//    stub.set(ADDR(Package, compareVersion),package_compareVersion);

    QStringList ads = m_packageManager->packageReverseDependsList("package1", "");

    ASSERT_TRUE(ads.isEmpty());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_packageReverseDependsList_3)
{
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(Package, requiredByList), package_requiredByList);
    stub.set(ADDR(Package, recommendsList), package_requiredByList);
    stub.set(ADDR(Package, suggestsList), package_requiredByList);
    stub.set(ADDR(Package, isInstalled), stub_isInstalled);


    stub.set(ADDR(PackagesManager, packageWithArch), stub_avaialbe_packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    usleep(10 * 1000);
    m_packageManager->appendPackage({"/"});

//    stub.set(ADDR(Package, compareVersion),package_compareVersion);

    QStringList ads = m_packageManager->packageReverseDependsList("package2", "");

    ASSERT_TRUE(ads.isEmpty());
}

TEST_F(ut_packagesManager_test, PackageManager_UT_packageReverseDependsList_4)
{
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(Package, requiredByList), package_requiredByList);
    stub.set(ADDR(Package, recommendsList), package_requiredByList);
    stub.set(ADDR(Package, suggestsList), package_suggestList);
    stub.set(ADDR(Package, isInstalled), stub_isInstalled);


    stub.set(ADDR(PackagesManager, packageWithArch), stub_avaialbe_packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    usleep(10 * 1000);
    m_packageManager->appendPackage({"/"});

//    stub.set(ADDR(Package, compareVersion),package_compareVersion);

    QStringList ads = m_packageManager->packageReverseDependsList("package4", "");

    ASSERT_TRUE(ads.isEmpty());
}


TEST_F(ut_packagesManager_test, PackageManager_UT_packageReverseDependsList_5)
{
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(Package, requiredByList), package_requiredByList);
    stub.set(ADDR(Package, recommendsList), package_recommandList);
    stub.set(ADDR(Package, suggestsList), package_suggestList);
    stub.set(ADDR(Package, isInstalled), stub_isInstalled);


    stub.set(ADDR(PackagesManager, packageWithArch), stub_avaialbe_packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(PackagesManager, dealPackagePath), stub_dealPackagePath);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), stub_dealInvalidPackage);

    usleep(10 * 1000);
    m_packageManager->appendPackage({"/"});

//    stub.set(ADDR(Package, compareVersion),package_compareVersion);

    QStringList ads = m_packageManager->packageReverseDependsList("package6", "");

    ASSERT_FALSE(ads.isEmpty());
}

bool backend_reloadCache()
{
    return true;
}

TEST_F(ut_packagesManager_test, PackageManager_UT_reset)
{
    stub.set(ADDR(Backend, reloadCache), backend_reloadCache);
    stub.set(ADDR(DebFile, depends), deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
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
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
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
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
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
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
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
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
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
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
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
    stub.set((QApt::Package *(QApt::Backend::*)(const QString & name) const)ADDR(Backend, package), package_package);

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
TEST_F(ut_packagesManager_test, PackageManager_UT_isArchMatches_03)
{
    ASSERT_FALSE(m_packageManager->isArchMatches(":amd64", "", 0));
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

TEST_F(ut_packagesManager_test, PackageManager_UT_resolvMultiArchAnnotation_03)
{
    ASSERT_STREQ(m_packageManager->resolvMultiArchAnnotation(":i386", "", MultiArchForeign).toLocal8Bit(), "");
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
    stub.set(ADDR(PackagesManager, packageWithArch), stub_packageWithArch);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    usleep(10 * 1000);
    QSet<QString> set;
    m_packageManager->checkDependsPackageStatus(set, "", conflicts());
}

QString ut_availableversion()
{
    return "1.0";
}

QString ut_version()
{
    return "1.1";
}

TEST_F(ut_packagesManager_test, PackageManager_UT_checkDependsPackageStatus01)
{
    usleep(10 * 1000);
    QSet<QString> set;
    stub.set(ADDR(PackagesManager, packageWithArch), stub_avaialbe_packageWithArch);

    stub.set(ADDR(Package, installedVersion), package_installedVersion1);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(Package, name), package_name);
    stub.set(ADDR(Package, architecture), package_architecture);
    stub.set(ADDR(Package, conflicts), package_conflicts);
    stub.set(ADDR(Package, availableVersion), ut_availableversion);
    stub.set(ADDR(Package, depends), deb_conflicts_null);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    m_packageManager->checkDependsPackageStatus(set, "", conflicts().at(0).at(0));
    stub.set(ADDR(PackagesManager, dependencyVersionMatch), ut_packagesManager_dependencyVersionMatch);
    m_packageManager->checkDependsPackageStatus(set, "", conflicts().at(0).at(0));
}

TEST_F(ut_packagesManager_test, PackageManager_UT_checkDependsPackageStatus02)
{
    usleep(10 * 1000);
    QSet<QString> set;
    Stub stub;
    stub.set(ADDR(PackagesManager, packageWithArch), stub_avaialbe_packageWithArch);

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);
    stub.set(ADDR(Package, name), package_name);
    stub.set(ADDR(Package, version), ut_version);
    stub.set(ADDR(Package, architecture), package_architecture);
    stub.set(ADDR(Package, conflicts), package_conflicts);
    stub.set(ADDR(Package, availableVersion), ut_availableversion);
    stub.set(ADDR(Package, depends), deb_conflicts_null);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dependencyVersionMatch), ut_packagesManager_dependencyVersionMatch);
    m_packageManager->checkDependsPackageStatus(set, "", conflicts().at(0).at(0));
    Stub stub1;
    set.insert("");
    stub1.set(ADDR(PackagesManager, dependencyVersionMatch), ut_packagesManager_dependencyVersionMatch1);
    m_packageManager->checkDependsPackageStatus(set, "", conflicts().at(0).at(0));
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

TEST_F(ut_packagesManager_test, PackageManager_UT_packageCandidateChoose)
{
    QSet<QString> choosed_set;
    QString debArch = "";
    DependencyItem cadicateList;

    DependencyInfo info("depends","0.0.1",QApt::RelationType::GreaterThan, QApt::Depends);
    cadicateList.append(info);
    stub.set(ADDR(DependencyInfo, multiArchAnnotation), package_installedVersion);
    stub.set(ADDR(DependencyInfo, packageName), deb_package_name);

    stub.set(ADDR(PackagesManager, packageWithArch), stub_avaialbe_packageWithArch);

    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), ut_packagesManager_compareVersion);
    stub.set(ADDR(Package, name), package_name);
    stub.set(ADDR(Package, architecture), package_architecture);
    stub.set(ADDR(Package, conflicts), package_conflicts);
    stub.set(ADDR(Package, depends), deb_conflicts_null);
    stub.set((QApt::Package *(QApt::Backend::*)(const QString & name) const)ADDR(Backend, package), packagesManager_package);

    stub.set((const ConflictResult (PackagesManager::*)(const QString &, const QList<QApt::DependencyItem> &))ADDR(PackagesManager, isConflictSatisfy), stub_isConflictSatisfy);

    stub.set((const PackageDependsStatus (PackagesManager::*)(QSet<QString> &, const QString &,
                                                              const QList<QApt::DependencyItem> &))
             ADDR(PackagesManager, checkDependsPackageStatus), stub_checkDependsPackageStatus);

    m_packageManager->packageCandidateChoose(choosed_set,debArch,cadicateList);
}

TEST_F(ut_packagesManager_test, PackageManager_UT_packageCandidateChoose_1)
{
    QSet<QString> choosed_set;
    QString debArch = "";
    DependencyItem cadicateList;

    DependencyInfo info("depends","0.0.1",QApt::RelationType::GreaterThan, QApt::Depends);
    cadicateList.append(info);
    stub.set(ADDR(DependencyInfo, multiArchAnnotation), package_installedVersion);
    stub.set(ADDR(DependencyInfo, packageName), deb_package_name);

    stub.set(ADDR(PackagesManager, packageWithArch), stub_avaialbe_packageWithArch);

    stub.set(ADDR(Package, installedVersion), package_version);
    stub.set(ADDR(Package, compareVersion), package_compareVersion1);
    stub.set(ADDR(Package, name), package_name);
    stub.set(ADDR(Package, architecture), package_architecture);
    stub.set(ADDR(Package, conflicts), package_conflicts);
    stub.set(ADDR(Package, depends), deb_conflicts_null);
    stub.set((QApt::Package *(QApt::Backend::*)(const QString & name) const)ADDR(Backend, package), packagesManager_package);

    stub.set((const ConflictResult (PackagesManager::*)(const QString &, const QList<QApt::DependencyItem> &))ADDR(PackagesManager, isConflictSatisfy), stub_isConflictSatisfy);

    stub.set((const PackageDependsStatus (PackagesManager::*)(QSet<QString> &, const QString &,
                                                              const QList<QApt::DependencyItem> &))
             ADDR(PackagesManager, checkDependsPackageStatus), stub_checkDependsPackageStatus);

    m_packageManager->packageCandidateChoose(choosed_set,debArch,cadicateList);
}

TEST_F(ut_packagesManager_test, PackageManager_UT_packageCandidateChoose_2)
{
    QSet<QString> choosed_set;
    QString debArch = "";
    DependencyItem cadicateList;

    DependencyInfo info("depends","0.0.1",QApt::RelationType::GreaterThan, QApt::Depends);
    cadicateList.append(info);
    stub.set(ADDR(DependencyInfo, multiArchAnnotation), package_installedVersion);
    stub.set(ADDR(DependencyInfo, packageName), deb_package_name);

    stub.set(ADDR(PackagesManager, packageWithArch), stub_avaialbe_packageWithArch);

    stub.set(ADDR(Package, installedVersion), package_version);
    stub.set(ADDR(Package, compareVersion), package_compareVersion1);
    stub.set(ADDR(Package, name), package_name);
    stub.set(ADDR(Package, architecture), package_architecture);
    stub.set(ADDR(Package, conflicts), package_conflicts);
    stub.set(ADDR(Package, depends), deb_conflicts_null);
    stub.set((QApt::Package *(QApt::Backend::*)(const QString & name) const)ADDR(Backend, package), package_package);

    stub.set((const ConflictResult (PackagesManager::*)(const QString &, const QList<QApt::DependencyItem> &))ADDR(PackagesManager, isConflictSatisfy), stub_isConflictSatisfy);

    stub.set((const PackageDependsStatus (PackagesManager::*)(QSet<QString> &, const QString &,
                                                              const QList<QApt::DependencyItem> &))
             ADDR(PackagesManager, checkDependsPackageStatus), stub_checkDependsPackageStatus_ok);

    m_packageManager->packageCandidateChoose(choosed_set,debArch,cadicateList);
}

TEST_F(ut_packagesManager_test, PackageManager_UT_packageCandidateChoose_3)
{
    QSet<QString> choosed_set;
    QString debArch = "";
    DependencyItem cadicateList;

    DependencyInfo info("depends","0.0.1",QApt::RelationType::GreaterThan, QApt::Depends);
    cadicateList.append(info);
    stub.set(ADDR(DependencyInfo, multiArchAnnotation), package_installedVersion);
    stub.set(ADDR(DependencyInfo, packageName), deb_package_name);

    stub.set(ADDR(PackagesManager, packageWithArch), stub_avaialbe_packageWithArch);

    stub.set(ADDR(Package, installedVersion), package_version);
    stub.set(ADDR(Package, compareVersion), package_compareVersion2);
    stub.set(ADDR(Package, name), package_name);
    stub.set(ADDR(Package, architecture), package_architecture);
    stub.set(ADDR(Package, conflicts), package_conflicts);
    stub.set(ADDR(Package, depends), deb_conflicts_null);
    stub.set((QApt::Package *(QApt::Backend::*)(const QString & name) const)ADDR(Backend, package), package_package);

    stub.set((const ConflictResult (PackagesManager::*)(const QString &, const QList<QApt::DependencyItem> &))ADDR(PackagesManager, isConflictSatisfy), stub_isConflictSatisfy_error);

    stub.set((const PackageDependsStatus (PackagesManager::*)(QSet<QString> &, const QString &,
                                                              const QList<QApt::DependencyItem> &))
             ADDR(PackagesManager, checkDependsPackageStatus), stub_checkDependsPackageStatus_ok);

    m_packageManager->packageCandidateChoose(choosed_set,debArch,cadicateList);
}

TEST_F(ut_packagesManager_test, PackageManager_UT_packageCandidateChoose_4)
{
    QSet<QString> choosed_set;
    QString debArch = "";
    DependencyItem cadicateList;

    DependencyInfo info("depends","0.0.1",QApt::RelationType::GreaterThan, QApt::Depends);
    cadicateList.append(info);
    stub.set(ADDR(DependencyInfo, multiArchAnnotation), package_installedVersion);
    stub.set(ADDR(DependencyInfo, packageName), deb_package_name);

    stub.set(ADDR(PackagesManager, packageWithArch), stub_avaialbe_packageWithArch);

    stub.set(ADDR(Package, installedVersion), package_version);
    stub.set(ADDR(Package, compareVersion), package_compareVersion1);
    stub.set(ADDR(Package, name), package_name);
    stub.set(ADDR(Package, architecture), package_architecture);
    stub.set(ADDR(Package, conflicts), package_conflicts);
    stub.set(ADDR(Package, depends), deb_conflicts_null);
    stub.set((QApt::Package *(QApt::Backend::*)(const QString & name) const)ADDR(Backend, package), package_package);

    stub.set((const ConflictResult (PackagesManager::*)(const QString &, const QList<QApt::DependencyItem> &))ADDR(PackagesManager, isConflictSatisfy), stub_isConflictSatisfy_error);

    stub.set((const PackageDependsStatus (PackagesManager::*)(QSet<QString> &, const QString &,
                                                              const QList<QApt::DependencyItem> &))
             ADDR(PackagesManager, checkDependsPackageStatus), stub_checkDependsPackageStatus_ok);

    m_packageManager->packageCandidateChoose(choosed_set,debArch,cadicateList);
}


