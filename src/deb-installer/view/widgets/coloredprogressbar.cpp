// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "coloredprogressbar.h"
#include "utils/ddlog.h"

#include <DObjectPrivate>

#include <QMap>
#include <QPainter>
#include <QDebug>
#include <QStylePainter>
#include <QStyleOptionProgressBar>

ColoredProgressBar::ColoredProgressBar(QWidget *parent)
    : QProgressBar(parent)
{
    qCDebug(appLog) << "Initializing ColoredProgressBar...";
    themeChanged();
    QObject::connect(
        DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &ColoredProgressBar::themeChanged);
    qCDebug(appLog) << "ColoredProgressBar initialized";
}

void ColoredProgressBar::themeChanged()
{
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    qCDebug(appLog) << "Theme changed to:" << themeType;
    m_themeType = themeType;
    if (themeType == DGuiApplicationHelper::LightType) {
        qCDebug(appLog) << "Clearing thresholds for light theme";
        threshmap.clear();
    } else if (themeType == DGuiApplicationHelper::DarkType) {
        qCDebug(appLog) << "Setting dark theme colors";
        DPalette pa = DebApplicationHelper::instance()->palette(this);

        QColor colorStart = pa.color(QPalette::Highlight);
        QColor colorEnd = pa.color(QPalette::HighlightedText);
        qCDebug(appLog) << "Using colors from:" << colorStart.name() << "to" << colorEnd.name();

        threshmap.insert(0, QBrush(colorStart));
        threshmap.insert(1, QBrush(colorEnd));
    } else {
        qCDebug(appLog) << "Unknown theme type, clearing thresholds";
        threshmap.clear();
    }
}

void ColoredProgressBar::addThreshold(int threshold, QBrush brush)
{
    qCDebug(appLog) << "Adding threshold at:" << threshold << "with brush:" << brush.color().name();
    threshmap[threshold] = brush;
}

void ColoredProgressBar::removeThreshold(int threshold)
{
    if (threshmap.contains(threshold)) {
        qCDebug(appLog) << "Removing threshold at:" << threshold;
        threshmap.remove(threshold);
    }
}

QList<int> ColoredProgressBar::thresholds() const
{
    return threshmap.keys();
}

void ColoredProgressBar::paintEvent(QPaintEvent *event)
{
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if (themeType == DGuiApplicationHelper::LightType) {
        qCDebug(appLog) << "Using default painting for light theme";
        QProgressBar::paintEvent(event);
    } else if (themeType == DGuiApplicationHelper::DarkType) {
        qCDebug(appLog) << "Custom painting for dark theme";
        QStylePainter painter(this);
        QStyleOptionProgressBar styopt;
        initStyleOption(&styopt);

        if (threshmap.upperBound(value()) != threshmap.begin()) {
            styopt.palette.setBrush(QPalette::ColorRole::Highlight, (--threshmap.upperBound(value())).value());
        }

        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.drawRect(styopt.rect);

        painter.drawControl(QStyle::ControlElement::CE_ProgressBarGroove, styopt);
        painter.drawControl(QStyle::ControlElement::CE_ProgressBarContents, styopt);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        if (styopt.textVisible && styopt.orientation == Qt::Horizontal) {
#else
        if (styopt.textVisible && (styopt.direction == Qt::LeftToRight || styopt.direction == Qt::RightToLeft)) {
#endif
            qCDebug(appLog) << "Painting progress bar label";
            painter.drawControl(QStyle::ControlElement::CE_ProgressBarLabel, styopt);
        }
    } else {
        qCDebug(appLog) << "Unknown theme type, using default painting";
        QProgressBar::paintEvent(event);
    }
}
