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
#include "utils.h"

#include <QVBoxLayout>
#include <QPixmap>
#include <QIcon>

#include <DStyleHelper>
#include <DApplicationHelper>

InfoControlButton::InfoControlButton(const QString &expandTips, const QString &shrinkTips, DWidget *parent)
    : DWidget(parent)
    , m_expand(false)
    , m_expandTips(expandTips)
    , m_shrinkTips(shrinkTips)
    , m_arrowIcon(new DLabel)
    , m_tipsText(new DLabel)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QString normalFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    QFont font = Utils::loadFontBySizeAndWeight(normalFontFamily, 12, QFont::ExtraLight);

    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    m_arrowIcon->setAlignment(Qt::AlignCenter);
    if(themeType == DGuiApplicationHelper::LightType) {
        m_arrowIcon->setPixmap(Utils::renderSVG(":/images/arrow_up.svg", QSize(25, 8)));
    }
    else if(themeType == DGuiApplicationHelper::DarkType) {
        m_arrowIcon->setPixmap(Utils::renderSVG(":/images/arrow_up_dark.svg", QSize(25, 8)));
    }
    else {
        m_arrowIcon->setPixmap(Utils::renderSVG(":/images/arrow_up.svg", QSize(25, 8)));
    }
    m_arrowIcon->setFixedHeight(13);

    DPalette palette = DApplicationHelper::instance()->palette(m_tipsText);
    palette.setColor(DPalette::WindowText, palette.color(DPalette::TextLively));
    m_tipsText->setPalette(palette);
    m_tipsText->setAlignment(Qt::AlignCenter);
    m_tipsText->setFont(font);
    m_tipsText->setText(expandTips);
    m_tipsText->setFixedHeight(15);

    centralLayout = new QVBoxLayout;
    centralLayout->setSpacing(5);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->addWidget(m_arrowIcon);
    centralLayout->addWidget(m_tipsText);

    setLayout(centralLayout);

    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                        this, &InfoControlButton::themeChanged);

//#define SHOWBGCOLOR
#ifdef SHOWBGCOLOR
    m_tipsText->setStyleSheet("QLabel{background: cyan;}");
    m_arrowIcon->setStyleSheet("QLabel{background: red;}");
#endif
}

void InfoControlButton::mouseReleaseEvent(QMouseEvent *e)
{
    DWidget::mouseReleaseEvent(e);

    onMouseRelease();
}

void InfoControlButton::onMouseRelease()
{
    if (m_expand) {
        emit shrink();
    } else {
        emit expand();
    }

    m_expand = !m_expand;

    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();

    centralLayout->removeWidget(m_arrowIcon);
    centralLayout->removeWidget(m_tipsText);
    if (!m_expand) {
        centralLayout->setSpacing(5);
        centralLayout->addWidget(m_arrowIcon);
        centralLayout->addWidget(m_tipsText);
        if(themeType == DGuiApplicationHelper::LightType) {
            m_arrowIcon->setPixmap(Utils::renderSVG(":/images/arrow_up.svg", QSize(25, 8)));
        }
        else if(themeType == DGuiApplicationHelper::DarkType) {
            m_arrowIcon->setPixmap(Utils::renderSVG(":/images/arrow_up_dark.svg", QSize(25, 8)));
        }
        else {
            m_arrowIcon->setPixmap(Utils::renderSVG(":/images/arrow_up.svg", QSize(25, 8)));
        }
        m_tipsText->setText(m_expandTips);
    } else {
        centralLayout->setSpacing(0);
        centralLayout->addWidget(m_tipsText);
        centralLayout->addWidget(m_arrowIcon);
        if(themeType == DGuiApplicationHelper::LightType) {
            m_arrowIcon->setPixmap(Utils::renderSVG(":/images/arrow_down.svg", QSize(25, 8)));
        }
        else if(themeType == DGuiApplicationHelper::DarkType) {
            m_arrowIcon->setPixmap(Utils::renderSVG(":/images/arrow_down_dark.svg", QSize(25, 8)));
        }
        else {
            m_arrowIcon->setPixmap(Utils::renderSVG(":/images/arrow_down.svg", QSize(25, 8)));
        }
        m_tipsText->setText(m_shrinkTips);
    }
}

void InfoControlButton::setExpandTips(const QString text)
{
    m_expandTips = text;
    m_tipsText->setText(m_expandTips);
}

void InfoControlButton::setShrinkTips(const QString text)
{
    m_shrinkTips = text;
    m_tipsText->setText(m_shrinkTips);
}

void InfoControlButton::themeChanged()
{
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if(m_expand) {
        if(themeType == DGuiApplicationHelper::LightType) {
            m_arrowIcon->setPixmap(Utils::renderSVG(":/images/arrow_down.svg", QSize(25, 8)));
        }
        else if(themeType == DGuiApplicationHelper::DarkType) {
            m_arrowIcon->setPixmap(Utils::renderSVG(":/images/arrow_down_dark.svg", QSize(25, 8)));
        }
        else {
            m_arrowIcon->setPixmap(Utils::renderSVG(":/images/arrow_down.svg", QSize(25, 8)));
        }
    } else {
        if(themeType == DGuiApplicationHelper::LightType) {
            m_arrowIcon->setPixmap(Utils::renderSVG(":/images/arrow_up.svg", QSize(25, 8)));
        }
        else if(themeType == DGuiApplicationHelper::DarkType) {
            m_arrowIcon->setPixmap(Utils::renderSVG(":/images/arrow_up_dark.svg", QSize(25, 8)));
        }
        else {
            m_arrowIcon->setPixmap(Utils::renderSVG(":/images/arrow_up.svg", QSize(25, 8)));
        }
    }
}
