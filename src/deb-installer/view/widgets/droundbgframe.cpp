// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "droundbgframe.h"
#include "utils/utils.h"
#include "utils/ddlog.h"

#include <DStyleHelper>
#include <DGuiApplicationHelper>

#include <QPainter>
#include <QPainterPath>
#include <QModelIndex>
#include <QDebug>

DRoundBgFrame::DRoundBgFrame(QWidget *parent, int bgOffsetTop, int bgOffsetBottom)
    : QWidget(parent)
    , m_bgOffsetTop(bgOffsetTop)
    , m_bgOffsetBottom(bgOffsetBottom)
{
    qCDebug(appLog) << "Initializing DRoundBgFrame with offsets - top:" << bgOffsetTop << "bottom:" << bgOffsetBottom;
    // 添加accessibleName
    this->setObjectName("DRoundBgFrame");
    this->setAccessibleName("DRoundBgFrame");
    qCDebug(appLog) << "DRoundBgFrame initialized";
}

void DRoundBgFrame::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    qCDebug(appLog) << "Painting DRoundBgFrame";

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);  // 设置边缘抗锯齿
    DPalette pa = DebApplicationHelper::instance()->palette(this);
    qCDebug(appLog) << "Using palette color:" << pa.color(DPalette::Base).name();

    QPainterPath path;
    QRectF rect = this->rect();
    rect.setX(0);                                      // 设置x位置
    rect.setY(m_bgOffsetTop);                          // 设置y位置
    rect.setWidth(rect.width());                       // 设置宽度
    rect.setHeight(rect.height() - m_bgOffsetBottom);  // 设置高度
    qCDebug(appLog) << "Painting rounded rect at:" << rect << "with radius 8";

    path.addRoundedRect(rect, 8, 8);
    painter.fillPath(path, QBrush(pa.color(DPalette::Base)));  // 填充颜色
}
