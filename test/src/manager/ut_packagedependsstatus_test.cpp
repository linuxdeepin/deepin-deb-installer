#include <gtest/gtest.h>

#define private public  // hack complier
#define protected public

#include "../deb_installer/manager/PackageDependsStatus.h"

#undef private
#undef protected

#include "../deb_installer/model/deblistmodel.h"
#include <stub.h>

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_001)
{
    PackageDependsStatus *pds = new PackageDependsStatus();

    ASSERT_EQ(pds->ok().status,DebListModel::DependsOk);
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_002)
{
    PackageDependsStatus *pds = new PackageDependsStatus();

    ASSERT_EQ(pds->available("package").status,DebListModel::DependsAvailable);
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_003)
{
    PackageDependsStatus *pds = new PackageDependsStatus(DebListModel::DependsBreak, "packageName");

    ASSERT_TRUE(pds->isBreak());
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_004)
{
    PackageDependsStatus *pds = new PackageDependsStatus(DebListModel::DependsAvailable, "packageName");

    ASSERT_TRUE(pds->isAvailable());
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_005)
{
    PackageDependsStatus *pds = new PackageDependsStatus(DebListModel::DependsAuthCancel, "packageName");

    ASSERT_TRUE(pds->isAuthCancel());
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_006)
{
    PackageDependsStatus *small = new PackageDependsStatus(DebListModel::DependsAvailable, "packageName");

    PackageDependsStatus *big = new PackageDependsStatus(DebListModel::DependsBreak, "packageName");

    ASSERT_TRUE(small->min(*big).isAvailable());
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_007)
{
    PackageDependsStatus *small = new PackageDependsStatus(DebListModel::DependsAvailable, "packageName");

    PackageDependsStatus *big = new PackageDependsStatus(DebListModel::DependsBreak, "packageName");

    ASSERT_TRUE(small->minEq(*big).isAvailable());
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_008)
{
    PackageDependsStatus *small = new PackageDependsStatus(DebListModel::DependsAvailable, "packageName");

    PackageDependsStatus *big = new PackageDependsStatus(DebListModel::DependsBreak, "packageName");

    ASSERT_TRUE(small->max(*big).isBreak());
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_009)
{
    PackageDependsStatus *small = new PackageDependsStatus(DebListModel::DependsAvailable, "packageName");

    PackageDependsStatus *big = new PackageDependsStatus(DebListModel::DependsBreak, "packageName");

    ASSERT_TRUE(small->maxEq(*big).isBreak());
}
