/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:     cuizhen <cuizhen@uniontech.com>
* Maintainer:  cuizhen <cuizhen@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <gtest/gtest.h>
#define private public  // hack complier
#define protected public
#include "../deb_installer/manager/PackageDependsStatus.h"
#include "../deb_installer/manager/packagesmanager.h"

#undef private
#undef protected

#include <stub.h>
#include <QFuture>
#include <QDir>

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
    return nullptr;
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
    DependencyInfo info("packageName","0.0",RelationType::Equals,Depends);
    QList<DependencyInfo> dependencyItem;
    dependencyItem<<info;
    QList<DependencyItem> conflicts;
    conflicts<<dependencyItem;

    return conflicts;
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
    DependencyInfo info("packageName","0.0",RelationType::Equals,Conflicts);
    QList<DependencyInfo> dependencyItem;
    dependencyItem<<info;
    QList<DependencyItem> conflicts;
    conflicts<<dependencyItem;

    return conflicts;
}

QList<DependencyItem> package_conflicts()
{
    DependencyInfo info("packageName","0.0",RelationType::Equals,Conflicts);
    QList<DependencyInfo> dependencyItem;
    dependencyItem<<info;
    QList<DependencyItem> conflicts;
    conflicts<<dependencyItem;

    return conflicts;
}

Backend *ut_init_backend()
{
    return nullptr;
}
TEST(PackageManager_UT, PackageManager_UT_001)
{
    Stub stub;

//    stub.set(init_backend, ut_init_backend);
    stub.set(ADDR(Backend,init), backend_init);

    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    ASSERT_TRUE(p->isBackendReady());
    delete p;
}


TEST(PackageManager_UT, PackageManager_UT_002)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_all);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(DebFile, isValid),deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),deb_installSize);
    stub.set(ADDR(DebFile, packageName),deb_packageName);
    stub.set(ADDR(DebFile, longDescription),deb_longDescription);
    stub.set(ADDR(DebFile, version),deb_version);
    usleep(300);
    PackagesManager *p = new PackagesManager();

    usleep(10*1000);
    p->appendPackage({"/1"});

    ASSERT_FALSE(p->m_packageMd5.isEmpty());

    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_003)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_all);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(DebFile, isValid),deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),deb_installSize);
    stub.set(ADDR(DebFile, packageName),deb_packageName);
    stub.set(ADDR(DebFile, longDescription),deb_longDescription);
    stub.set(ADDR(DebFile, version),deb_version);

    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    p->appendPackage({"/1"});
    ASSERT_FALSE(p->isArchError(0));
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_004)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(DebFile, isValid),deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),deb_installSize);
    stub.set(ADDR(DebFile, packageName),deb_packageName);
    stub.set(ADDR(DebFile, longDescription),deb_longDescription);
    stub.set(ADDR(DebFile, version),deb_version);

    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    p->appendPackage({"/1"});
    ASSERT_FALSE(p->isArchError(0));
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_005)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(DebFile, isValid),deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),deb_installSize);
    stub.set(ADDR(DebFile, packageName),deb_packageName);
    stub.set(ADDR(DebFile, longDescription),deb_longDescription);
    stub.set(ADDR(DebFile, version),deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    p->appendPackage({"/1"});

    ConflictResult cr = p->packageConflictStat(0);
    ASSERT_TRUE(cr.is_ok());
    delete p;

}

TEST(PackageManager_UT, PackageManager_UT_006)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(DebFile, isValid),deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),deb_installSize);
    stub.set(ADDR(DebFile, packageName),deb_packageName);
    stub.set(ADDR(DebFile, longDescription),deb_longDescription);
    stub.set(ADDR(DebFile, version),deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    PackagesManager *p = new PackagesManager();

    p->appendPackage({"/1"});
    usleep(10*1000);
    ConflictResult cr = p->isConflictSatisfy("i386", conflicts());
    ASSERT_TRUE(cr.is_ok());
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_007)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(DebFile, isValid),deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),deb_installSize);
    stub.set(ADDR(DebFile, packageName),deb_packageName);
    stub.set(ADDR(DebFile, longDescription),deb_longDescription);
    stub.set(ADDR(DebFile, version),deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    p->appendPackage({"/1"});
    qInfo()<<package_conflicts().size();


    Package *package = nullptr;
    stub.set(ADDR(Package, name),package_name);
    stub.set(ADDR(Package, version),package_version);
    stub.set(ADDR(Package, architecture),package_architecture);
    stub.set(ADDR(Package, conflicts),package_conflicts);


    ConflictResult cr = p->isConflictSatisfy("i386", package);
    ASSERT_TRUE(cr.is_ok());

    delete p;
}

QString package_installedVersion()
{
    return "";
}

int package_compareVersion()
{
    return 0;
}

Package *package_package(const QString & name)
{
    return nullptr;
}

TEST(PackageManager_UT, PackageManager_UT_008)
{
    Stub stub;

    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package *(Backend::*)(const QString &) const)ADDR(Backend,package), package_package);
    stub.set(ADDR(DebFile, isValid),deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),deb_installSize);
    stub.set(ADDR(DebFile, packageName),deb_packageName);
    stub.set(ADDR(DebFile, longDescription),deb_longDescription);
    stub.set(ADDR(DebFile, version),deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    p->appendPackage({"/"});
    qInfo()<<package_conflicts().size();


    stub.set(ADDR(Package, installedVersion),package_installedVersion);
    stub.set(ADDR(Package, compareVersion),package_compareVersion);

    ASSERT_EQ(p->packageInstallStatus(0), 0);
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_009)
{
    Stub stub;

    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package *(Backend::*)(const QString &) const)ADDR(Backend,package), package_package);
    stub.set(ADDR(DebFile, isValid),deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),deb_installSize);
    stub.set(ADDR(DebFile, packageName),deb_packageName);
    stub.set(ADDR(DebFile, longDescription),deb_longDescription);
    stub.set(ADDR(DebFile, version),deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    p->DealDependResult(4,0,"");
    delete p;
}

QList<DependencyItem> deb_depends()
{
    DependencyInfo info("packageName","0.0",RelationType::Equals,Depends);
    QList<DependencyInfo> dependencyItem;
    dependencyItem<<info;
    QList<DependencyItem> conflicts;
    conflicts<<dependencyItem;

    return conflicts;
}

TEST(PackageManager_UT, PackageManager_UT_010)
{
    Stub stub;

    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package *(Backend::*)(const QString &) const)ADDR(Backend,package), package_package);
    stub.set(ADDR(DebFile, isValid),deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),deb_installSize);
    stub.set(ADDR(DebFile, packageName),deb_packageName);
    stub.set(ADDR(DebFile, longDescription),deb_longDescription);
    stub.set(ADDR(DebFile, version),deb_version);

    stub.set(ADDR(DebFile, depends),deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    p->appendPackage({"/"});
    qInfo()<<package_conflicts().size();


    stub.set(ADDR(Package, installedVersion),package_installedVersion);
    stub.set(ADDR(Package, compareVersion),package_compareVersion);

    PackageDependsStatus pd = p->getPackageDependsStatus(0);

    ASSERT_TRUE(pd.isBreak());

    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_011)
{
    Stub stub;

    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package *(Backend::*)(const QString &) const)ADDR(Backend,package), package_package);
    stub.set(ADDR(DebFile, isValid),deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),deb_installSize);
    stub.set(ADDR(DebFile, packageName),deb_packageName);
    stub.set(ADDR(DebFile, longDescription),deb_longDescription);
    stub.set(ADDR(DebFile, version),deb_version);

    stub.set(ADDR(DebFile, depends),deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    p->appendPackage({"/"});

    stub.set(ADDR(Package, installedVersion),package_installedVersion);
    stub.set(ADDR(Package, compareVersion),package_compareVersion);

    p->m_packageInstallStatus[p->m_packageMd5[0]] = 0;
    QString version = p->packageInstalledVersion(0);

    ASSERT_TRUE(version.isEmpty());
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_012)
{
    Stub stub;

    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package *(Backend::*)(const QString &) const)ADDR(Backend,package), package_package);
    stub.set(ADDR(DebFile, isValid),deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),deb_installSize);
    stub.set(ADDR(DebFile, packageName),deb_packageName);
    stub.set(ADDR(DebFile, longDescription),deb_longDescription);
    stub.set(ADDR(DebFile, version),deb_version);

    stub.set(ADDR(DebFile, depends),deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    p->appendPackage({"/"});

    stub.set(ADDR(Package, installedVersion),package_installedVersion);
    stub.set(ADDR(Package, compareVersion),package_compareVersion);

    QStringList ads = p->packageAvailableDepends(0);

    ASSERT_TRUE(ads.isEmpty());
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_013)
{
    Stub stub;

    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(DebFile, isValid),deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),deb_installSize);
    stub.set(ADDR(DebFile, packageName),deb_packageName);
    stub.set(ADDR(DebFile, longDescription),deb_longDescription);
    stub.set(ADDR(DebFile, version),deb_version);

    stub.set(ADDR(DebFile, depends),deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    PackagesManager *p = new PackagesManager();

    p->appendPackage({"/"});
    usleep(10*1000);

    QMap<QString, QString> sp = p->specialPackage();

    ASSERT_EQ(sp.size(), 2);
    delete p;
}

QStringList package_requiredByList()
{
    QStringList rbl;
    rbl<<"package1"<<"package2"<<"package3"<<"package4";
    return rbl;
}
TEST(PackageManager_UT, PackageManager_UT_014)
{
    Stub stub;

    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package *(Backend::*)(const QString &) const)ADDR(Backend,package), package_package);
    stub.set(ADDR(DebFile, isValid),deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),deb_installSize);
    stub.set(ADDR(DebFile, packageName),deb_packageName);
    stub.set(ADDR(DebFile, longDescription),deb_longDescription);
    stub.set(ADDR(DebFile, version),deb_version);

    stub.set(ADDR(DebFile, depends),deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    p->appendPackage({"/"});

    stub.set(ADDR(Package, installedVersion),package_installedVersion);
    stub.set(ADDR(Package, compareVersion),package_compareVersion);

    stub.set(ADDR(Package, requiredByList),package_requiredByList);
//    stub.set(ADDR(Package, compareVersion),package_compareVersion);

    QStringList ads = p->packageReverseDependsList("","");

    ASSERT_TRUE(ads.isEmpty());
    delete p;
}

bool backend_reloadCache()
{
    return true;
}

TEST(PackageManager_UT, PackageManager_UT_015)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(Backend, reloadCache), backend_reloadCache);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package *(Backend::*)(const QString &) const)ADDR(Backend,package), package_package);
    stub.set(ADDR(DebFile, isValid),deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),deb_installSize);
    stub.set(ADDR(DebFile, packageName),deb_packageName);
    stub.set(ADDR(DebFile, longDescription),deb_longDescription);
    stub.set(ADDR(DebFile, version),deb_version);

    stub.set(ADDR(DebFile, depends),deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    p->appendPackage({"/"});

    p->reset();

    ASSERT_TRUE(p->m_appendedPackagesMd5.isEmpty());
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_016)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(Backend, reloadCache), backend_reloadCache);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package *(Backend::*)(const QString &) const)ADDR(Backend,package), package_package);
    stub.set(ADDR(DebFile, isValid),deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),deb_installSize);
    stub.set(ADDR(DebFile, packageName),deb_packageName);
    stub.set(ADDR(DebFile, longDescription),deb_longDescription);
    stub.set(ADDR(DebFile, version),deb_version);

    stub.set(ADDR(DebFile, depends),deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    p->appendPackage({"/"});

    p->resetInstallStatus();

    ASSERT_TRUE(p->m_packageMd5DependsStatus.isEmpty());
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_017)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(Backend, reloadCache), backend_reloadCache);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package *(Backend::*)(const QString &) const)ADDR(Backend,package), package_package);
    stub.set(ADDR(DebFile, isValid),deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),deb_installSize);
    stub.set(ADDR(DebFile, packageName),deb_packageName);
    stub.set(ADDR(DebFile, longDescription),deb_longDescription);
    stub.set(ADDR(DebFile, version),deb_version);

    stub.set(ADDR(DebFile, depends),deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    p->appendPackage({"/"});

    p->resetPackageDependsStatus(0);

    ASSERT_TRUE(p->m_packageMd5DependsStatus.isEmpty());

    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_018)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(Backend, reloadCache), backend_reloadCache);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package *(Backend::*)(const QString &) const)ADDR(Backend,package), package_package);
    stub.set(ADDR(DebFile, isValid),deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),deb_installSize);
    stub.set(ADDR(DebFile, packageName),deb_packageName);
    stub.set(ADDR(DebFile, longDescription),deb_longDescription);
    stub.set(ADDR(DebFile, version),deb_version);

    stub.set(ADDR(DebFile, depends),deb_depends);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    p->appendPackage({"/"});

    p->removePackage(0);

    ASSERT_TRUE(p->m_appendedPackagesMd5.isEmpty());

    delete p;
}


TEST(PackageManager_UT, PackageManager_UT_019)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);

    stub.set(ADDR(Backend, init), backend_init);

    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    p->rmTempDir();
    ASSERT_STREQ(p->SymbolicLink("test","test").toLocal8Bit(), (QString("/tmp/LinkTemp/")+QString("test")).toLocal8Bit());
    delete p;
}
TEST(PackageManager_UT, PackageManager_UT_020)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);

    stub.set(ADDR(Backend, init), backend_init);

    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
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
TEST(PackageManager_UT, PackageManager_UT_021)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);

    stub.set(ADDR(Backend, init), backend_init);

    stub.set((bool(QDir::*)(const QString &)const)ADDR(QDir, mkdir),pm_mkdir);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((bool(QDir::*)()const)ADDR(QDir, exists),pm_exits);
    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    ASSERT_TRUE(p->mkTempDir());
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_022)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_i386);

    stub.set(ADDR(Backend, init), backend_init);

    PackagesManager *p = new PackagesManager();

    usleep(10*1000);
    ASSERT_STREQ(p->link("test","test1").toLocal8Bit(), (QString("test")).toLocal8Bit());
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_023)
{
    Stub stub;

    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package *(Backend::*)(const QString &) const)ADDR(Backend,package), package_package);
    stub.set(ADDR(DebFile, isValid),deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),deb_installSize);
    stub.set(ADDR(DebFile, packageName),deb_packageName);
    stub.set(ADDR(DebFile, longDescription),deb_longDescription);
    stub.set(ADDR(DebFile, version),deb_version);

    stub.set(ADDR(DebFile, depends),deb_depends);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);
    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    p->appendPackage({"/"});

    stub.set(ADDR(Package, installedVersion),package_installedVersion);
    stub.set(ADDR(Package, compareVersion),package_compareVersion);

    stub.set(ADDR(Package, requiredByList),package_requiredByList);
//    stub.set(ADDR(Package, compareVersion),package_compareVersion);

    ASSERT_FALSE(p->packageWithArch("package","arch"));

    p->deleteLater();
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_024)
{
    Stub stub;
    stub.set(ADDR(Backend, init), backend_init);
    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    p->appendPackageFinished();
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_025)
{
    ASSERT_TRUE(isArchMatches("all","",0));

}

TEST(PackageManager_UT, PackageManager_UT_026)
{
    ASSERT_STREQ(resolvMultiArchAnnotation("all","",InvalidMultiArchType).toLocal8Bit(), "");
}

TEST(PackageManager_UT, PackageManager_UT_027)
{
    ASSERT_TRUE(dependencyVersionMatch(0,Equals));
}

TEST(PackageManager_UT, PackageManager_UT_028)
{

    Stub stub;
    stub.set(ADDR(Backend, init), backend_init);
    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    ASSERT_TRUE(p->backend());

    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_029)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_all);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(DebFile, isValid),deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),deb_installSize);
    stub.set(ADDR(DebFile, packageName),deb_packageName);
    stub.set(ADDR(DebFile, longDescription),deb_longDescription);
    stub.set(ADDR(DebFile, version),deb_version);

    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    p->appendPackage({"/1"});

    ASSERT_STREQ(p->package(0).toLocal8Bit(), "/1");
    delete p;
}

