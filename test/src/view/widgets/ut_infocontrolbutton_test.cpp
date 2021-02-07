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

#include "../deb_installer/view/widgets/infocontrolbutton.h"

#include <ut_Head.h>

#include <QMouseEvent>
#include <QKeyEvent>
#include <QCoreApplication>

#include <gtest/gtest.h>

class InfoControlButton_Test : public UT_HEAD
{
public:
    virtual void setup()
    {
    }
    void TearDown()
    {
        delete btn;
    }
    InfoControlButton *btn;
};

TEST_F(InfoControlButton_Test, InfoControlButton_UT_setExpandTips)
{
    btn = new InfoControlButton("", "");
    btn->setExpandTips("");
}

TEST_F(InfoControlButton_Test, InfoControlButton_UT_setShrinkTips)
{
    btn = new InfoControlButton("", "");
    btn->setShrinkTips("");
}

TEST_F(InfoControlButton_Test, InfoControlButton_UT_onMouseRelease)
{
    btn = new InfoControlButton("", "");
    btn->onMouseRelease();
    btn->m_expand = true;
    btn->onMouseRelease();
    EXPECT_FALSE(btn->m_expand);
}

TEST_F(InfoControlButton_Test, InfoControlButton_UT_themeChanged)
{
    btn = new InfoControlButton("", "");
    btn->themeChanged();
    btn->m_expand = true;
    btn->themeChanged();
    EXPECT_TRUE(btn->m_expand);
}

TEST_F(InfoControlButton_Test, InfoControlButton_UT_mouseReleaseEvent)
{
    btn = new InfoControlButton("", "");
    QMouseEvent mouseReleaseEvent(QEvent::MouseButtonRelease, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    btn->mouseReleaseEvent(&mouseReleaseEvent);
}

TEST_F(InfoControlButton_Test, InfoControlButton_UT_keyPressEvent)
{
    btn = new InfoControlButton("", "");
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QCoreApplication::sendEvent(btn, &keyPressEvent);
}
