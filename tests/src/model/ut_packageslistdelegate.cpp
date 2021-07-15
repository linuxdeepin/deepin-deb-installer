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
#include "../deb-installer/model/packageslistdelegate.h"
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

class ut_packageslistdelegate_Test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp()
    {
        m_listview = new PackagesListView;
        m_delegate = new PackagesListDelegate(nullptr, m_listview);
    }
    void TearDown()
    {
        delete m_delegate;
        delete m_listview;
    }

    PackagesListView *m_listview = nullptr;
    PackagesListDelegate *m_delegate;
    Stub stub;
};

TEST_F(ut_packageslistdelegate_Test, packageslistdelegate_UT_getItemHeight)
{
    m_delegate->getItemHeight(48);
}

TEST_F(ut_packageslistdelegate_Test, packageslistdelegate_UT_refreshDebItemStatus)
{
    PackagesListDelegate *delegate = new PackagesListDelegate(nullptr, m_listview);
    QPainter painter(m_listview);
    m_delegate->refreshDebItemStatus(1, QRect(0, 0, 10, 10), &painter, true, true);
    m_delegate->refreshDebItemStatus(2, QRect(0, 0, 10, 10), &painter, true, true);
    m_delegate->refreshDebItemStatus(4, QRect(0, 0, 10, 10), &painter, true, true);
    m_delegate->refreshDebItemStatus(0, QRect(0, 0, 10, 10), &painter, true, true);
}

QVariant stud_data(int role)
{
    return DebListModel::Waiting;
}

TEST_F(ut_packageslistdelegate_Test, packageslistdelegate_UT_paint)
{
    PackagesListDelegate *delegate = new PackagesListDelegate(nullptr, m_listview);
    QPainter painter(m_listview);
    QStyleOptionViewItem option;
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
    model->slotAppendPackage(QStringList() << "\n");
    model->m_packagesManager->m_preparedPackages.append("deb");
    m_listview->setModel(model);
    QModelIndex index = m_listview->model()->index(0, 0);
    m_delegate->paint(&painter, option, index);
}

PackageDependsStatus delegate_getPackageDependsStatus(const int index)
{
    Q_UNUSED(index);
    PackageDependsStatus status;
    return status;
}
TEST_F(ut_packageslistdelegate_Test, packageslistdelegate_UT_sizeHint)
{
    PackagesListDelegate *delegate = new PackagesListDelegate(nullptr, m_listview);
    QStyleOptionViewItem option;

    stub.set(ADDR(Backend, init), delegate_backend_init);
    stub.set(ADDR(PackagesManager, isBackendReady), delegate_backendReady);
    stub.set(ADDR(DebListModel, checkSystemVersion), delegate_checkSystemVersion);

    stub.set(ADDR(DebFile, architecture), delegate_deb_arch_i386);
    stub.set(ADDR(Backend, architectures), delegate_backend_architectures);
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
    model->slotAppendPackage(QStringList() << "\n");
    QModelIndex index = model->index(0);
    m_delegate->sizeHint(option, index);
}
