// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QTCOMPAT_H
#define QTCOMPAT_H

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QRegularExpression>
#include <QRegularExpressionValidator>
class QEnterEvent;

#define SKIP_EMPTY_PARTS    Qt::SkipEmptyParts
#define SPLIT_BH            Qt::SplitBehavior
#define ENDL                Qt::endl
#define REG_EXP             QRegularExpression
#define REG_EXPV            QRegularExpressionValidator
#define DATE_FOTIME         QDateTime::fromSecsSinceEpoch
#define DATE_TOTIME         QDateTime::toSecsSinceEpoch

using EnterEvent = QEnterEvent;

#else // QT_VERSION < 6.0.0

#include <QRegExp>
class QEvent;

#define SKIP_EMPTY_PARTS    QString::SkipEmptyParts
#define SPLIT_BH            QString::SplitBehavior
#define ENDL                endl
#define REG_EXP             QRegExp
#define REG_EXPV            QRegExpValidator
#define DATE_FOTIME         QDateTime::fromTime_t
#define DATE_TOTIME         QDateTime::toTime_t

using EnterEvent = QEvent;

#endif

#endif // QTCOMPAT_H
