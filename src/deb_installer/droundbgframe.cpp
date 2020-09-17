#include "droundbgframe.h"
#include "utils.h"

#include <DStyleHelper>
#include <DApplicationHelper>

#include <QPainter>
#include <QModelIndex>
#include <QDebug>

DRoundBgFrame::DRoundBgFrame(QWidget *parent, int bgOffsetTop, int bgOffsetBottom)
    : QWidget(parent)
    , m_bgOffsetTop(bgOffsetTop)
    , m_bgOffsetBottom(bgOffsetBottom)
{
    this->setObjectName("DRoundBgFrame");
    this->setAccessibleName("DRoundBgFrame");
}

void DRoundBgFrame::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    DPalette pa = DebApplicationHelper::instance()->palette(this);

    QPainterPath path;
    QRectF rect = this->rect();
    rect.setX(0);
    rect.setY(m_bgOffsetTop);
    rect.setWidth(rect.width());
    rect.setHeight(rect.height() - m_bgOffsetBottom);

    path.addRoundedRect(rect, 8, 8);
    painter.fillPath(path, QBrush(pa.color(DPalette::Base)));
}
