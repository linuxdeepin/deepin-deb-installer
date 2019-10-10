/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
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

#include "infocontrolbutton.h"

#include <QVBoxLayout>
#include <QPixmap>
#include <QIcon>

#include <DStyleHelper>
#include <DApplicationHelper>
InfoControlButton::InfoControlButton(const QString &expandTips, const QString &shrinkTips, DWidget *parent)
    : DWidget(parent),
      m_expand(false),
      m_expandTips(expandTips),
      m_shrinkTips(shrinkTips),
      m_arrowIcon(new DLabel),
      m_tipsText(new DLabel)
{

    m_font.setPixelSize(12);

    DPalette palette = DApplicationHelper::instance()->palette(m_tipsText);
    palette.setColor(DPalette::WindowText, palette.color(DPalette::Highlight));
    m_tipsText->setPalette(palette);

    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    m_arrowIcon->setAlignment(Qt::AlignCenter);
    if(themeType == DGuiApplicationHelper::LightType)
        m_arrowIcon->setPixmap(QIcon(":/images/arrow_up.svg").pixmap(25, 8));//21.8
    else if(themeType == DGuiApplicationHelper::DarkType)
        m_arrowIcon->setPixmap(QIcon(":/images/arrow_up_dark.svg").pixmap(25, 8));//21.8
    else
        m_arrowIcon->setPixmap(QIcon(":/images/arrow_up.svg").pixmap(25, 8));//21.8
    m_tipsText->setAlignment(Qt::AlignCenter);
    m_tipsText->setFont(m_font);
    m_tipsText->setText(expandTips);
    m_tipsText->setObjectName("TipsText");

    centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_arrowIcon);
    centralLayout->addWidget(m_tipsText);
    centralLayout->setSpacing(10);
    centralLayout->setContentsMargins(0, 0, 0, 5);

    setLayout(centralLayout);
    //setFixedSize(200, 31);

    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                        this, &InfoControlButton::themeChanged);
}

void InfoControlButton::mouseReleaseEvent(QMouseEvent *e)
{
    DWidget::mouseReleaseEvent(e);

    onMouseRelease();
}

void InfoControlButton::onMouseRelease()
{
    if (m_expand)
        emit shrink();
    else
        emit expand();

    m_expand = !m_expand;

    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();

    centralLayout->removeWidget(m_arrowIcon);
    centralLayout->removeWidget(m_tipsText);
    if (!m_expand) {
        centralLayout->addWidget(m_arrowIcon);
        centralLayout->addWidget(m_tipsText);
        if(themeType == DGuiApplicationHelper::LightType)
            m_arrowIcon->setPixmap(QIcon(":/images/arrow_up.svg").pixmap(25, 8));
        else if(themeType == DGuiApplicationHelper::DarkType)
            m_arrowIcon->setPixmap(QIcon(":/images/arrow_up_dark.svg").pixmap(25, 8));
        else
            m_arrowIcon->setPixmap(QIcon(":/images/arrow_up.svg").pixmap(25, 8));
        m_tipsText->setText(m_expandTips);
    } else {
        centralLayout->addWidget(m_tipsText);
        centralLayout->addWidget(m_arrowIcon);
        if(themeType == DGuiApplicationHelper::LightType)
            m_arrowIcon->setPixmap(QIcon(":/images/arrow_down.svg").pixmap(25, 8));
        else if(themeType == DGuiApplicationHelper::DarkType)
            m_arrowIcon->setPixmap(QIcon(":/images/arrow_down_dark.svg").pixmap(25, 8));
        else
            m_arrowIcon->setPixmap(QIcon(":/images/arrow_down.svg").pixmap(25, 8));
        m_tipsText->setText(m_shrinkTips);
    }
}
void InfoControlButton::setShowText(const QString text)
{
    m_expandTips = text;
    m_tipsText->setText(m_expandTips);
}
void InfoControlButton::themeChanged()
{
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if(m_expand)
    {
        if(themeType == DGuiApplicationHelper::LightType)
            m_arrowIcon->setPixmap(QIcon(":/images/arrow_down.svg").pixmap(25, 8));
        else if(themeType == DGuiApplicationHelper::DarkType)
            m_arrowIcon->setPixmap(QIcon(":/images/arrow_down_dark.svg").pixmap(25, 8));
        else
            m_arrowIcon->setPixmap(QIcon(":/images/arrow_down.svg").pixmap(25, 8));
    }
    else
    {
        if(themeType == DGuiApplicationHelper::LightType)
            m_arrowIcon->setPixmap(QIcon(":/images/arrow_up.svg").pixmap(25, 8));
        else if(themeType == DGuiApplicationHelper::DarkType)
            m_arrowIcon->setPixmap(QIcon(":/images/arrow_up_dark.svg").pixmap(25, 8));
        else
            m_arrowIcon->setPixmap(QIcon(":/images/arrow_up.svg").pixmap(25, 8));
    }
}
