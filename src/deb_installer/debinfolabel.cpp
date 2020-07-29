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

/**
 * @brief DebInfoLabel::setCustomQPalette
 * @param colorRole 字体调色板的颜色
 * 设置自定的Qt风格 字体颜色，根据传入的颜色角色来对DLabel进行设置
 */
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

/**
 * @brief DebInfoLabel::setCustomDPalette
 * @param colorType 字体调色板的颜色
 * 设置自定的Deepin字体风格，根据传入的颜色角色来对DLabel进行设置
 */
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

/**
 * @brief DebInfoLabel::setCustomDPalette
 * 设置当前DLabel 为默认的风格
 */
void DebInfoLabel::setCustomDPalette()
{
    m_bMultiIns = true;
    m_bUserColorType = true;
    DPalette palette = DApplicationHelper::instance()->palette(this);
    //    palette.setColor(DPalette::ButtonText, palette.color(DPalette::TextLively));
    palette.setBrush(DPalette::WindowText, QColor(00, 130, 252));
    this->setPalette(palette);
}

/**
 * @brief DebInfoLabel::paintEvent
 * @param event
 * 根据调色板设置进行重绘
 */
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
