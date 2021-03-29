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

#include "../deb-installer/view/widgets/coloredprogressbar.h"
#include <stub.h>
#include "utils/utils.h"

#include <DGuiApplicationHelper>

#include <QPaintEvent>

#include <gtest/gtest.h>

DGuiApplicationHelper::ColorType stub_themeType()
{
    return DGuiApplicationHelper::DarkType;
}

class ut_coloredProgressBar_Test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp()
    {
        m_colorProcessBar = new ColoredProgressBar;
    }
    void TearDown()
    {
        delete m_colorProcessBar;
    }

    ColoredProgressBar *m_colorProcessBar = nullptr;
};

TEST_F(ut_coloredProgressBar_Test, ColoredProgressBar_UT_themeChanged)
{
    m_colorProcessBar->themeChanged();
}

TEST_F(ut_coloredProgressBar_Test, ColoredProgressBar_UT_addThreshold)
{
    m_colorProcessBar->addThreshold(0, QBrush(nullptr));
}

TEST_F(ut_coloredProgressBar_Test, ColoredProgressBar_UT_removeThreshold)
{
    m_colorProcessBar->removeThreshold(1);
}

TEST_F(ut_coloredProgressBar_Test, ColoredProgressBar_UT_thresholds)
{
    m_colorProcessBar->thresholds();
}

TEST_F(ut_coloredProgressBar_Test, ColoredProgressBar_UT_paintEvent_Light)
{
    QPaintEvent paint(QRect(m_colorProcessBar->rect()));
    m_colorProcessBar->paintEvent(&paint);
}

TEST_F(ut_coloredProgressBar_Test, ColoredProgressBar_UT_paintEvent_Dark)
{
    Stub stub;
    stub.set(ADDR(DGuiApplicationHelper, themeType), stub_themeType);

    QPaintEvent paint(QRect(m_colorProcessBar->rect()));
    m_colorProcessBar->paintEvent(&paint);
}
