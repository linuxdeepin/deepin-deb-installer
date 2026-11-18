// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QApt/Package>
#include <QApt/Backend>
#include <stub.h>

#define private public
#include "../deb-installer/model/packageanalyzer.h"

class ut_packageanalyzer_TEST : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() {}
    void TearDown() {}

private:
    PackageAnalyzer *w;
};

TEST_F(ut_packageanalyzer_TEST, PackageAnalyzer_UT_supportArch)
{
    ASSERT_EQ(PackageAnalyzer::instance().supportArch("any"), true);
    ASSERT_EQ(PackageAnalyzer::instance().supportArch("all"), true);
}

TEST_F(ut_packageanalyzer_TEST, PackageAnalyzer_UT_packageInstallStatus)
{
    DebIr ir;
    ir.isValid = true;
    ir.packageName = "deepin-hd;o3h8dhoewl";
    ir.architecture = "amd64";

    ir.version = "1.0";
    auto data = PackageAnalyzer::instance().packageInstallStatus(ir);
    ASSERT_EQ(data.first, Pkg::NotInstalled);

    ir.packageName = "libc6";
    data = PackageAnalyzer::instance().packageInstallStatus(ir);
    ASSERT_EQ(data.first, Pkg::InstalledLaterVersion);

    ir.version = "999.999";
    data = PackageAnalyzer::instance().packageInstallStatus(ir);
    ASSERT_EQ(data.first, Pkg::InstalledEarlierVersion);
}

TEST_F(ut_packageanalyzer_TEST, PackageAnalyzer_UT_packageWithArch)
{
    auto package = PackageAnalyzer::instance().packageWithArch("libc6", "deepin-hd;o3h8dhoewl", "");
    ASSERT_NE(package, nullptr);
}

TEST_F(ut_packageanalyzer_TEST, PackageAnalyzer_UT_virtualPackageIsExist)
{
    auto result = PackageAnalyzer::instance().virtualPackageIsExist("deepin-hd;o3h8dhoewl");
    ASSERT_EQ(result, false);
}

TEST_F(ut_packageanalyzer_TEST, PackageAnalyzer_UT_versionMatched)
{
    bool result;

    result = PackageAnalyzer::instance().versionMatched("1.0", "1.0", QApt::LessOrEqual);
    ASSERT_EQ(result, true);

    result = PackageAnalyzer::instance().versionMatched("1.5", "1.0", QApt::GreaterOrEqual);
    ASSERT_EQ(result, false);

    result = PackageAnalyzer::instance().versionMatched("1.0", "1.5", QApt::GreaterThan);
    ASSERT_EQ(result, true);

    result = PackageAnalyzer::instance().versionMatched("1.0", "1.0", QApt::NotEqual);
    ASSERT_EQ(result, false);

    result = PackageAnalyzer::instance().versionMatched("1.0", "1.1", QApt::NotEqual);
    ASSERT_EQ(result, true);
}

TEST_F(ut_packageanalyzer_TEST, PackageAnalyzer_UT_readDConfigLongLong)
{
    qint64 result = PackageAnalyzer::instance().readDConfigLongLong("lastCacheUpdateTime", 0);
    ASSERT_GE(result, 0);
}

TEST_F(ut_packageanalyzer_TEST, PackageAnalyzer_UT_writeAndReadDConfigLongLong)
{
    qint64 testValue = 1700000000;
    bool writeOk = PackageAnalyzer::instance().writeDConfigLongLong("lastCacheUpdateTime", testValue);

    if (writeOk) {
        qint64 readValue = PackageAnalyzer::instance().readDConfigLongLong("lastCacheUpdateTime", 0);
        ASSERT_EQ(readValue, testValue);
    } else {
        qint64 readValue = PackageAnalyzer::instance().readDConfigLongLong("lastCacheUpdateTime", 0);
        ASSERT_EQ(readValue, 0);
    }
}

TEST_F(ut_packageanalyzer_TEST, PackageAnalyzer_UT_shouldUpdateCache_cooldownActive)
{
    // 冷却期内：lastCacheUpdateTime 设为当前时间 → shouldUpdateCache 返回 false
    PackageAnalyzer::instance().writeDConfigLongLong("lastCacheUpdateTime", QDateTime::currentSecsSinceEpoch());
    ASSERT_EQ(PackageAnalyzer::instance().shouldUpdateCache(), false);
}

TEST_F(ut_packageanalyzer_TEST, PackageAnalyzer_UT_shouldUpdateCache_cooldownExpired)
{
    // 冷却期外：lastCacheUpdateTime 设为 31 分钟前 → 跳过冷却 → 走 mtime 逻辑
    PackageAnalyzer::instance().writeDConfigLongLong("lastCacheUpdateTime", QDateTime::currentSecsSinceEpoch() - 1860);
    bool expiredResult = PackageAnalyzer::instance().shouldUpdateCache();

    // 冷却期外与 lastCacheUpdateTime=0（同样跳过冷却）的结果应一致，证明冷却期被正确跳过
    PackageAnalyzer::instance().writeDConfigLongLong("lastCacheUpdateTime", 0);
    bool zeroResult = PackageAnalyzer::instance().shouldUpdateCache();

    ASSERT_EQ(expiredResult, zeroResult);
}

TEST_F(ut_packageanalyzer_TEST, PackageAnalyzer_UT_shouldUpdateCache_zeroTimestampSkipsCooldown)
{
    // lastCacheUpdateTime=0：跳过冷却 → 走 mtime 逻辑
    PackageAnalyzer::instance().writeDConfigLongLong("lastCacheUpdateTime", 0);
    bool zeroResult = PackageAnalyzer::instance().shouldUpdateCache();

    // 冷却期内设为当前时间后必定返回 false
    PackageAnalyzer::instance().writeDConfigLongLong("lastCacheUpdateTime", QDateTime::currentSecsSinceEpoch());
    ASSERT_EQ(PackageAnalyzer::instance().shouldUpdateCache(), false);

    // 验证 lastCacheUpdateTime=0 确实跳过了冷却（结果与冷却期外一致）
    PackageAnalyzer::instance().writeDConfigLongLong("lastCacheUpdateTime", QDateTime::currentSecsSinceEpoch() - 1860);
    bool expiredResult = PackageAnalyzer::instance().shouldUpdateCache();
    ASSERT_EQ(zeroResult, expiredResult);
}
