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


#include "../deb_installer/model/deblistmodel.h"
#include <stub.h>

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_ok)
{
    PackageDependsStatus *pds = new PackageDependsStatus();

    ASSERT_EQ(pds->ok().status, DebListModel::DependsOk);
    delete pds;
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_available)
{
    PackageDependsStatus *pds = new PackageDependsStatus();

    ASSERT_EQ(pds->available("package").status, DebListModel::DependsAvailable);
    delete pds;
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_isBreak)
{
    PackageDependsStatus *pds = new PackageDependsStatus(DebListModel::DependsBreak, "packageName");

    ASSERT_TRUE(pds->isBreak());
    delete pds;
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_isAvailable)
{
    PackageDependsStatus *pds = new PackageDependsStatus(DebListModel::DependsAvailable, "packageName");

    ASSERT_TRUE(pds->isAvailable());
    delete pds;
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_isAuthCancel)
{
    PackageDependsStatus *pds = new PackageDependsStatus(DebListModel::DependsAuthCancel, "packageName");

    ASSERT_TRUE(pds->isAuthCancel());
    delete pds;
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_min)
{
    PackageDependsStatus *small = new PackageDependsStatus(DebListModel::DependsAvailable, "packageName");

    PackageDependsStatus *big = new PackageDependsStatus(DebListModel::DependsBreak, "packageName");

    ASSERT_TRUE(small->min(*big).isAvailable());
    delete small;
    delete big;
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_minEq)
{
    PackageDependsStatus *small = new PackageDependsStatus(DebListModel::DependsAvailable, "packageName");

    PackageDependsStatus *big = new PackageDependsStatus(DebListModel::DependsBreak, "packageName");

    ASSERT_TRUE(small->minEq(*big).isAvailable());
    delete small;
    delete big;
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_max)
{
    PackageDependsStatus *small = new PackageDependsStatus(DebListModel::DependsAvailable, "packageName");

    PackageDependsStatus *big = new PackageDependsStatus(DebListModel::DependsBreak, "packageName");

    ASSERT_TRUE(small->max(*big).isBreak());
    delete small;
    delete big;
}

TEST(PackageDependsStatus_Test, PackageDependsStatus_UT_maxEq)
{
    PackageDependsStatus *small = new PackageDependsStatus(DebListModel::DependsAvailable, "packageName");

    PackageDependsStatus *big = new PackageDependsStatus(DebListModel::DependsBreak, "packageName");

    ASSERT_TRUE(small->maxEq(*big).isBreak());
    delete small;
    delete big;
}
