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

#include "coloredprogressbar.h"

#include <DObjectPrivate>

#include <QMap>
#include <QPainter>
#include <QDebug>
#include <QStylePainter>
#include <QStyleOptionProgressBar>

class ColoredProgressBarPrivate : DTK_CORE_NAMESPACE::DObjectPrivate
{
    Q_DECLARE_PUBLIC(ColoredProgressBar)
    ColoredProgressBarPrivate(ColoredProgressBar *q);
private:
    QMap<int, QBrush> threshmap;
};


ColoredProgressBarPrivate::ColoredProgressBarPrivate(ColoredProgressBar *q)
    : DObjectPrivate(q)
{
}

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

void ColoredProgressBar::addThreshold(int threshold, QBrush brush)
{
    D_D(ColoredProgressBar);
    d->threshmap[threshold] = brush;
}

void ColoredProgressBar::removeThreshold(int threshold)
{
    D_D(ColoredProgressBar);
    if (d->threshmap.contains(threshold)) {
        d->threshmap.remove(threshold);
    }
}

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
