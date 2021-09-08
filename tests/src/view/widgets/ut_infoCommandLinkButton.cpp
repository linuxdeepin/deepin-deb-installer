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

#include "../deb-installer/view/widgets/InfoCommandLinkButton.h"
#include "utils/utils.h"

#include <QKeyEvent>

#include <gtest/gtest.h>

TEST(InfoCommandLinkButton_TEST, InfoCommandLinkButton_UT_setFamily)
{
    InfoCommandLinkButton *btn = new InfoCommandLinkButton("");
    btn->setFocusPolicy(Qt::TabFocus);
    EXPECT_EQ(Qt::TabFocus, btn->focusPolicy());
    delete btn;
}

TEST(InfoCommandLinkButton_TEST, InfoCommandLinkButton_UT_keyPressEvent)
{
    InfoCommandLinkButton *btn = new InfoCommandLinkButton("");
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QCoreApplication::sendEvent(btn, &keyPressEvent);
    EXPECT_FALSE(btn->hasFocus());
    delete btn;
}
