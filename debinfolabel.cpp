#include "debinfolabel.h"

#include <QPainter>
#include <QDebug>

DebInfoLabel::DebInfoLabel(QWidget *parent, Qt::WindowFlags f)
    :DLabel(parent, f)
{
}


void DebInfoLabel::setCustomPalette(QPalette::ColorRole colorRole)
{
    m_colorRole = colorRole;

    DPalette palette = DebApplicationHelper::instance()->palette(this);
    palette.setBrush(DPalette::WindowText, palette.color(colorRole));
    this->setPalette(palette);
}

void DebInfoLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    DWidget tmpWidget;
    DPalette palette = DebApplicationHelper::instance()->palette(&tmpWidget);
    QPainter painter(this);
    painter.setPen(QColor(palette.color(m_colorRole)));

    painter.drawText(contentsRect(), static_cast<int>(alignment()), text());
}
