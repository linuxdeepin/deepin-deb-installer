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


#include "../deb-installer/model/deblistmodel.h"
#include <stub.h>

class ut_packageDependsStatus_Test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp()
    {
        big = new PackageDependsStatus(DebListModel::DependsBreak, "packageName");
    }
    void TearDown()
    {
        delete m_pds;
        delete big;
    }

    PackageDependsStatus *m_pds = nullptr;
    PackageDependsStatus *big = nullptr;
};

TEST_F(ut_packageDependsStatus_Test, PackageDependsStatus_UT_ok)
{
    ASSERT_EQ(m_pds->ok().status, DebListModel::DependsOk);
}

TEST_F(ut_packageDependsStatus_Test, PackageDependsStatus_UT_available)
{
    ASSERT_EQ(m_pds->available("package").status, DebListModel::DependsAvailable);
}

TEST_F(ut_packageDependsStatus_Test, PackageDependsStatus_UT_isBreak)
{
    m_pds = new PackageDependsStatus(DebListModel::DependsBreak, "packageName");
    ASSERT_TRUE(m_pds->isBreak());
}

TEST_F(ut_packageDependsStatus_Test, PackageDependsStatus_UT_isAvailable)
{
    m_pds = new PackageDependsStatus(DebListModel::DependsAvailable, "packageName");

    ASSERT_TRUE(m_pds->isAvailable());
}

TEST_F(ut_packageDependsStatus_Test, PackageDependsStatus_UT_isAuthCancel)
{
    m_pds = new PackageDependsStatus(DebListModel::DependsAuthCancel, "packageName");
    ASSERT_TRUE(m_pds->isAuthCancel());
}

TEST_F(ut_packageDependsStatus_Test, PackageDependsStatus_UT_min)
{
    m_pds = new PackageDependsStatus(DebListModel::DependsAvailable, "packageName");
    ASSERT_TRUE(m_pds->min(*big).isAvailable());
}

TEST_F(ut_packageDependsStatus_Test, PackageDependsStatus_UT_minEq)
{
    m_pds = new PackageDependsStatus(DebListModel::DependsAvailable, "packageName");
    ASSERT_TRUE(m_pds->minEq(*big).isAvailable());
}

TEST_F(ut_packageDependsStatus_Test, PackageDependsStatus_UT_max)
{
    m_pds = new PackageDependsStatus(DebListModel::DependsAvailable, "packageName");
    ASSERT_TRUE(m_pds->max(*big).isBreak());
}

TEST_F(ut_packageDependsStatus_Test, PackageDependsStatus_UT_maxEq)
{
    m_pds = new PackageDependsStatus(DebListModel::DependsAvailable, "packageName");
    ASSERT_TRUE(m_pds->maxEq(*big).isBreak());
}
