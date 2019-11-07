/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "packagelistview.h"
#include "deblistmodel.h"

#include <QPainter>
#include <QShortcut>

PackagesListView::PackagesListView(DWidget *parent)
    : DListView(parent)
    , m_bLeftMouse(false)
    , m_rightMenu(nullptr)
{
    initUI();
    initConnections();
    initRightContextMenu();
}

void PackagesListView::initUI()
{
    setVerticalScrollMode(ScrollPerPixel);
    setSelectionMode(NoSelection);
    setAutoScroll(true);
    setMouseTracking(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    setAutoFillBackground(false);
}

void PackagesListView::initConnections()
{
    connect(this,
            &PackagesListView::onShowContextMenu,
            this,
            &PackagesListView::onListViewShowContextMenu,
            Qt::ConnectionType::QueuedConnection);
}

void PackagesListView::leaveEvent(QEvent *e)
{
    DListView::leaveEvent(e);

    emit entered(QModelIndex());
}

//QRect getIconRect(QRect visualRect)
//{
//    int icon_width = 22;
//    int icon_height = 22;
//    const int x = visualRect.right() - icon_width - 30;
//    const int y = visualRect.top() + (visualRect.height() - icon_height) / 2;
//    return QRect(x, y, icon_width, icon_height);
//}

void PackagesListView::mouseMoveEvent(QMouseEvent *event)
{
    DListView::mouseMoveEvent(event);
}

void PackagesListView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_bLeftMouse = true;
    } else {
        m_bLeftMouse = false;
    }

    DListView::mousePressEvent(event);
}

void PackagesListView::mouseReleaseEvent(QMouseEvent *event)
{
    DebListModel *debListModel = qobject_cast<DebListModel *>(this->model());
    if (!debListModel->isWorkerPrepare()) {
        return;
    }

    DListView::mouseReleaseEvent(event);

//    QPoint clickPoint = event->pos();
//    QModelIndex modelIndex = indexAt(clickPoint);
//    QRect rect = visualRect(modelIndex);
//    QRect removeIconRect = getIconRect(rect);

//    if (removeIconRect.contains(clickPoint)) {
//        emit onItemRemoveClicked(modelIndex);
//    }
}

void PackagesListView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    DListView::setSelection(rect, command);

    QPoint clickPoint(rect.x(), rect.y());
    QModelIndex modelIndex = indexAt(clickPoint);

    if (!m_bLeftMouse) {
        emit onShowContextMenu(modelIndex);
    }
}

void PackagesListView::initRightContextMenu()
{
    if (nullptr == m_rightMenu) {
        m_rightMenu = new DMenu(this);

        //给右键菜单添加快捷键Delete
        QAction *deleteAction = new QAction(tr("Delete"), this);
        deleteAction->setShortcuts(QKeySequence::Delete);
        deleteAction->setShortcutContext(Qt::WindowShortcut);

        QShortcut *menuShortcut = new QShortcut(QKeySequence::Delete, m_rightMenu);
        menuShortcut->setContext(Qt::WindowShortcut);
        connect(menuShortcut, SIGNAL(activated()), this, SLOT(onRightMenuDeleteAction()));

        m_rightMenu->addAction(deleteAction);
        connect(deleteAction, SIGNAL(triggered()), this, SLOT(onRightMenuDeleteAction()));
    }
}

void PackagesListView::onListViewShowContextMenu(QModelIndex index)
{
    Q_UNUSED(index)

    m_currModelIndex = index;
    DMenu *rightMenu = m_rightMenu;

    const int operate_stat = index.data(DebListModel::PackageOperateStatusRole).toInt();
    if (DebListModel::Success == operate_stat || DebListModel::Waiting == operate_stat) {
        return;
    }

    //在当前鼠标位置显示右键菜单
    rightMenu->exec(QCursor::pos());
}

void PackagesListView::onRightMenuDeleteAction()
{
    qDebug() << "onRightMenuDeleteAction";
    emit onItemRemoveClicked(m_currModelIndex);
}
