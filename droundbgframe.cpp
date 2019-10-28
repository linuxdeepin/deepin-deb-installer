#include "droundbgframe.h"
#include "utils.h"

#include <QPainter>
#include <QDebug>
#include <DApplicationHelper>

DRoundBgFrame::DRoundBgFrame(QWidget* parent, bool hasFrameBoarder, int bgOffsetTop, int bgOffsetBottom)
    :DWidget (parent)
    , m_hasFrameBorder(hasFrameBoarder)
    , m_bgOffsetTop(bgOffsetTop)
    , m_bgOffsetBottom(bgOffsetBottom)
{
}

void DRoundBgFrame::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    DPalette pa = DebApplicationHelper::instance()->palette(this);
    QPainterPath painterPath;

    if(m_hasFrameBorder) {

        //Save pen
        QPen oldPen = painter.pen();

        painter.setBrush(QBrush(pa.color(DPalette::Base)));
        QColor penColor = pa.color(DPalette::FrameBorder);
        penColor.setAlphaF(0.05);
        painter.setPen(QPen(penColor));

        QRectF rect = this->rect();
        rect.setX(0.5);
        rect.setY(m_bgOffsetTop+0.5);
        rect.setWidth(rect.width()-0.5);
        rect.setHeight(rect.height()-m_bgOffsetBottom-0.5);

        painterPath.addRoundedRect(rect, 8, 8);
        painter.drawPath(painterPath);

        //Restore the pen
        painter.setPen(oldPen);
    }
    else {
        QRectF rect = this->rect();
        rect.setX(0);
        rect.setY(m_bgOffsetTop);
        rect.setWidth(rect.width());
        rect.setHeight(rect.height()-m_bgOffsetBottom);

        painterPath.addRoundedRect(rect, 8, 8);
        painter.fillPath(painterPath, QBrush(pa.color(DPalette::Base)));
    }
}
