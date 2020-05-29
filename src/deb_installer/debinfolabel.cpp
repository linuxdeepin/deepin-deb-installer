#include "debinfolabel.h"

#include <QPainter>
#include <QDebug>

DebInfoLabel::DebInfoLabel(QWidget *parent, Qt::WindowFlags f)
    : DLabel(parent, f)
    , m_colorRole(QPalette::NoRole)
    , m_colorType(DPalette::NoType)
    , m_bUserColorType(false)
{
}


void DebInfoLabel::setCustomQPalette(QPalette::ColorRole colorRole)
{
    m_colorRole = colorRole;
    m_bUserColorType = false;

    qDebug() << "setCustomQPalette m_colorRole:" << m_colorRole << endl;
    if (m_colorRole == QPalette::WindowText) {
        DPalette palette = DApplicationHelper::instance()->palette(this);
        palette.setBrush(QPalette::Text, palette.color(QPalette::WindowText));
        this->setPalette(palette);
    } else {
        DPalette palette = DebApplicationHelper::instance()->palette(this);
        palette.setBrush(DPalette::Text, palette.color(m_colorRole));
        this->setPalette(palette);
    }
}

void DebInfoLabel::setCustomDPalette(DPalette::ColorType colorType)
{
    m_colorType = colorType;
    m_bUserColorType = true;
    m_bMultiIns = false;

    qDebug() << "setCustomDPalette m_colorType:" << colorType << endl;
    DPalette palette = DebApplicationHelper::instance()->palette(this);
    palette.setBrush(DPalette::WindowText, palette.color(m_colorType));
    this->setPalette(palette);
}

void DebInfoLabel::setCustomDPalette()
{
    m_bMultiIns = true;
    m_bUserColorType = true;
    DPalette palette = DApplicationHelper::instance()->palette(this);
    //    palette.setColor(DPalette::ButtonText, palette.color(DPalette::TextLively));
    palette.setBrush(DPalette::WindowText, QColor(00, 130, 252));
    this->setPalette(palette);
}

void DebInfoLabel::paintEvent(QPaintEvent *event)
{
    QWidget tmpWidget;

    if (m_bUserColorType) {
        if (m_bMultiIns) {
            DPalette palette = DebApplicationHelper::instance()->palette(&tmpWidget);
            QPainter painter(this);
            painter.setPen(QColor(00, 130, 252));
            painter.drawText(contentsRect(), static_cast<int>(alignment()), text());
            QWidget::paintEvent(event);
        } else {
            DPalette palette = DebApplicationHelper::instance()->palette(&tmpWidget);
            QPainter painter(this);
            painter.setPen(QColor(palette.color(m_colorType)));
            painter.drawText(contentsRect(), static_cast<int>(alignment()), text());
            QWidget::paintEvent(event);
        }
    } else {
        if (m_colorRole == QPalette::WindowText) {
            DLabel::paintEvent(event);
        } else {
            DPalette palette = DebApplicationHelper::instance()->palette(&tmpWidget);
            QPainter painter(this);
            painter.setPen(QColor(palette.color(m_colorRole)));
            painter.drawText(contentsRect(), static_cast<int>(alignment()), text());
            QWidget::paintEvent(event);
        }
    }

}
