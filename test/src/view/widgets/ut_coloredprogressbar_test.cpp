/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      zhangkai <zhangkai@uniontech.com>
* Maintainer:  zhangkai <zhangkai@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../deb_installer/view/widgets/coloredprogressbar.h"
#include <stub.h>
#include "utils/utils.h"

#include <DGuiApplicationHelper>

#include <QPaintEvent>

#include <gtest/gtest.h>

DGuiApplicationHelper::ColorType stub_themeType()
{
    return DGuiApplicationHelper::DarkType;
}

TEST(ColoredProgressBar_TEST, ColoredProgressBar_UT_themeChanged)
{
    ColoredProgressBar *bar = new ColoredProgressBar;
    bar->themeChanged();
}

TEST(ColoredProgressBar_TEST, ColoredProgressBar_UT_addThreshold)
{
    ColoredProgressBar *bar = new ColoredProgressBar;
    bar->addThreshold(0, QBrush(nullptr));
}

TEST(ColoredProgressBar_TEST, ColoredProgressBar_UT_removeThreshold)
{
    ColoredProgressBar *bar = new ColoredProgressBar;
    bar->removeThreshold(1);
}

TEST(ColoredProgressBar_TEST, ColoredProgressBar_UT_thresholds)
{
    ColoredProgressBar *bar = new ColoredProgressBar;
    qDebug() << bar->thresholds();
}

TEST(ColoredProgressBar_TEST, ColoredProgressBar_UT_paintEvent_Light)
{
    ColoredProgressBar *bar = new ColoredProgressBar;
    QPaintEvent paint(QRect(bar->rect()));
    bar->paintEvent(&paint);
    delete bar;
}

TEST(ColoredProgressBar_TEST, ColoredProgressBar_UT_paintEvent_Dark)
{
    Stub stub;
    stub.set(ADDR(DGuiApplicationHelper, themeType), stub_themeType);
    ColoredProgressBar *bar = new ColoredProgressBar;
    QPaintEvent paint(QRect(bar->rect()));
    bar->paintEvent(&paint);
    delete bar;
}
