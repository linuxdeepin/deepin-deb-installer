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

#include "../deb-installer/view/widgets/installprocessinfoview.h"
#include "../deb-installer/view/widgets/ShowInstallInfoTextEdit.h"

#include <DGuiApplicationHelper>

#include <stub.h>

#include <gtest/gtest.h>

class ut_installProcessInfoView_Test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp()
    {
        m_infoView = new InstallProcessInfoView(100, 50);
    }
    void TearDown()
    {
        delete m_infoView;
    }

    InstallProcessInfoView *m_infoView = nullptr;
};

TEST_F(ut_installProcessInfoView_Test, InstallProcessInfoView_UT_setTextColor)
{
    m_infoView->setTextColor(DPalette::TextTitle);
    ASSERT_EQ(DPalette::TextTitle, m_infoView->m_colorType);
}

TEST_F(ut_installProcessInfoView_Test, InstallProcessInfoView_UT_appendText)
{
    m_infoView->appendText("");
    m_infoView->repaint();
    EXPECT_EQ("", m_infoView->m_editor->toPlainText());
}

TEST_F(ut_installProcessInfoView_Test, InstallProcessInfoView_UT_clearText)
{
    m_infoView->clearText();
    EXPECT_EQ("", m_infoView->m_editor->toPlainText());
}

DGuiApplicationHelper::ColorType ut_view_themeType()
{
    return DGuiApplicationHelper::DarkType;
}

DGuiApplicationHelper::ColorType ut_view_themeType1()
{
    return DGuiApplicationHelper::UnknownType;
}

DGuiApplicationHelper::ColorType ut_view_themeType2()
{
    return DGuiApplicationHelper::LightType;
}

TEST_F(ut_installProcessInfoView_Test, InstallProcessInfoView_UT_paintEvent)
{
    Stub stub;
    stub.set(ADDR(DGuiApplicationHelper, themeType), ut_view_themeType2);
    QPaintEvent paint(QRect(m_infoView->rect()));
    m_infoView->paintEvent(&paint);
    ASSERT_EQ(QColor(96, 157, 200), m_infoView->m_editor->palette().text().color());
}

TEST_F(ut_installProcessInfoView_Test, InstallProcessInfoView_UT_paintEvent_01)
{
    Stub stub;
    stub.set(ADDR(DGuiApplicationHelper,themeType),ut_view_themeType);
    QPaintEvent paint1(QRect(m_infoView->rect()));
    m_infoView->paintEvent(&paint1);
    ASSERT_EQ(QColor(109, 124, 136), m_infoView->m_editor->palette().text().color());
}

TEST_F(ut_installProcessInfoView_Test, InstallProcessInfoView_UT_paintEvent_02)
{
    Stub stub1;
    stub1.set(ADDR(DGuiApplicationHelper,themeType),ut_view_themeType1);
    QPaintEvent paint2(QRect(m_infoView->rect()));
    m_infoView->paintEvent(&paint2);
    ASSERT_EQ(QColor(96, 157, 200), m_infoView->m_editor->palette().text().color());
}
