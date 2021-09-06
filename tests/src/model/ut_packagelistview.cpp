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

#include "../deb-installer/model/deblistmodel.h"
#include "../deb-installer/model/packagelistview.h"
#include "../deb-installer/manager/packagesmanager.h"
#include "../deb-installer/manager/PackageDependsStatus.h"
#include "utils/utils.h"

#include <stub.h>

class ut_packagelistview_Test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp()
    {
        m_listview = new PackagesListView;
    }
    void TearDown()
    {
        delete m_listview;
    }

    PackagesListView *m_listview = nullptr;
};

TEST_F(ut_packagelistview_Test, packagelistview_UT_initUI)
{
    m_listview->initUI();
    EXPECT_TRUE(m_listview->hasAutoScroll());
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_initConnection)
{
    m_listview->initConnections();
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_initShortcus)
{
    m_listview->initShortcuts();
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_initRightContextMenu)
{
    m_listview->initRightContextMenu();
    EXPECT_EQ(1, m_listview->m_rightMenu->actions().size());
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_onShortcutDeleteAction)
{
    m_listview->slotShortcutDeleteAction();
    EXPECT_FALSE(m_listview->m_rightMenu->isVisible());
    EXPECT_EQ(-1, m_listview->m_currModelIndex.row());
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_onListViewShowContextMenu)
{
    QModelIndex index;
    m_listview->slotListViewShowContextMenu(index);
    EXPECT_FALSE(m_listview->m_bShortcutDelete);
    EXPECT_FALSE(m_listview->m_bIsRightMenuShow);
    EXPECT_FALSE(m_listview->m_rightMenu->isVisible());
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_onRightMenuDeleteAction)
{
    m_listview->slotRightMenuDeleteAction();
    EXPECT_EQ(-1, m_listview->m_currModelIndex.row());
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_getPos)
{
    m_listview->m_currentIndex = 1;
    m_listview->slotGetPos(QRect(1, 1, 1, 1), 1);
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_setRightMenuShowStatus)
{
    m_listview->setRightMenuShowStatus(true);
    EXPECT_TRUE(m_listview->m_bIsRightMenuShow);
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_setSelection)
{
    QItemSelectionModel::SelectionFlags command;
    m_listview->setSelection(QRect(10, 10, 10, 10), command);
    ASSERT_FALSE(m_listview->m_bShortcutDelete);
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_mousePressEvent)
{
    QMouseEvent mousePressEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_listview->mousePressEvent(&mousePressEvent);
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_PaintEvent)
{
    QPaintEvent paint(QRect(m_listview->rect()));
    m_listview->paintEvent(&paint);
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_keyPressEvent)
{
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QCoreApplication::sendEvent(m_listview, &keyPressEvent);
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_focusInEvent)
{
    QFocusEvent focus(QFocusEvent::FocusIn);
    QCoreApplication::sendEvent(m_listview, &focus);
}

TEST_F(ut_packagelistview_Test, packagelistview_UT_event)
{
    QEvent event(QEvent::FontChange);
    QCoreApplication::sendEvent(m_listview, &event);
}
