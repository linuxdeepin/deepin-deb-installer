#include <gtest/gtest.h>

#define private public  // hack complier
#define protected public

#include "../deb_installer/utils/DebugTimeManager.h"

#undef private
#undef protected
#include <stub.h>

TEST(DebugTimeManager_Test, DebugTimeManager_UT_001)
{
    DebugTimeManager * dtm = new DebugTimeManager();
    dtm->beginPointQt("001","");
    ASSERT_FALSE(dtm->m_MapPoint.isEmpty());
}

TEST(DebugTimeManager_Test, DebugTimeManager_UT_002)
{
    DebugTimeManager * dtm = new DebugTimeManager();
    dtm->beginPointQt("001","");
    dtm->endPointQt("001");
    ASSERT_TRUE(dtm->m_MapPoint["001"].time >=0);
}

TEST(DebugTimeManager_Test, DebugTimeManager_UT_003)
{
    DebugTimeManager * dtm = new DebugTimeManager();
    dtm->clear();

    ASSERT_TRUE(dtm->m_MapPoint.isEmpty());
}
