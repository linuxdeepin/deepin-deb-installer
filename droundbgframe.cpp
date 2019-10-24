#include "droundbgframe.h"
#include "utils.h"

#include <QPainter>
#include <DApplicationHelper>

DRoundBgFrame::DRoundBgFrame(QWidget* parent, bool hasFrameBoarder)
    :DFrame (parent)
    , m_hasFrameBorder(hasFrameBoarder)
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
        rect.setY(0.5);
        rect.setWidth(rect.width()-0.5);
        rect.setHeight(rect.height()-0.5);

        painterPath.addRoundedRect(rect, 8, 8);
        painter.drawPath(painterPath);

        //Restore the pen
        painter.setPen(oldPen);
    }
    else {
        painterPath.addRoundedRect(this->rect(), 8, 8);
        painter.fillPath(painterPath, QBrush(pa.color(DPalette::Base)));
    }
}
