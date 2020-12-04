#include <gtest/gtest.h>

#define private public  // hack complier
#define protected public

#include "../deb_installer/model/deblistmodel.h"
#include "../deb_installer/manager/packagesmanager.h"
#include "../deb_installer/manager/PackageDependsStatus.h"
#undef private
#undef protected

#include <stub.h>


using namespace QApt;

bool model_backend_init()
{
    return true;
}

void model_checkSystemVersion()
{
}

bool model_reloadCache(){
    return true;
}

bool model_BackendReady()
{
    return true;
}

TEST(deblistmodel_Test, deblistmodel_UT_001)
{
    Stub stub;
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(Backend, reloadCache), model_backend_init);
    stub.set(ADDR(DebListModel, checkSystemVersion),model_checkSystemVersion);

    DebListModel *model = new DebListModel();
    usleep(5*1000);
    model->reset();
    ASSERT_EQ(model->m_workerStatus, DebListModel::WorkerPrepare);
}

TEST(deblistmodel_Test, deblistmodel_UT_002)
{
    Stub stub;
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(Backend, reloadCache), model_backend_init);
    stub.set(ADDR(DebListModel, checkSystemVersion),model_checkSystemVersion);

    DebListModel *model = new DebListModel();

    usleep(5*1000);
    model->m_packageOperateStatus[0] = 1;
    model->reset_filestatus();
    ASSERT_TRUE(model->m_packageOperateStatus.isEmpty());
}


TEST(deblistmodel_Test, deblistmodel_UT_003)
{
    Stub stub;
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(PackagesManager, isBackendReady), model_BackendReady);
    stub.set(ADDR(DebListModel, checkSystemVersion),model_checkSystemVersion);


    DebListModel *model = new DebListModel();

    usleep(5*1000);
    ASSERT_TRUE(model->isReady());
}

TEST(deblistmodel_Test, deblistmodel_UT_004)
{
    Stub stub;
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(PackagesManager, isBackendReady), model_BackendReady);
    stub.set(ADDR(DebListModel, checkSystemVersion),model_checkSystemVersion);


    DebListModel *model = new DebListModel();
    usleep(5*1000);

    ASSERT_TRUE(model->isWorkerPrepare());
}

TEST(deblistmodel_Test, deblistmodel_UT_005)
{
    Stub stub;
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(PackagesManager, isBackendReady), model_BackendReady);
    stub.set(ADDR(DebListModel, checkSystemVersion),model_checkSystemVersion);


    DebListModel *model = new DebListModel();

    usleep(5*1000);
    ASSERT_TRUE(model->preparedPackages().isEmpty());
}

QString model_deb_arch_all()
{
    return "all";
}

QString model_deb_arch_i386()
{
    return "i386";
}

QStringList model_architectures()
{
    return {"i386", "amd64"};
}

bool model_deb_isValid()
{
    return true;
}

QByteArray model_deb_md5Sum()
{
    return nullptr;
}

int model_deb_installSize()
{
    return 0;
}

QString model_deb_packageName()
{
    return "";
}

QString model_deb_longDescription()
{
    return "longDescription";
}

QString model_deb_shortDescription()
{
    return "shortDescription";
}

QString model_deb_version()
{
    return "version";
}


QList<DependencyItem> model_deb_conflicts()
{
    DependencyInfo info("packageName","0.0",RelationType::Equals,Depends);
    QList<DependencyInfo> dependencyItem;
    dependencyItem<<info;
    QList<DependencyItem> conflicts;
    conflicts<<dependencyItem;

    return conflicts;
}

Package *model_packageWithArch(QString, QString, QString)
{
    return nullptr;
}

PackageList model_backend_availablePackages()
{
    return {};
}

QLatin1String model_package_name()
{
    return QLatin1String("name");
}

QString model_package_version()
{
    return "version";
}

QString model_package_architecture()
{
    return "i386";
}

QList<DependencyItem> model_conflicts()
{
    DependencyInfo info("packageName","0.0",RelationType::Equals,Conflicts);
    QList<DependencyInfo> dependencyItem;
    dependencyItem<<info;
    QList<DependencyItem> conflicts;
    conflicts<<dependencyItem;

    return conflicts;
}

QList<DependencyItem> model_package_conflicts()
{
    DependencyInfo info("packageName","0.0",RelationType::Equals,Conflicts);
    QList<DependencyInfo> dependencyItem;
    dependencyItem<<info;
    QList<DependencyItem> conflicts;
    conflicts<<dependencyItem;

    return conflicts;
}
QStringList model_backend_architectures()
{
    return {"i386", "amd64"};
}

TEST(deblistmodel_Test, deblistmodel_UT_006)
{
    Stub stub;
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(PackagesManager, isBackendReady), model_BackendReady);
    stub.set(ADDR(DebListModel, checkSystemVersion),model_checkSystemVersion);

    stub.set(ADDR(DebFile, architecture), model_deb_arch_i386);
    stub.set(ADDR(Backend, architectures), model_backend_architectures);
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(DebFile, isValid),model_deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),model_deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),model_deb_installSize);
    stub.set(ADDR(DebFile, packageName),model_deb_packageName);
    stub.set(ADDR(DebFile, longDescription),model_deb_longDescription);
    stub.set(ADDR(DebFile, version),model_deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), model_packageWithArch);

    stub.set(ADDR(DebFile, conflicts), model_deb_conflicts);

    DebListModel *model = new DebListModel();

    usleep(5*1000);

    QStringList list;
    list<<"/";
    model->appendPackage(list);

    ASSERT_EQ(model->preparedPackages().size(),1);
}

TEST(deblistmodel_Test, deblistmodel_UT_007)
{
    Stub stub;
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(PackagesManager, isBackendReady), model_BackendReady);
    stub.set(ADDR(DebListModel, checkSystemVersion),model_checkSystemVersion);

    stub.set(ADDR(DebFile, architecture), model_deb_arch_i386);
    stub.set(ADDR(Backend, architectures), model_backend_architectures);
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(DebFile, isValid),model_deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),model_deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),model_deb_installSize);
    stub.set(ADDR(DebFile, packageName),model_deb_packageName);
    stub.set(ADDR(DebFile, longDescription),model_deb_longDescription);
    stub.set(ADDR(DebFile, version),model_deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), model_packageWithArch);

    stub.set(ADDR(DebFile, conflicts), model_deb_conflicts);

    DebListModel *model = new DebListModel();

    usleep(5*1000);

    QStringList list;
    list<<"/";
    model->appendPackage(list);

    ASSERT_EQ(model->first().row(),0);
}

TEST(deblistmodel_Test, deblistmodel_UT_008)
{
    Stub stub;
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(PackagesManager, isBackendReady), model_BackendReady);
    stub.set(ADDR(DebListModel, checkSystemVersion),model_checkSystemVersion);

    stub.set(ADDR(DebFile, architecture), model_deb_arch_i386);
    stub.set(ADDR(Backend, architectures), model_backend_architectures);
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(DebFile, isValid),model_deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),model_deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),model_deb_installSize);
    stub.set(ADDR(DebFile, packageName),model_deb_packageName);
    stub.set(ADDR(DebFile, longDescription),model_deb_longDescription);
    stub.set(ADDR(DebFile, version),model_deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), model_packageWithArch);

    stub.set(ADDR(DebFile, conflicts), model_deb_conflicts);

    DebListModel *model = new DebListModel();

    usleep(5*1000);

    QStringList list;
    list<<"/";
    model->appendPackage(list);

    QModelIndex index;
    ASSERT_EQ(model->rowCount(index),1);
}

TEST(deblistmodel_Test, deblistmodel_UT_009)
{
    Stub stub;
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(PackagesManager, isBackendReady), model_BackendReady);
    stub.set(ADDR(DebListModel, checkSystemVersion),model_checkSystemVersion);

    stub.set(ADDR(DebFile, architecture), model_deb_arch_i386);
    stub.set(ADDR(Backend, architectures), model_backend_architectures);
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(DebFile, isValid),model_deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),model_deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),model_deb_installSize);
    stub.set(ADDR(DebFile, packageName),model_deb_packageName);
    stub.set(ADDR(DebFile, longDescription),model_deb_longDescription);
    stub.set(ADDR(DebFile, shortDescription),model_deb_shortDescription);
    stub.set(ADDR(DebFile, version),model_deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), model_packageWithArch);

    stub.set(ADDR(DebFile, conflicts), model_deb_conflicts);

    DebListModel *model = new DebListModel();

    usleep(5*1000);

    QStringList list;
    list<<"/";
    model->appendPackage(list);

    QModelIndex index = model->index(0);
    ASSERT_TRUE(model->data(index,DebListModel::WorkerIsPrepareRole).toBool());
}

TEST(deblistmodel_Test, deblistmodel_UT_010)
{
    Stub stub;
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(PackagesManager, isBackendReady), model_BackendReady);
    stub.set(ADDR(DebListModel, checkSystemVersion),model_checkSystemVersion);

    stub.set(ADDR(DebFile, architecture), model_deb_arch_i386);
    stub.set(ADDR(Backend, architectures), model_backend_architectures);
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(DebFile, isValid),model_deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),model_deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),model_deb_installSize);
    stub.set(ADDR(DebFile, packageName),model_deb_packageName);
    stub.set(ADDR(DebFile, longDescription),model_deb_longDescription);
    stub.set(ADDR(DebFile, shortDescription),model_deb_shortDescription);
    stub.set(ADDR(DebFile, version),model_deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), model_packageWithArch);

    stub.set(ADDR(DebFile, conflicts), model_deb_conflicts);

    DebListModel *model = new DebListModel();

    usleep(5*1000);

    QStringList list;
    list<<"/";
    model->appendPackage(list);

    model->initPrepareStatus();
    ASSERT_EQ(model->m_packageOperateStatus[0], DebListModel::Prepare);
}

Package *model_package_package(const QString & name)
{
    return nullptr;
}

QList<DependencyItem> model_deb_depends()
{
    DependencyInfo info("packageName","0.0",RelationType::Equals,Depends);
    QList<DependencyInfo> dependencyItem;
    dependencyItem<<info;
    QList<DependencyItem> conflicts;
    conflicts<<dependencyItem;

    return conflicts;
}

PackageDependsStatus model_getPackageDependsStatus(const int index)
{
    Q_UNUSED(index);
    PackageDependsStatus status;
    return status;
}
TEST(deblistmodel_Test, deblistmodel_UT_011)
{
    Stub stub;
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(PackagesManager, isBackendReady), model_BackendReady);
    stub.set(ADDR(DebListModel, checkSystemVersion),model_checkSystemVersion);

    stub.set(ADDR(DebFile, architecture), model_deb_arch_i386);
    stub.set(ADDR(Backend, architectures), model_backend_architectures);
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(DebFile, isValid),model_deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),model_deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),model_deb_installSize);
    stub.set(ADDR(DebFile, packageName),model_deb_packageName);
    stub.set(ADDR(DebFile, longDescription),model_deb_longDescription);
    stub.set(ADDR(DebFile, shortDescription),model_deb_shortDescription);
    stub.set(ADDR(DebFile, version),model_deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), model_packageWithArch);
    stub.set((Package *(Backend::*)(const QString &) const)ADDR(Backend,package), model_package_package);


    stub.set(ADDR(DebFile, depends),model_deb_depends);

    stub.set(ADDR(DebFile, conflicts), model_deb_conflicts);

    stub.set(ADDR(PackagesManager, getPackageDependsStatus), model_getPackageDependsStatus);

    DebListModel *model = new DebListModel();

    usleep(5*1000);

    QStringList list;
    list<<"/";
    model->appendPackage(list);

    model->initDependsStatus();
    ASSERT_EQ(model->index(0).data(DebListModel::PackageDependsStatusRole).toInt(), DebListModel::DependsOk);
}

TEST(deblistmodel_Test, deblistmodel_UT_012)
{
    Stub stub;
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(PackagesManager, isBackendReady), model_BackendReady);
    stub.set(ADDR(DebListModel, checkSystemVersion),model_checkSystemVersion);

    stub.set(ADDR(DebFile, architecture), model_deb_arch_i386);
    stub.set(ADDR(Backend, architectures), model_backend_architectures);
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(DebFile, isValid),model_deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),model_deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),model_deb_installSize);
    stub.set(ADDR(DebFile, packageName),model_deb_packageName);
    stub.set(ADDR(DebFile, longDescription),model_deb_longDescription);
    stub.set(ADDR(DebFile, shortDescription),model_deb_shortDescription);
    stub.set(ADDR(DebFile, version),model_deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), model_packageWithArch);

    stub.set(ADDR(DebFile, conflicts), model_deb_conflicts);

    DebListModel *model = new DebListModel();

    usleep(5*1000);

    QStringList list;
    list<<"/";
    model->appendPackage(list);


    ASSERT_EQ(model->getInstallFileSize(),1);
}

TEST(deblistmodel_Test, deblistmodel_UT_013)
{
    Stub stub;
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(PackagesManager, isBackendReady), model_BackendReady);
    stub.set(ADDR(DebListModel, checkSystemVersion),model_checkSystemVersion);

    stub.set(ADDR(DebFile, architecture), model_deb_arch_i386);
    stub.set(ADDR(Backend, architectures), model_backend_architectures);
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(DebFile, isValid),model_deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),model_deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),model_deb_installSize);
    stub.set(ADDR(DebFile, packageName),model_deb_packageName);
    stub.set(ADDR(DebFile, longDescription),model_deb_longDescription);
    stub.set(ADDR(DebFile, shortDescription),model_deb_shortDescription);
    stub.set(ADDR(DebFile, version),model_deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), model_packageWithArch);

    stub.set(ADDR(DebFile, conflicts), model_deb_conflicts);

    DebListModel *model = new DebListModel();

    usleep(5*1000);

    QStringList list;
    list<<"/";
    model->appendPackage(list);


    QModelIndex index = model->index(0);
    model->setCurrentIndex(index);
    ASSERT_EQ(model->m_currentIdx, model->index(0));
}

void model_initRowStatus(){
    return;
}

void model_installNextDeb(){
    return;
}

TEST(deblistmodel_Test, deblistmodel_UT_014)
{
    Stub stub;
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(PackagesManager, isBackendReady), model_BackendReady);
    stub.set(ADDR(DebListModel, checkSystemVersion),model_checkSystemVersion);

    stub.set(ADDR(DebFile, architecture), model_deb_arch_i386);
    stub.set(ADDR(Backend, architectures), model_backend_architectures);
    stub.set(ADDR(Backend, init), model_backend_init);
    stub.set(ADDR(DebFile, isValid),model_deb_isValid);
    stub.set(ADDR(DebFile, md5Sum),model_deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize),model_deb_installSize);
    stub.set(ADDR(DebFile, packageName),model_deb_packageName);
    stub.set(ADDR(DebFile, longDescription),model_deb_longDescription);
    stub.set(ADDR(DebFile, shortDescription),model_deb_shortDescription);
    stub.set(ADDR(DebFile, version),model_deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), model_packageWithArch);

    stub.set(ADDR(DebFile, conflicts), model_deb_conflicts);

    stub.set(ADDR(DebListModel, initRowStatus), model_initRowStatus);
    stub.set(ADDR(DebListModel, installNextDeb), model_installNextDeb);
    DebListModel *model = new DebListModel();

    usleep(5*1000);

    QStringList list;
    list<<"/";
    model->appendPackage(list);

    model->installPackages();
    ASSERT_EQ(model->m_workerStatus, DebListModel::WorkerProcessing);
}
