// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "../deb-installer/model/deblistmodel.h"
#include "../deb-installer/model/backend/apt_install_backend.h"
#include "../deb-installer/manager/packagesmanager.h"
#include "../deb-installer/manager/PackageDependsStatus.h"
#include "../deb-installer/utils/hierarchicalverify.h"

#include <stub.h>

#include <QList>
#include <unistd.h>

#include <fstream>

using namespace QApt;

namespace {

const QByteArray kMd5 { "md5-deps-loop" };

bool aptBackend_backend_init()
{
    return true;
}

QStringList aptBackend_backend_architectures()
{
    return { "i386", "amd64" };
}

QApt::Transaction *aptBackend_backend_commitChanges()
{
    return nullptr;
}

Package *aptBackend_packageWithArch(QString, QString, QString)
{
    return nullptr;
}

Package *aptBackend_backend_package(const QString &)
{
    return nullptr;
}

bool aptBackend_isBackendReady()
{
    return true;
}

QString aptBackend_packageManager_package(const int)
{
    return "";
}

bool aptBackend_dealInvalidPackage(QString)
{
    return true;
}

bool aptBackend_stub_model_is_open()
{
    return true;
}

void aptBackend_stub_model_open(const std::string &, std::ios_base::openmode)
{
}

bool aptBackend_Hierarchical_isValid()
{
    return false;
}

QApt::ExitStatus aptBackend_exitStatus_success()
{
    return QApt::ExitSuccess;
}

QApt::Transaction *g_senderTransaction = nullptr;

QObject *aptBackend_sender()
{
    if (!g_senderTransaction)
        g_senderTransaction = new QApt::Transaction("1");
    return g_senderTransaction;
}

DebListModel *g_model = nullptr;
bool g_installNextDebCalled = false;
bool g_dependsCachePresentWhenInstallNextDeb = false;

void aptBackend_installNextDeb_watchCache()
{
    g_installNextDebCalled = true;
    g_dependsCachePresentWhenInstallNextDeb =
        g_model->m_packagesManager->m_packageMd5DependsStatus.contains(kMd5);
}

} // namespace

class ut_AptInstallBackend_test : public ::testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    DebListModel *m_debListModel = nullptr;
    Stub stub;
};

void ut_AptInstallBackend_test::SetUp()
{
    stub.set((void(std::fstream::*)(const std::string &, std::ios_base::openmode))ADDR(std::fstream, open),
             aptBackend_stub_model_open);
    stub.set((bool(std::fstream::*)())ADDR(std::fstream, is_open), aptBackend_stub_model_is_open);

    stub.set(ADDR(Backend, init), aptBackend_backend_init);
    stub.set(ADDR(Backend, architectures), aptBackend_backend_architectures);
    stub.set(ADDR(Backend, commitChanges), aptBackend_backend_commitChanges);
    stub.set(ADDR(Backend, reloadCache), aptBackend_backend_init);

    stub.set((Package *(Backend::*)(const QString &) const)ADDR(Backend, package), aptBackend_backend_package);

    stub.set(ADDR(PackagesManager, isBackendReady), aptBackend_isBackendReady);
    stub.set(ADDR(PackagesManager, packageWithArch), aptBackend_packageWithArch);
    stub.set(ADDR(PackagesManager, package), aptBackend_packageManager_package);
    stub.set(ADDR(PackagesManager, dealInvalidPackage), aptBackend_dealInvalidPackage);

    stub.set(ADDR(HierarchicalVerify, isValid), aptBackend_Hierarchical_isValid);

    g_installNextDebCalled = false;
    g_dependsCachePresentWhenInstallNextDeb = false;

    m_debListModel = new DebListModel();
    g_model = m_debListModel;
    usleep(10 * 1000);
}

void ut_AptInstallBackend_test::TearDown()
{
    g_model = nullptr;
    delete m_debListModel;
    if (g_senderTransaction) {
        delete g_senderTransaction;
        g_senderTransaction = nullptr;
    }
}

TEST_F(ut_AptInstallBackend_test, aptInstallBackend_UT_slotDependsInstallTransactionFinished_success_invalidatesDependsCache)
{
    stub.set(ADDR(Transaction, exitStatus), aptBackend_exitStatus_success);
    stub.set(ADDR(DebListModel, installNextDeb), aptBackend_installNextDeb_watchCache);

    PackagesManager *pm = m_debListModel->m_packagesManager;
    pm->m_preparedPackages.append("/tmp/ut-deps-loop.deb");
    pm->m_packageMd5.append(kMd5);
    pm->m_packageMd5DependsStatus.insert(kMd5, PackageDependsStatus(Pkg::DependsAvailable, "ut-deps-loop"));

    m_debListModel->m_operatingIndex = 0;
    m_debListModel->m_operatingStatusIndex = 0;
    m_debListModel->m_operatingPackageMd5 = kMd5;

    AptInstallBackend backend(m_debListModel);

    Stub stubSender;
    stubSender.set(ADDR(QObject, sender), aptBackend_sender);

    backend.slotDependsInstallTransactionFinished();

    EXPECT_TRUE(g_installNextDebCalled);
    EXPECT_FALSE(g_dependsCachePresentWhenInstallNextDeb);
    EXPECT_FALSE(pm->m_packageMd5DependsStatus.contains(kMd5));
}
