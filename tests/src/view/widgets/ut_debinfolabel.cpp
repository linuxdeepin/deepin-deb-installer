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
#include <gtest/gtest.h>

#include "../deb-installer/view/widgets/debinfolabel.h"

#include <QPaintEvent>

class UT_DebInfoLabel_TEST : public ::testing::Test
{
    // Test interface
protected:
    void SetUp()
    {
        m_label = new DebInfoLabel;
    }
    void TearDown()
    {
        delete m_label;
    }

    DebInfoLabel *m_label = nullptr;
};

TEST_F(UT_DebInfoLabel_TEST, DebInfoLabel_UT_setCustomQPalette)
{
    m_label->setCustomQPalette(QPalette::WindowText);
    ASSERT_EQ(QPalette::WindowText, m_label->m_colorRole);
    ASSERT_FALSE(m_label->m_bUserColorType);
}

TEST_F(UT_DebInfoLabel_TEST, DebInfoLabel_UT_setCustomQPalette_01)
{
    m_label->setCustomQPalette(QPalette::BrightText);
    ASSERT_EQ(QPalette::BrightText, m_label->m_colorRole);
    ASSERT_FALSE(m_label->m_bUserColorType);
}

TEST_F(UT_DebInfoLabel_TEST, DebInfoLabel_UT_setCustomDPalette)
{
    m_label->setCustomDPalette();
    ASSERT_TRUE(m_label->m_bMultiIns);
    ASSERT_TRUE(m_label->m_bUserColorType);
}

TEST_F(UT_DebInfoLabel_TEST, DebInfoLabel_UT_setCustomDPalette_01)
{
    m_label->setCustomDPalette(DPalette::TextLively);
    ASSERT_EQ(DPalette::TextLively, m_label->m_colorType);
    ASSERT_FALSE(m_label->m_bMultiIns);
    ASSERT_TRUE(m_label->m_bUserColorType);
}

TEST_F(UT_DebInfoLabel_TEST, DebInfoLabel_UT_PaintEvent)
{
    m_label->m_bUserColorType = true;
    m_label->m_bMultiIns = true;
    QPaintEvent paint(QRect(m_label->rect()));
    m_label->paintEvent(&paint);
    ASSERT_TRUE(m_label->m_bUserColorType);
    ASSERT_TRUE(m_label->m_bMultiIns);
}

TEST_F(UT_DebInfoLabel_TEST, DebInfoLabel_UT_PaintEvent_01)
{
    m_label->m_bUserColorType = true;
    m_label->m_bMultiIns = false;
    QPaintEvent paint1(QRect(m_label->rect()));
    m_label->paintEvent(&paint1);
    ASSERT_TRUE(m_label->m_bUserColorType);
    ASSERT_FALSE(m_label->m_bMultiIns);
}

TEST_F(UT_DebInfoLabel_TEST, DebInfoLabel_UT_PaintEvent_02)
{
    m_label->m_colorRole = QPalette::WindowText;
    m_label->m_bUserColorType = false;
    QPaintEvent paint2(QRect(m_label->rect()));
    m_label->paintEvent(&paint2);
    ASSERT_FALSE(m_label->m_bUserColorType);
    ASSERT_EQ(QPalette::WindowText, m_label->m_colorRole);
}

TEST_F(UT_DebInfoLabel_TEST, DebInfoLabel_UT_PaintEvent_03)
{
    m_label->m_colorRole = QPalette::BrightText;
    m_label->m_bUserColorType = false;
    QPaintEvent paint2(QRect(m_label->rect()));
    m_label->paintEvent(&paint2);
    ASSERT_FALSE(m_label->m_bUserColorType);
    ASSERT_EQ(QPalette::BrightText, m_label->m_colorRole);
}
