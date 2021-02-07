#include <gtest/gtest.h>

#define private public  // hack complier
#define protected public

#include "../deb_installer/utils/result.h"

#undef private
#undef protected
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
