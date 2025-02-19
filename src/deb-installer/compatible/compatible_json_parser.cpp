// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compatible_json_parser.h"
#include "compatible_backend.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace Compatible {

static const QString kFieldCode = "Code";
static const QString kFieldMsg = "Msg";
static const QString kFieldExt = "Ext";

CompatibleJsonParser::CompatibleJsonParser() {}

/**
   @brief parse deepin-compatible-ctl result

   @example
   {"Code":0,"Msg":null,"Ext":{"Code":0,"Msg":["成功 [\n
    {\n    \"name\": \"hello\",\n    \"version\": \"2.10-2\",\n    \"arch\": \"amd64\"\n  },\n
    {\n    \"name\": \"tree\",\n    \"version\": \"1.8.0-1\",\n    \"arch\": \"amd64\"\n  }\n]"]}}

 */
CompatibleRet::Ptr CompatibleJsonParser::parseCommonField(const QByteArray &jsonString)
{
    if (jsonString.isEmpty()) {
        return {};
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString, &err);
    if (QJsonParseError::NoError != err.error) {
        qWarning() << QString("Parse compatible result failed: %1 offset:%2").arg(err.errorString()).arg(err.offset);
        return {};
    }

    auto rootObj = doc.object();

    auto ret = CompatibleRet::Ptr::create();
    ret->code = rootObj.value(kFieldCode).toInt();
    ret->message = rootObj.value(kFieldMsg).toString();

    auto extObj = rootObj.value(kFieldExt).toObject();
    if (!extObj.isEmpty()) {
        ret->ext.code = extObj.value(kFieldCode).toInt();
        ret->ext.detailMessage = extObj.value(kFieldMsg);
    }

    return ret;
}

QHash<QString, CompPkgInfo::Ptr> CompatibleJsonParser::parseAppList(const CompatibleRet::Ptr &ret)
{
    if (ret.isNull() || CompError == ret->code) {
        return {};
    }

    auto array = ret->ext.detailMessage.toArray();
    if (array.empty()) {
        return {};
    }

    QString listJson = array.first().toString();
    // remove unnecessary prefix
    listJson.remove(QRegularExpression("^[^\[]*"));

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(listJson.toUtf8(), &err);
    if (QJsonParseError::NoError != err.error) {
        qWarning() << QString("Parse compatible result failed: %1 offset:%2").arg(err.errorString()).arg(err.offset);
        return {};
    }

    array = doc.array();
    QHash<QString, CompPkgInfo::Ptr> packages;
    for (const QJsonValue &item : array) {
        if (!item.isObject()) {
            continue;
        }
        auto obj = item.toObject();

        auto pkgPtr = CompPkgInfo::Ptr::create();
        pkgPtr->name = obj.value("name").toString();
        pkgPtr->version = obj.value("version").toString();
        pkgPtr->arch = obj.value("arch").toString();

        packages.insert(pkgPtr->name, pkgPtr);
    }

    return packages;
}

QList<RootfsInfo::Ptr> CompatibleJsonParser::parseRootfsList(const CompatibleRet::Ptr &ret)
{
    /* e.g.:
       {"Code":0,"Msg":null,"Ext":{"Code":0,"Msg":["成功 {\n  \"Containers\": [\n    {\n      \"ID\": \"4aeedabc79a4\",\n
       \"Name\":\"uos-rootfs-20\",\n      \"Status\": \"Up 15 minutes\",\n      \"Image\": \"localhost/uos-rootfs-20:latest\"\n
       }\n  ]\n}"]}}
     */

    if (ret.isNull() || CompError == ret->code) {
        return {};
    }

    auto array = ret->ext.detailMessage.toArray();
    if (array.empty()) {
        return {};
    }

    QString listJson = array.first().toString();
    // remove unnecessary prefix
    listJson.remove(QRegularExpression("^[^\[]*"));

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(listJson.toUtf8(), &err);
    if (QJsonParseError::NoError != err.error) {
        qWarning() << QString("Parse compatible result failed: %1 offset:%2").arg(err.errorString()).arg(err.offset);
        return {};
    }

    auto rootObj = doc.object();
    auto containerArray = rootObj.value("Containers").toArray();
    QList<RootfsInfo::Ptr> rootfs;

    for (const QJsonValue &item : containerArray) {
        if (!item.isObject()) {
            continue;
        }
        auto obj = item.toObject();

        auto rootfsPtr = RootfsInfo::Ptr::create();
        rootfsPtr->name = obj.value("Name").toString();
        rootfsPtr->osName = rootfsPtr->name;

        rootfs.append(rootfsPtr);
    }

    return rootfs;
}

QList<RootfsInfo::Ptr> CompatibleJsonParser::parseSupportfsList(const CompatibleRet::Ptr &ret)
{
    /*
       e.g.:
       {"Code":0,"Msg":null,"Ext":{"Code":0,"Msg":["成功 [\n \"uos-rootfs-20\",\n \"ubuntu-18.04\" \n]"]}}
       {"Code":0,"Msg":null,"Ext":{"Code":0,"Msg":["失败 [\n \"NOTFOUND\" \n]"]}}
     */

    if (ret.isNull() || CompError == ret->code) {
        return {};
    }

    auto array = ret->ext.detailMessage.toArray();
    if (array.empty()) {
        return {};
    }

    QString listJson = array.first().toString();
    // remove unnecessary prefix
    listJson.remove(QRegularExpression("^[^\[]*"));

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(listJson.toUtf8(), &err);
    if (QJsonParseError::NoError != err.error) {
        qWarning() << QString("Parse compatible result failed: %1 offset:%2").arg(err.errorString()).arg(err.offset);
        return {};
    }

    array = doc.array();
    QList<RootfsInfo::Ptr> supportRootfs;

    auto rootfsList = CompBackend::instance()->rootfsList();

    for (const QJsonValue &item : array) {
        QString itemStr = item.toString();

        auto findItr = std::find_if(
            rootfsList.begin(), rootfsList.end(), [&](const RootfsInfo::Ptr &rootfsPtr) { return rootfsPtr->name == itemStr; });
        if (findItr != rootfsList.end()) {
            supportRootfs.append(*findItr);
        }
    }

    return supportRootfs;
}

}  // namespace Compatible
