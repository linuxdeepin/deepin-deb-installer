// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workerprogress.h"
#include "utils/ddlog.h"

WorkerProgress::WorkerProgress(QWidget *parent)
    : DProgressBar(parent)
{
    qCDebug(appLog) << "Initializing WorkerProgress";
    // 进度条在为设置初始值时，值为-1 安装进度条前面一段显示为方头
    setValue(0);  // 设置初始进度为0
    setMinimum(0);
    setMaximum(100);
    setFixedHeight(8);
    setFixedWidth(240);
    setTextVisible(false);
    qCDebug(appLog) << "WorkerProgress initialized with size 240x8, range 0-100";
}
