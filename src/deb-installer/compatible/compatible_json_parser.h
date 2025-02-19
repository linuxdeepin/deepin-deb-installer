// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPATIBLE_JSON_PARSER_H
#define COMPATIBLE_JSON_PARSER_H

#include <QJsonValue>

#include "compatible_defines.h"

namespace Compatible {

struct CompatibleRet
{
    typedef QSharedPointer<CompatibleRet> Ptr;

    int code{0};
    QString message;  // short message
    struct Ext
    {
        int code{0};
        QJsonValue detailMessage;
    } ext;  // detail info
};

class CompatibleJsonParser
{
public:
    CompatibleJsonParser();

    static CompatibleRet::Ptr parseCommonField(const QByteArray &jsonString);
    static QHash<QString, CompPkgInfo::Ptr> parseAppList(const CompatibleRet::Ptr &ret);
    static QList<RootfsInfo::Ptr> parseRootfsList(const CompatibleRet::Ptr &ret);
    static QList<RootfsInfo::Ptr> parseSupportfsList(const CompatibleRet::Ptr &ret);
};

}  // namespace Compatible

#endif  // COMPATIBLE_JSON_PARSER_H
