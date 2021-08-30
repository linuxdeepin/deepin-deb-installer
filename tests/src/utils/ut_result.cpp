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

#include "../src/deb-installer/utils/result.h"
#include <QString>
typedef Result<QString> TestResult;

TEST(Result_Test, Result_UT_001)
{
    TestResult t = TestResult::err("test");
}

TEST(Result_Test, Result_UT_002)
{
    TestResult t = TestResult::err("test");
    t.err("test");
    t.unwrap();
}
TEST(Result_Test, Result_UT_003)
{
    Result<QString> t(true,"QString");
    t.is_ok();
}
