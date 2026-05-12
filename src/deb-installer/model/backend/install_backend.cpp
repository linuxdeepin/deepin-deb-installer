// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "install_backend.h"

#include "model/deblistmodel.h"

InstallBackend::InstallBackend(DebListModel *model, QObject *parent)
    : QObject{parent}
    , m_model{model}
{
}
