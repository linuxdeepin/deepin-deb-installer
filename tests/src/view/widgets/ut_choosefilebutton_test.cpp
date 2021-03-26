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

#include "../deb-installer/view/widgets/choosefilebutton.h"
#include "utils/utils.h"

#include <QKeyEvent>

#include <gtest/gtest.h>

class ut_chooseFileButton_TEST : public ::testing::Test
{
    // Test interface
protected:
    void SetUp()
    {
        m_choosFileBtn = new ChooseFileButton("");
    }
    void TearDown()
    {
        delete m_choosFileBtn;
    }

    ChooseFileButton *m_choosFileBtn = nullptr;
};

TEST_F(ut_chooseFileButton_TEST, ChooseFileButton_UT_setFamily)
{
    m_choosFileBtn->setText("");
}

TEST_F(ut_chooseFileButton_TEST, ChooseFileButton_UT_keyPressEvent)
{
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QCoreApplication::sendEvent(m_choosFileBtn, &keyPressEvent);
}
