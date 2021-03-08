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

using namespace QApt;

bool delegate_backend_init()
{
    return true;
}

void delegate_checkSystemVersion()
{
}

bool delegate_backendReady()
{
    return true;
}

QString delegate_deb_arch_i386()
{
    return "i386";
}

bool delegate_deb_isValid()
{
    return true;
}

QByteArray delegate_deb_md5Sum()
{
    return nullptr;
}

int delegate_deb_installSize()
{
    return 0;
}

QString delegate_deb_packageName()
{
    return "";
}

QString delegate_deb_longDescription()
{
    return "longDescription";
}

QString delegate_deb_version()
{
    return "version";
}

QList<DependencyItem> delegate_deb_conflicts()
{
    DependencyInfo info("packageName", "0.0", RelationType::Equals, Depends);
    QList<DependencyInfo> dependencyItem;
    dependencyItem << info;
    QList<DependencyItem> conflicts;
    conflicts << dependencyItem;

    return conflicts;
}

Package *delegate_packageWithArch(QString, QString, QString)
{
    return nullptr;
}

QStringList delegate_backend_architectures()
{
    return {"i386", "amd64"};
}

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

QVariant stud_data(int role)
{
    return DebListModel::Waiting;
}

TEST(packageslistdelegate_Test, packageslistdelegate_UT_paint)
{
    PackagesListView *listview = new PackagesListView;
    PackagesListDelegate *delegate = new PackagesListDelegate(nullptr, listview);
    QPainter painter(listview);
    QStyleOptionViewItem option;
    Stub stub;
    stub.set(ADDR(Backend, init), delegate_backend_init);
    stub.set(ADDR(PackagesManager, isBackendReady), delegate_backendReady);
    stub.set(ADDR(DebListModel, checkSystemVersion), delegate_checkSystemVersion);

    stub.set(ADDR(DebFile, architecture), delegate_deb_arch_i386);
    stub.set(ADDR(Backend, architectures), delegate_backend_architectures);
    stub.set(ADDR(QModelIndex, isValid), delegate_deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), delegate_deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), delegate_deb_installSize);
    stub.set(ADDR(DebFile, packageName), delegate_deb_packageName);
    stub.set(ADDR(DebFile, longDescription), delegate_deb_longDescription);
    stub.set(ADDR(DebFile, version), delegate_deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), delegate_packageWithArch);
    stub.set(ADDR(PackagesManager, removePackage), delegate_checkSystemVersion);
    stub.set(ADDR(QModelIndex, data), stud_data);
    stub.set(ADDR(DebFile, conflicts), delegate_deb_conflicts);
    DebListModel *model = new DebListModel;
    model->appendPackage(QStringList() << "\n");
    QModelIndex index = model->index(0);
    delegate->paint(&painter, option, index);
    delete delegate;
    delete listview;
}

PackageDependsStatus delegate_getPackageDependsStatus(const int index)
{
    Q_UNUSED(index);
    PackageDependsStatus status;
    return status;
}
TEST(packageslistdelegate_Test, packageslistdelegate_UT_sizeHint)
{
    PackagesListView *listview = new PackagesListView;
    PackagesListDelegate *delegate = new PackagesListDelegate(nullptr, listview);
    QStyleOptionViewItem option;

    Stub stub;
    stub.set(ADDR(Backend, init), delegate_backend_init);
    stub.set(ADDR(PackagesManager, isBackendReady), delegate_backendReady);
    stub.set(ADDR(DebListModel, checkSystemVersion), delegate_checkSystemVersion);

    stub.set(ADDR(DebFile, architecture), delegate_deb_arch_i386);
    stub.set(ADDR(Backend, architectures), delegate_backend_architectures);
    stub.set(ADDR(Backend, init), delegate_backend_init);
    stub.set(ADDR(DebFile, isValid), delegate_deb_isValid);
    stub.set(ADDR(DebFile, md5Sum), delegate_deb_md5Sum);
    stub.set(ADDR(DebFile, installedSize), delegate_deb_installSize);
    stub.set(ADDR(DebFile, packageName), delegate_deb_packageName);
    stub.set(ADDR(DebFile, longDescription), delegate_deb_longDescription);
    stub.set(ADDR(DebFile, version), delegate_deb_version);
    stub.set(ADDR(PackagesManager, packageWithArch), delegate_packageWithArch);
    stub.set(ADDR(PackagesManager, removePackage), delegate_checkSystemVersion);
    stub.set(ADDR(PackagesManager, getPackageDependsStatus), delegate_getPackageDependsStatus);


    stub.set(ADDR(DebFile, conflicts), delegate_deb_conflicts);
    DebListModel *model = new DebListModel;
    model->appendPackage(QStringList() << "\n");
    QModelIndex index = model->index(0);
    delegate->sizeHint(option, index);
    delete delegate;
    delete listview;
}
