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
#include "../deb_installer/model/packageslistdelegate.h"
#include "utils/utils.h"

#include <QPainter>

#include <stub.h>

TEST(packageslistdelegate_Test, packageslistdelegate_UT_getItemHeight)
{
    PackagesListView *listview = new PackagesListView;
    PackagesListDelegate *delegate = new PackagesListDelegate(nullptr, listview);
    delegate->getItemHeight(48);
    delete delegate;
    delete listview;
}

TEST(packageslistdelegate_Test, packageslistdelegate_UT_refreshDebItemStatus)
{
    PackagesListView *listview = new PackagesListView;
    PackagesListDelegate *delegate = new PackagesListDelegate(nullptr, listview);
    QPainter painter(listview);
    delegate->refreshDebItemStatus(1, QRect(0, 0, 10, 10), &painter, true, true);
    delegate->refreshDebItemStatus(2, QRect(0, 0, 10, 10), &painter, true, true);
    delete delegate;
    delete listview;
}

TEST(packageslistdelegate_Test, packageslistdelegate_UT_paint)
{
    PackagesListView *listview = new PackagesListView;
    PackagesListDelegate *delegate = new PackagesListDelegate(nullptr, listview);
    QPainter painter(listview);
    QStyleOptionViewItem option;
    DebListModel *model = new DebListModel;
    model->appendPackage(QStringList() << "\n");
    QModelIndex index = model->index(0);
    delegate->paint(&painter, option, index);
    delete delegate;
    delete listview;
}

TEST(packageslistdelegate_Test, packageslistdelegate_UT_sizeHint)
{
    PackagesListView *listview = new PackagesListView;
    PackagesListDelegate *delegate = new PackagesListDelegate(nullptr, listview);
    QStyleOptionViewItem option;
    DebListModel *model = new DebListModel;
    model->appendPackage(QStringList() << "\n");
    QModelIndex index = model->index(0);
    delegate->sizeHint(option, index);
    delete delegate;
    delete listview;
}
