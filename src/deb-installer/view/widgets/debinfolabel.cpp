// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "debinfolabel.h"
#include "utils/utils.h"
#include "utils/ddlog.h"

#include <QPainter>
#include <QDebug>

#include <DPaletteHelper>

DebInfoLabel::DebInfoLabel(QWidget *parent, Qt::WindowFlags windowFlags)
    : DLabel(parent, windowFlags)
    , m_colorRole(QPalette::NoRole)
    , m_colorType(DPalette::NoType)
    , m_bUserColorType(false)
{
    qCDebug(appLog) << "Initializing DebInfoLabel...";
    // 设置AccessibleName
    this->setAccessibleName("DebInfoLabel");
    this->setObjectName("DebInfoLabel");
    qCDebug(appLog) << "DebInfoLabel initialized";
}

void DebInfoLabel::setCustomQPalette(QPalette::ColorRole colorRole)
{
    qCDebug(appLog) << "Setting QPalette with color role:" << colorRole;
    m_colorRole = colorRole;   // 获取当前使用QPalette的字体颜色角色
    m_bUserColorType = false;  // 当前使用的不是DPalette

    // 判断传入的QPalette是否是WindowText 如果是则设置当前Label的Palette的画刷为DGuiApplicationHelper画刷,颜色为WindowText
    // 否则设置当前的Label的Palette的画刷为DebApplicationHelperr画刷,颜色为传入的颜色
    if (m_colorRole == QPalette::WindowText) {
        qCDebug(appLog) << "Using WindowText color from DPalette";
        DPalette palette = DPaletteHelper::instance()->palette(this);
        palette.setBrush(QPalette::Text, palette.color(QPalette::WindowText));
        this->setPalette(palette);
    } else {
        qCDebug(appLog) << "Using custom color from DebApplicationHelper";
        DPalette palette = DebApplicationHelper::instance()->palette(this);
        palette.setBrush(DPalette::Text, palette.color(m_colorRole));
        this->setPalette(palette);
    }
}

void DebInfoLabel::setCustomDPalette(DPalette::ColorType colorType)
{
    qCDebug(appLog) << "Setting DPalette with color type:" << colorType;
    m_colorType = colorType;  // 获取当前使用DPalette的字体颜色类型
    m_bUserColorType = true;  // 当前使用的是DPalette
    m_bMultiIns = false;      // 当前使用的不是默认颜色

    // 设置当前的Label的Palette的画刷为DebApplicationHelperr画刷,颜色为传入的颜色
    DPalette palette = DebApplicationHelper::instance()->palette(this);
    palette.setBrush(DPalette::WindowText, palette.color(m_colorType));
    this->setPalette(palette);
    qCDebug(appLog) << "DPalette set with color:" << palette.color(m_colorType).name();
}

void DebInfoLabel::setCustomDPalette()
{
    m_bMultiIns = true;       // 当前使用的是默认的颜色
    m_bUserColorType = true;  // 当前是用的是DPalette

    // 设置当前的Label的Palette的画刷为DGuiApplicationHelper画刷,颜色为默认的颜色
    DPalette palette = DPaletteHelper::instance()->palette(this);
    palette.setBrush(DPalette::WindowText, QColor(00, 130, 252));
    this->setPalette(palette);
}

void DebInfoLabel::setTextAndTips(const QString &text)
{
    qCDebug(appLog) << "Setting text and tooltip:" << text;
    setText(text);
    m_toolTip = text;
    updateTipText();
}

void DebInfoLabel::paintEvent(QPaintEvent *event)
{
    QWidget tmpWidget;

    if (m_bUserColorType) {
        // 当前使用的是DPalette
        if (m_bMultiIns) {
            qCDebug(appLog) << "Using default color (00, 130, 252)";
            DPalette palette = DebApplicationHelper::instance()->palette(&tmpWidget);
            QPainter painter(this);
            painter.setPen(QColor(00, 130, 252));
            painter.drawText(contentsRect(), static_cast<int>(alignment()), paintText());
            QWidget::paintEvent(event);
        } else {
            qCDebug(appLog) << "Using custom DPalette color type:" << m_colorType;
            DPalette palette = DebApplicationHelper::instance()->palette(&tmpWidget);
            QPainter painter(this);
            painter.setPen(QColor(palette.color(m_colorType)));
            painter.drawText(contentsRect(), static_cast<int>(alignment()), paintText());
            QWidget::paintEvent(event);
        }
    } else {
        // 当前使用的是QPalette
        if (m_colorRole == QPalette::WindowText) {
            qCDebug(appLog) << "Using WindowText from QPalette";
            DLabel::paintEvent(event);
        } else {
            qCDebug(appLog) << "Using custom QPalette color role:" << m_colorRole;
            DPalette palette = DebApplicationHelper::instance()->palette(&tmpWidget);
            QPainter painter(this);
            painter.setPen(QColor(palette.color(m_colorRole)));
            painter.drawText(contentsRect(), static_cast<int>(alignment()), paintText());
            QWidget::paintEvent(event);
        }
    }
}

QString DebInfoLabel::paintText()
{
    // update tips while repaint
    updateTipText();

    QString paintString = text();
    if (Qt::ElideNone != elideMode()) {
        return fontMetrics().elidedText(paintString, elideMode(), width());
    }

    return paintString;
}

/**
 * @brief Default tooltip not provides wordwrap, use QTextDocument layout instead.
 */
void DebInfoLabel::updateTipText()
{
    if (m_toolTip.isEmpty()) {
        setToolTip(m_toolTip);
        return;
    }

    QString tipsText = Utils::formatWrapText(m_toolTip, width());
    setToolTip(tipsText);
}
