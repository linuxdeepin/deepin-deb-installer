/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Chris Xiong <chirs241097@gmail.com>
 *
 * Maintainer: Chris Xiong <chirs241097@gmail.com>
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

#include "coloredprogressbar.h"

#include <DObjectPrivate>

#include <QMap>
#include <QPainter>
#include <QDebug>
#include <QStylePainter>
#include <QStyleOptionProgressBar>

class ColoredProgressBarPrivate : DTK_CORE_NAMESPACE::DObjectPrivate
{
    D_DECLARE_PUBLIC(ColoredProgressBar)
    ColoredProgressBarPrivate(ColoredProgressBar *q);
private:
    QMap<int, QBrush> threshmap;
};


ColoredProgressBarPrivate::ColoredProgressBarPrivate(ColoredProgressBar *q)
    : DObjectPrivate(q)
{
}

/*!
 * \class ColoredProgressBar
 * \brief ColoredProgressBar is the same as QProgressBar, except it can change its appearance depending on the value displayed.
 */

ColoredProgressBar::ColoredProgressBar(QWidget *parent)
    : QProgressBar(parent)
    , DObject(*new ColoredProgressBarPrivate(this))
{
    themeChanged();
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this, &ColoredProgressBar::themeChanged);
}

void ColoredProgressBar::themeChanged()
{
    D_D(ColoredProgressBar);

    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    m_themeType = themeType;
    if (themeType == DGuiApplicationHelper::LightType) {
        d->threshmap.clear();
    } else if (themeType == DGuiApplicationHelper::DarkType) {
        DPalette pa = DebApplicationHelper::instance()->palette(this);

        QColor colorStart = pa.color(QPalette::Highlight);
        QColor colorEnd = pa.color(QPalette::HighlightedText);

        d->threshmap.insert(0, QBrush(colorStart));
        d->threshmap.insert(1, QBrush(colorEnd));
    } else {
        d->threshmap.clear();
    }
}

/*!
 * \brief ColoredProgressBar::addThreshold adds a new threshold value and specifies the brush to use once that value is reached.
 * If a threshold of the same value already exists, it will be overwritten.
 * \param threshold Minimum value for this brush to be used.
 * \param brush The brush to use when the currently displayed value is no less than \threshold and less than the next threshold value.
 */
void ColoredProgressBar::addThreshold(int threshold, QBrush brush)
{
    D_D(ColoredProgressBar);
    d->threshmap[threshold] = brush;
}

/*!
 * \brief ColoredProgressBar::removeThreshold removes a threshold.
 * \param threshold The threshold value to remove.
 */
void ColoredProgressBar::removeThreshold(int threshold)
{
    D_D(ColoredProgressBar);
    if (d->threshmap.contains(threshold)) {
        d->threshmap.remove(threshold);
    }
}

/*!
 * \brief ColoredProgressBar::threadsholds gets all threshold values.
 * \return A list of threshold values.
 */
QList<int> ColoredProgressBar::thresholds() const
{
    D_D(const ColoredProgressBar);
    return d->threshmap.keys();
}

void ColoredProgressBar::paintEvent(QPaintEvent *event)
{
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if (themeType == DGuiApplicationHelper::LightType) {
        QProgressBar::paintEvent(event);
    } else if (themeType == DGuiApplicationHelper::DarkType) {

        D_D(ColoredProgressBar);
        QStylePainter painter(this);
        QStyleOptionProgressBar styopt;
        initStyleOption(&styopt);

        if (d->threshmap.upperBound(value()) != d->threshmap.begin()) {
            styopt.palette.setBrush(QPalette::ColorRole::Highlight, (--d->threshmap.upperBound(value())).value());
        }

        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.drawRect(styopt.rect);

        painter.drawControl(QStyle::ControlElement::CE_ProgressBarGroove, styopt);
        painter.drawControl(QStyle::ControlElement::CE_ProgressBarContents, styopt);

        if (styopt.textVisible && styopt.orientation == Qt::Horizontal) {
            painter.drawControl(QStyle::ControlElement::CE_ProgressBarLabel, styopt);
        }
    } else {
        QProgressBar::paintEvent(event);
    }
}
