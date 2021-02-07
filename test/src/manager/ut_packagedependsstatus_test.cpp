#include <gtest/gtest.h>

#include "../deb_installer/manager/PackageDependsStatus.h"


#include "../deb_installer/model/deblistmodel.h"
#include <stub.h>

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_ok)
{
    PackageDependsStatus *pds = new PackageDependsStatus();

    ASSERT_EQ(pds->ok().status, DebListModel::DependsOk);
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_available)
{
    PackageDependsStatus *pds = new PackageDependsStatus();

    ASSERT_EQ(pds->available("package").status, DebListModel::DependsAvailable);
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_isBreak)
{
    PackageDependsStatus *pds = new PackageDependsStatus(DebListModel::DependsBreak, "packageName");

    ASSERT_TRUE(pds->isBreak());
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_isAvailable)
{
    PackageDependsStatus *pds = new PackageDependsStatus(DebListModel::DependsAvailable, "packageName");

    ASSERT_TRUE(pds->isAvailable());
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_isAuthCancel)
{
    PackageDependsStatus *pds = new PackageDependsStatus(DebListModel::DependsAuthCancel, "packageName");

    ASSERT_TRUE(pds->isAuthCancel());
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_min)
{
    PackageDependsStatus *small = new PackageDependsStatus(DebListModel::DependsAvailable, "packageName");

    PackageDependsStatus *big = new PackageDependsStatus(DebListModel::DependsBreak, "packageName");

    ASSERT_TRUE(small->min(*big).isAvailable());
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_minEq)
{
    PackageDependsStatus *small = new PackageDependsStatus(DebListModel::DependsAvailable, "packageName");

    PackageDependsStatus *big = new PackageDependsStatus(DebListModel::DependsBreak, "packageName");

    ASSERT_TRUE(small->minEq(*big).isAvailable());
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_max)
{
    PackageDependsStatus *small = new PackageDependsStatus(DebListModel::DependsAvailable, "packageName");

    PackageDependsStatus *big = new PackageDependsStatus(DebListModel::DependsBreak, "packageName");

    ASSERT_TRUE(small->max(*big).isBreak());
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_maxEq)
{
    PackageDependsStatus *small = new PackageDependsStatus(DebListModel::DependsAvailable, "packageName");

    PackageDependsStatus *big = new PackageDependsStatus(DebListModel::DependsBreak, "packageName");

    ASSERT_TRUE(small->maxEq(*big).isBreak());
}
