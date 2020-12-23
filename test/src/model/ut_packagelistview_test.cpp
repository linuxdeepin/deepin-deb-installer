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

#include "../deb_installer/model/deblistmodel.h"
#include "../deb_installer/model/packagelistview.h"
#include "../deb_installer/manager/packagesmanager.h"
#include "../deb_installer/manager/PackageDependsStatus.h"
#include "utils/utils.h"

#include <stub.h>

TEST(packagelistview_Test, packagelistview_UT_initUI)
{
    PackagesListView *listview = new PackagesListView;
    listview->initUI();
}

TEST(packagelistview_Test, packagelistview_UT_initConnection)
{
    PackagesListView *listview = new PackagesListView;
    listview->initConnections();
}

TEST(packagelistview_Test, packagelistview_UT_initShortcus)
{
    PackagesListView *listview = new PackagesListView;
    listview->initShortcuts();
}

TEST(packagelistview_Test, packagelistview_UT_initRightContextMenu)
{
    PackagesListView *listview = new PackagesListView;
    listview->initRightContextMenu();
}

TEST(packagelistview_Test, packagelistview_UT_onShortcutDeleteAction)
{
    PackagesListView *listview = new PackagesListView;
    listview->onShortcutDeleteAction();
}

TEST(packagelistview_Test, packagelistview_UT_onListViewShowContextMenu)
{
    PackagesListView *listview = new PackagesListView;
    QModelIndex index;
    listview->onListViewShowContextMenu(index);
}

TEST(packagelistview_Test, packagelistview_UT_onRightMenuDeleteAction)
{
    PackagesListView *listview = new PackagesListView;
    listview->onRightMenuDeleteAction();
}

TEST(packagelistview_Test, packagelistview_UT_getPos)
{
    PackagesListView *listview = new PackagesListView;
    listview->m_currentIndex = 1;
    listview->getPos(QRect(1, 1, 1, 1), 1);
}

TEST(packagelistview_Test, packagelistview_UT_setRightMenuShowStatus)
{
    PackagesListView *listview = new PackagesListView;
    listview->setRightMenuShowStatus(true);
}

TEST(packagelistview_Test, packagelistview_UT_setSelection)
{
    PackagesListView *listview = new PackagesListView;
    QItemSelectionModel::SelectionFlags command;
    listview->setSelection(QRect(10, 10, 10, 10), command);
    ASSERT_FALSE(listview->m_bShortcutDelete);
}

TEST(packagelistview_Test, packagelistview_UT_mousePressEvent)
{
    PackagesListView *listview = new PackagesListView;
    QMouseEvent mousePressEvent(QEvent::MouseButtonRelease, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    listview->mousePressEvent(&mousePressEvent);
}

TEST(packagelistview_Test, packagelistview_UT_PaintEvent)
{
    PackagesListView *listview = new PackagesListView;
    QPaintEvent paint(QRect(listview->rect()));
    listview->paintEvent(&paint);
    delete listview;
}
