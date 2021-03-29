/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "debinfolabel.h"
#include "utils/utils.h"

#include <QPainter>
#include <QDebug>

DebInfoLabel::DebInfoLabel(QWidget *parent, Qt::WindowFlags f)
    : DLabel(parent, f)
    , m_colorRole(QPalette::NoRole)
    , m_colorType(DPalette::NoType)
    , m_bUserColorType(false)
{
    // 设置AccessibleName
    this->setAccessibleName("DebInfoLabel");
    this->setObjectName("DebInfoLabel");
}

/**
 * @brief DebInfoLabel::setCustomQPalette
 * @param colorRole 字体调色板的颜色
 * 设置自定的Qt风格 字体颜色，根据传入的颜色角色来对DLabel进行设置
 * 目前使用到此类型的有 singleInstallPage的packgeName packageVersion 以及他们提示的label
 */
void DebInfoLabel::setCustomQPalette(QPalette::ColorRole colorRole)
{
    m_colorRole = colorRole;                                                //获取当前使用QPalette的字体颜色角色
    m_bUserColorType = false;                                               //当前使用的不是DPalette

    // 判断传入的QPalette是否是WindowText 如果是则设置当前Label的Palette的画刷为DApplicationHelper画刷,颜色为WindowText
    // 否则设置当前的Label的Palette的画刷为DebApplicationHelperr画刷,颜色为传入的颜色
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
 * 目前使用到此类型的有 singleInstallPage的m_tipsLabel(单包安装时状态[依赖状态][安装状态][卸载状态][版本状态]提示)
 */
void DebInfoLabel::setCustomDPalette(DPalette::ColorType colorType)
{
    m_colorType = colorType;                                                //获取当前使用DPalette的字体颜色类型
    m_bUserColorType = true;                                                //当前使用的是DPalette
    m_bMultiIns = false;                                                    //当前使用的不是默认颜色

    //设置当前的Label的Palette的画刷为DebApplicationHelperr画刷,颜色为传入的颜色
    DPalette palette = DebApplicationHelper::instance()->palette(this);
    palette.setBrush(DPalette::WindowText, palette.color(m_colorType));
    this->setPalette(palette);
}

/**
 * @brief DebInfoLabel::setCustomDPalette
 * 设置当前DLabel 为默认的风格
 * 目前没有控件在使用
 */
void DebInfoLabel::setCustomDPalette()
{
    m_bMultiIns = true;                                                     //当前使用的是默认的颜色
    m_bUserColorType = true;                                                //当前是用的是DPalette

    //设置当前的Label的Palette的画刷为DApplicationHelper画刷,颜色为默认的颜色
    DPalette palette = DApplicationHelper::instance()->palette(this);
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
        // 当前使用的是DPalette
        if (m_bMultiIns) {
            // 当前使用的是默认颜色
            DPalette palette = DebApplicationHelper::instance()->palette(&tmpWidget);
            QPainter painter(this);
            painter.setPen(QColor(00, 130, 252));
            painter.drawText(contentsRect(), static_cast<int>(alignment()), text());
            QWidget::paintEvent(event);
        } else {
            // 当前使用的是自定义颜色
            DPalette palette = DebApplicationHelper::instance()->palette(&tmpWidget);
            QPainter painter(this);
            painter.setPen(QColor(palette.color(m_colorType)));
            painter.drawText(contentsRect(), static_cast<int>(alignment()), text());
            QWidget::paintEvent(event);
        }
    } else {
        // 当前使用的是QPalette
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
