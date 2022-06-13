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

#include "../deb-installer/view/widgets/filechoosewidget.h"
#include <stub.h>

#include <DFileDialog>

#include <gtest/gtest.h>
typedef int (*DFileDialogfptr)(QDialog *);
DFileDialogfptr DDialog_exec = (DFileDialogfptr)(&QDialog::exec);

int stub_exec()
{
    return 0;
}

TEST(FileChooseWidget_TEST, FileChooseWidget_UT_themeChanged)
{
    FileChooseWidget *fchooseWidget = new FileChooseWidget;
    fchooseWidget->themeChanged();
    ASSERT_EQ(QSize(160, 160), fchooseWidget->m_iconImage->size());
    delete fchooseWidget;
}

TEST(FileChooseWidget_TEST, FileChooseWidget_UT_chooseFiles)
{
    FileChooseWidget *fchooseWidget = new FileChooseWidget;
    fchooseWidget->m_settings.setValue("history_dir", "/");
    Stub stub;
    stub.set(DDialog_exec, stub_exec);
    fchooseWidget->chooseFiles();
    ASSERT_EQ("/", fchooseWidget->m_settings.value("history_dir"));
    delete fchooseWidget;
}

TEST(FileChooseWidget_TEST, FileChooseWidget_UT_clearChooseFileBtnFocus)
{
    FileChooseWidget *fchooseWidget = new FileChooseWidget;
    fchooseWidget->clearChooseFileBtnFocus();
    ASSERT_FALSE(fchooseWidget->hasFocus());
    delete fchooseWidget;
}
