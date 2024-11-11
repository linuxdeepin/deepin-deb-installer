// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "immutable_backend.h"

#include <QProcess>

namespace Immutable {

static const QString kImmutableBin = "deepin-immutable-ctl";
static const QString kImmutableStatus = "-s";
static const QByteArray kImmutableEnable = "immutable-mode:true";

ImmutableBackend::ImmutableBackend(QObject *parent)
    : QObject{parent}
{
    initBackend();
}

ImmutableBackend *ImmutableBackend::instance()
{
    static ImmutableBackend ins;
    return &ins;
}

bool ImmutableBackend::immutableEnabled() const
{
    return m_immutableEnabled;
}

void ImmutableBackend::initBackend()
{
    QProcess process;
    process.setProgram(kImmutableBin);
    process.setArguments({kImmutableStatus});
    process.start();
    process.waitForFinished();
    const QByteArray output = process.readAllStandardOutput();

    m_immutableEnabled = output.contains(kImmutableEnable);
}

};  // namespace Immutable
