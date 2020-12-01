#include "droundbgframe.h"
#include "utils/utils.h"

#include <DStyleHelper>
#include <DApplicationHelper>

#include <QPainter>
#include <QModelIndex>
#include <QDebug>
#include <QPainterPath>

DRoundBgFrame::DRoundBgFrame(QWidget *parent, int bgOffsetTop, int bgOffsetBottom)
    : QWidget(parent)
    , m_bgOffsetTop(bgOffsetTop)
    , m_bgOffsetBottom(bgOffsetBottom)
{
    // 添加accessibleName
    this->setObjectName("DRoundBgFrame");
    this->setAccessibleName("DRoundBgFrame");
}

/**
 * @brief DRoundBgFrame::paintEvent 重绘
 * @param event
 */
void DRoundBgFrame::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);            //设置边缘抗锯齿
    DPalette pa = DebApplicationHelper::instance()->palette(this);

    QPainterPath path;
    QRectF rect = this->rect();
    rect.setX(0);                                                   //设置x位置
    rect.setY(m_bgOffsetTop);                                       //设置y位置
    rect.setWidth(rect.width());                                    //设置宽度
    rect.setHeight(rect.height() - m_bgOffsetBottom);               //设置高度

    path.addRoundedRect(rect, 8, 8);
    painter.fillPath(path, QBrush(pa.color(DPalette::Base)));       //填充颜色
}
