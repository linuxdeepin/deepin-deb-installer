// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QDebug>

#include "../stub.h"

#include "../deb-installer/compatible/compatible_json_parser.h"
#include "../deb-installer/compatible/compatible_backend.h"

class UTCompatibleJsonParser : public ::testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
};

void UTCompatibleJsonParser::SetUp() {}

void UTCompatibleJsonParser::TearDown() {}

static const QByteArray kAppListJson =
    "{\"Code\":0,\"Msg\":null,\"Ext\":{\"Code\":0,\"Msg\":[\"\xE6\x88\x90\xE5\x8A\x9F [\\n  {\\n    \\\"name\\\": "
    "\\\"hello\\\",\\n    \\\"version\\\": \\\"2.10-2\\\",\\n    \\\"arch\\\": \\\"amd64\\\"\\n  },\\n  {\\n    \\\"name\\\": "
    "\\\"tree\\\",\\n    \\\"version\\\": \\\"1.8.0-1\\\",\\n    \\\"arch\\\": \\\"amd64\\\"\\n  }\\n]\"]}}\n";

TEST_F(UTCompatibleJsonParser, parseAppListSuccess)
{
    auto ret = Compatible::CompatibleJsonParser::parseCommonField(kAppListJson);

    ASSERT_FALSE(ret.isNull());

    EXPECT_EQ(ret->code, 0);
    EXPECT_EQ(ret->ext.code, 0);
    EXPECT_TRUE(ret->message.isEmpty());

    auto pkgList = Compatible::CompatibleJsonParser::parseAppList(ret);

    ASSERT_EQ(pkgList.size(), 2);

    EXPECT_EQ(pkgList.value("hello")->name, QString("hello"));
    EXPECT_EQ(pkgList.value("hello")->version, QString("2.10-2"));
    EXPECT_EQ(pkgList.value("hello")->arch, QString("amd64"));
    EXPECT_EQ(pkgList.value("tree")->name, QString("tree"));
    EXPECT_EQ(pkgList.value("tree")->version, QString("1.8.0-1"));
    EXPECT_EQ(pkgList.value("tree")->arch, QString("amd64"));
}

static const QByteArray kSupportRootfsJson = "{\"Code\":0,\"Msg\":null,\"Ext\":{\"Code\":0,\"Msg\":[\"成功 [\\n "
                                             "\\\"uos-rootfs-20\\\",\\n \\\"ubuntu-18.04\\\" \\n]\"]}}";

QList<Compatible::RootfsInfo::Ptr> stub_rootfsList()
{
    QList<Compatible::RootfsInfo::Ptr> rootfsList;
    auto uosPtr = Compatible::RootfsInfo::Ptr::create();
    uosPtr->name = "uos-rootfs-20";
    rootfsList.append(uosPtr);

    auto ubuntuPtr = Compatible::RootfsInfo::Ptr::create();
    uosPtr->name = "ubuntu-18.04";
    rootfsList.append(ubuntuPtr);

    return rootfsList;
}

TEST_F(UTCompatibleJsonParser, parseSupportRootfsSuccess)
{
    auto ret = Compatible::CompatibleJsonParser::parseCommonField(kAppListJson);

    ASSERT_FALSE(ret.isNull());

    EXPECT_EQ(ret->code, 0);
    EXPECT_EQ(ret->ext.code, 0);
    EXPECT_TRUE(ret->message.isEmpty());

    auto rootfsList = Compatible::CompatibleJsonParser::parseSupportfsList(ret);
    ASSERT_TRUE(rootfsList.isEmpty());

    Stub stub;
    stub.set(ADDR(Compatible::CompatibleBackend, rootfsList), stub_rootfsList);

    rootfsList = Compatible::CompatibleJsonParser::parseSupportfsList(ret);
    ASSERT_EQ(rootfsList.size(), 2);
}
