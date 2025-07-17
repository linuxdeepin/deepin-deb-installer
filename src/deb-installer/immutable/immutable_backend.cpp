// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "immutable_backend.h"
#include "utils/ddlog.h"

#include <QProcess>

namespace Immutable {

static const QString kImmutableBin = "deepin-immutable-ctl";
static const QString kImmutableStatus = "-s";
// return e.g. : "immutable mode:[true|false]"
static const QByteArray kImmutableEnable = "true";

ImmutableBackend::ImmutableBackend(QObject *parent)
    : QObject{parent}
{
    qCDebug(appLog) << "Constructing ImmutableBackend";
    initBackend();
}

ImmutableBackend *ImmutableBackend::instance()
{
    // qCDebug(appLog) << "Getting ImmutableBackend instance";
    static ImmutableBackend ins;
    return &ins;
}

bool ImmutableBackend::immutableEnabled() const
{
    qCDebug(appLog) << "Checking if immutable is enabled:" << m_immutableEnabled;
    return m_immutableEnabled;
}

void ImmutableBackend::initBackend()
{
    qCDebug(appLog) << "initBackend";
    QProcess process;
    process.setProgram(kImmutableBin);
    process.setArguments({kImmutableStatus});
    process.start();
    process.waitForFinished();
    const QByteArray output = process.readAllStandardOutput();

    m_immutableEnabled = output.contains(kImmutableEnable);
    qCDebug(appLog) << "end initBackend, immutableEnabled:" << m_immutableEnabled;
}

};  // namespace Immutable
