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

#include "../deb_installer/view/widgets/ShowInstallInfoTextEdit.h"

TEST(ShowInstallInfoTextEdit_TEST, ShowInstallInfoTextEdit_UT_slideGesture)
{
    ShowInstallInfoTextEdit *edit = new ShowInstallInfoTextEdit;
    edit->slideGesture(5);
}

TEST(ShowInstallInfoTextEdit_TEST, ShowInstallInfoTextEdit_UT_onSelectionArea)
{
    ShowInstallInfoTextEdit *edit = new ShowInstallInfoTextEdit;
    edit->m_gestureAction = ShowInstallInfoTextEdit::GA_tap;
    QTextCursor cursor = edit->textCursor();
    cursor.selectedText() = "//";
    edit->onSelectionArea();
}

TEST(FlashTween_TEST, FlashTween_UT_start)
{
    FlashTween flash;
    flash.start(0, 0, 1, 0, 0);
    flash.start(0, 0, 1, 1, 0);
}
