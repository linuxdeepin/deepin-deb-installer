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

#include "../deb_installer/manager/PackageDependsStatus.h"
#include "../deb_installer/manager/packagesmanager.h"

#include <stub.h>
#include <QFuture>
#include <QDir>
#include <fstream>
#include <QFileInfo>

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

Backend *ut_init_backend()
{
    return nullptr;
}


bool stub_is_open()
{
    qDebug()<<"stb——is_open";
    return true;
}

void stub_open(const std::string& __s,std::ios_base::openmode __mode)
{
    Q_UNUSED(__s);
    Q_UNUSED(__mode);
    qDebug()<<"stb——open";
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

bool stub_checkLocalFile_true(QString fileName)
{
    Q_UNUSED(fileName);
    return true;
}

bool stub_checkLocalFile_false(QString fileName)
{
    Q_UNUSED(fileName);
    return false;
}

QString stub_dealPackagePath(QString packagePath)
{
    Q_UNUSED(packagePath);
    return "";
}


TEST(PackageManager_UT, PackageManager_UT_isBackendReady)
{
    Stub stub;

//    stub.set(init_backend, ut_init_backend);
    stub.set(ADDR(Backend, init), backend_init);

    PackagesManager *p = new PackagesManager();
    usleep(50 * 1000);
    ASSERT_TRUE(p->isBackendReady());
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_dealPackagePath_SymbolicLink)
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
    usleep(300);

//    stub.set(ADDR(PackagesManager, dealPackagePath),)

    stub.set(ADDR(PackagesManager, SymbolicLink),stub_SymbolicLink);

    PackagesManager *p = new PackagesManager();

    usleep(10*1000);
   ASSERT_STREQ("", p->dealPackagePath("/ ").toUtf8());
}


TEST(PackageManager_UT, PackageManager_UT_dealPackagePath_AbsolutePath)
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
    usleep(300);


    stub.set(ADDR(QFileInfo, absoluteFilePath),stub_absoluteFilePath);

    stub.set(ADDR(PackagesManager, SymbolicLink),stub_SymbolicLink);


    PackagesManager *p = new PackagesManager();

    usleep(10*1000);
    ASSERT_STREQ("", p->dealPackagePath(" ").toUtf8());

}

TEST(PackageManager_UT, PackageManager_UT_checkLocalFile_true)
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


    stub.set((void (std::fstream::*)(const std::string& __s,std::ios_base::openmode __mode))ADDR(std::fstream, open), stub_open);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, is_open), stub_is_open);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, close), stub_close);

    stub.set(ADDR(QFileInfo,permission), stub_permission_true);

    PackagesManager *p = new PackagesManager();
    usleep(10*1000);
    p->checkLocalFile("/1");

    delete p;

}

TEST(PackageManager_UT, PackageManager_UT_checkLocalFile_false)
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
    stub.set((void (std::fstream::*)(const std::string& __s,std::ios_base::openmode __mode))ADDR(std::fstream, open), stub_open);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, is_open), stub_is_open);
    stub.set((bool (std::fstream::*)())ADDR(std::fstream, close), stub_close);

    stub.set(ADDR(QFileInfo,permission), stub_permission_false);

    PackagesManager *p = new PackagesManager();
    usleep(10*1000);

    p->checkLocalFile("/1");

    delete p;;

}

TEST(PackageManager_UT, PackageManager_UT_dealInvalidPackage_true)
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
    stub.set(ADDR(PackagesManager,checkLocalFile), stub_checkLocalFile_true);

    PackagesManager *p = new PackagesManager();
    usleep(10*1000);

    p->dealInvalidPackage("/1");

    delete p;;

}

TEST(PackageManager_UT, PackageManager_UT_dealInvalidPackage_false)
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
    stub.set(ADDR(PackagesManager,checkLocalFile), stub_checkLocalFile_false);

    PackagesManager *p = new PackagesManager();
    usleep(10*1000);

    p->dealInvalidPackage("/1");

    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_appendPackage)
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


    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);


    PackagesManager *p = new PackagesManager();

    usleep(10 * 1000);
    p->appendPackage({"/1"});

    ASSERT_FALSE(p->m_packageMd5.isEmpty());
    p->appendPackage(QStringList() << "/1"
                                   << "/2");

    usleep(10 * 1000);
    delete p;
}

bool deb_isValid_false()
{
    return true;
}

TEST(PackageManager_UT, PackageManager_UT_appendPackage_invalid)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_all);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(DebFile, isValid), deb_isValid_false);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);

    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    usleep(300);

    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);


    PackagesManager *p = new PackagesManager();

    usleep(10 * 1000);
    p->appendPackage({"/1"});

    ASSERT_FALSE(p->m_packageMd5.isEmpty());
    p->appendPackage(QStringList() << "/1"
                                   << "/2");

    usleep(10 * 1000);
    delete p;
}


bool deb_open_failed()
{
    return false;
}

TEST(PackageManager_UT, PackageManager_UT_appendPackage_openFailed)
{
    Stub stub;
    stub.set(ADDR(DebFile, architecture), deb_arch_all);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(DebFile, isValid), deb_isValid_false);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);

    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    usleep(300);

    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);


    PackagesManager *p = new PackagesManager();

    usleep(10 * 1000);
    p->appendPackage({"/1"});

    ASSERT_FALSE(p->m_packageMd5.isEmpty());
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_isArchError)
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

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->appendPackage({"/1"});
    ASSERT_FALSE(p->isArchError(0));
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_isArchError_1)
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

    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);

    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->appendPackage({"/1"});
    ASSERT_FALSE(p->isArchError(0));
    delete p;
}

Package *package_package(const QString &name)
{
    Q_UNUSED(name);
    return nullptr;
}

TEST(PackageManager_UT, PackageManager_UT_packageConflictStat)
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

    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->appendPackage({"/1"});

    ConflictResult cr = p->packageConflictStat(0);
    ASSERT_TRUE(cr.is_ok());
    delete p;

}

TEST(PackageManager_UT, PackageManager_UT_isConflictSatisfy)
{
    Stub stub;
    stub.set((Package * (Backend::*)(const QString &) const)ADDR(Backend, package), package_package);

    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(DebFile, isValid), deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);

    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);

    PackagesManager *p = new PackagesManager();

    p->appendPackage({"/1"});
    usleep(10 * 1000);
    ConflictResult cr = p->isConflictSatisfy("i386", conflicts());
    ASSERT_TRUE(cr.is_ok());
    delete p;
}


TEST(PackageManager_UT, PackageManager_UT_isConflictSatisfy_01)
{
    Stub stub;
    stub.set((Package * (Backend::*)(const QString &) const)ADDR(Backend, package), package_package);

    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(DebFile, isValid), deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);

    PackagesManager *p = new PackagesManager();

    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    p->appendPackage({"/1"});
    usleep(10 * 1000);
    ConflictResult cr = p->isConflictSatisfy("i386", conflicts());
    ASSERT_TRUE(cr.is_ok());
    delete p;
}

bool package_isInstalled()
{
    return true;
}

TEST(PackageManager_UT, PackageManager_UT_isInstalledConflict)
{
    Stub stub;
    stub.set((Package * (Backend::*)(const QString &) const)ADDR(Backend, package), package_package);

    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(Backend, availablePackages), backend_availablePackages);
    stub.set(ADDR(DebFile, isValid), deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);

    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(Package, isInstalled), package_isInstalled);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);

    PackagesManager *p = new PackagesManager();

    p->appendPackage({"/1"});
    usleep(10 * 1000);
    ConflictResult cr = p->isInstalledConflict("package name","packageversion","i386");
    ASSERT_TRUE(cr.is_ok());
    delete p;
}



TEST(PackageManager_UT, PackageManager_UT_isConflictSatisfy_0001)
{
    Stub stub;
    stub.set((Package * (Backend::*)(const QString &) const)ADDR(Backend, package), package_package);

    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    stub.set(ADDR(DebFile, isValid), deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->appendPackage({"/1"});
    qInfo() << package_conflicts().size();


    Package *package = nullptr;
    stub.set(ADDR(Package, name), package_name);
    stub.set(ADDR(Package, version), package_version);
    stub.set(ADDR(Package, architecture), package_architecture);
    stub.set(ADDR(Package, conflicts), package_conflicts);


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


Package *backend_package(const QString &name)
{
    return nullptr;
}

TEST(PackageManager_UT, PackageManager_UT_packageInstallStatus)
{
    Stub stub;

    stub.set(ADDR(DebFile, architecture), deb_arch_i386);
    stub.set(ADDR(Backend, architectures), backend_architectures);
    stub.set(ADDR(Backend, init), backend_init);
    //(int(A::*)(int))ADDR(A,foo)
    stub.set((Package * (Backend::*)(const QString &) const)ADDR(Backend, package), backend_package);
    stub.set(ADDR(DebFile, isValid), deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), deb_installSize);
    stub.set(ADDR(DebFile, packageName), deb_packageName);
    stub.set(ADDR(DebFile, longDescription), deb_longDescription);
    stub.set(ADDR(DebFile, version), deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), stub_getPackageDependsStatus);
    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->appendPackage({"/"});
    qInfo() << package_conflicts().size();


    stub.set(ADDR(Package, installedVersion), package_installedVersion);
    stub.set(ADDR(Package, compareVersion), package_compareVersion);

    ASSERT_EQ(p->packageInstallStatus(0), 0);
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_DealDependResult)
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
    stub.set(ADDR(PackagesManager, packageWithArch), packageWithArch);

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->m_dependInstallMark.append("test success");
    p->DealDependResult(4, 0, "");
    p->DealDependResult(2, 0, "");
    p->DealDependResult(5, 0, "");
    delete p;
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

bool isInstalled()
{
    return true;
}

bool ut_isArchError(int index)
{
    Q_UNUSED(index);
    return true;
}

PackageDependsStatus packageManager_getPackageDependsStatus(const int )
{
    return PackageDependsStatus::_break("package");
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

    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);

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

    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->appendPackage({"/"});
    qInfo() << package_conflicts().size();


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

    stub.set(ADDR(DebFile, conflicts), deb_conflicts);

    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);

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

    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);

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

    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);

    PackagesManager *p = new PackagesManager();

    p->appendPackage({"/"});
    usleep(10 * 1000);

    QMap<QString, QString> sp = p->specialPackage();

    ASSERT_EQ(sp.size(), 2);
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

    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);

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

    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->appendPackage({"/"});

    p->reset();

    ASSERT_TRUE(p->m_appendedPackagesMd5.isEmpty());
    delete p;
}

TEST(PackageManager_UT, PackageManager_UT_resetInstallStatus)
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

    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->appendPackage({"/"});

    p->resetInstallStatus();

    ASSERT_TRUE(p->m_packageMd5DependsStatus.isEmpty());
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

    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);

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

    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);

    PackagesManager *p = new PackagesManager();
    usleep(10 * 1000);
    p->appendPackage({"/"});

    p->removePackage(0);

    ASSERT_TRUE(p->m_appendedPackagesMd5.isEmpty());

    delete p;
}

bool ut_mktempDir()
{
    return false;
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

    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);

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
    stub.set(ADDR(PackagesManager, dealPackagePath),stub_dealPackagePath);

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
