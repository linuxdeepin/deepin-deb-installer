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

#include "../deb-installer/utils/DebugTimeManager.h"

#undef private
#undef protected
#include <stub.h>

TEST(DebugTimeManager_Test, DebugTimeManager_UT_001)
{
    DebugTimeManager *dtm = new DebugTimeManager();
    dtm->beginPointQt("001", "");
    ASSERT_FALSE(dtm->m_MapPoint.isEmpty());
    delete  dtm;
}

TEST(DebugTimeManager_Test, DebugTimeManager_UT_002)
{
    DebugTimeManager *dtm = new DebugTimeManager();
    dtm->beginPointQt("001", "");
    dtm->endPointQt("001");
    ASSERT_TRUE(dtm->m_MapPoint["001"].time >= 0);
    delete  dtm;
}

TEST(DebugTimeManager_Test, DebugTimeManager_UT_003)
{
    DebugTimeManager *dtm = new DebugTimeManager();
    dtm->clear();

    ASSERT_TRUE(dtm->m_MapPoint.isEmpty());
    delete  dtm;
}
