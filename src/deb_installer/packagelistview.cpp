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
#include "utils.h"

#include <QPainter>
#include <QShortcut>

PackagesListView::PackagesListView(QWidget *parent)
    : DListView(parent)
    , m_bLeftMouse(false)
    , m_rightMenu(nullptr)
{
    initUI();
    initConnections();
    initRightContextMenu();
    initShortcuts();
    //this->grabKeyboard();//在packagelist中添加焦点
    this->setFocusPolicy(Qt::StrongFocus);
}

void PackagesListView::initUI()
{
    setVerticalScrollMode(ScrollPerPixel);
    setSelectionMode(QListView::SingleSelection);
    setAutoScroll(true);
    setMouseTracking(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

void PackagesListView::initConnections()
{
    connect(this,
            &PackagesListView::onShowContextMenu,
            this,
            &PackagesListView::onListViewShowContextMenu,
            Qt::ConnectionType::QueuedConnection);
}

void PackagesListView::initShortcuts()
{
    QShortcut *deleteShortcut = new QShortcut(QKeySequence::Delete, this);
    deleteShortcut->setContext(Qt::ApplicationShortcut);
    connect(deleteShortcut, SIGNAL(activated()), this, SLOT(onShortcutDeleteAction()));
}

void PackagesListView::leaveEvent(QEvent *e)
{
    DListView::leaveEvent(e);

    emit entered(QModelIndex());
}

void PackagesListView::mouseMoveEvent(QMouseEvent *event)
{
    DListView::mouseMoveEvent(event);
}

void PackagesListView::scrollContentsBy(int dx, int dy)
{
    if (-1 == m_highlightIndex.row()) {
        QListView::scrollContentsBy(dx, dy);
        return;
    }

    QListView::scrollContentsBy(dx, dy);
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
}

void PackagesListView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    DListView::setSelection(rect, command);

    QPoint clickPoint(rect.x(), rect.y());
    QModelIndex modelIndex = indexAt(clickPoint);

    m_highlightIndex = modelIndex;
    m_currModelIndex = m_highlightIndex;
    if (!m_bLeftMouse) {
        m_bShortcutDelete = false;
        emit onShowContextMenu(modelIndex);
    } else {
        m_bShortcutDelete = true;
    }
}

void PackagesListView::keyPressEvent(QKeyEvent *event)
{
    m_bLeftMouse = true;
    DListView::keyPressEvent(event);
}

void PackagesListView::initRightContextMenu()
{
    if (nullptr == m_rightMenu) {
        m_rightMenu = new DMenu(this);

        //给右键菜单添加快捷键Delete
        QAction *deleteAction = new QAction(tr("Delete"), this);

        m_rightMenu->addAction(deleteAction);
        connect(deleteAction, SIGNAL(triggered()), this, SLOT(onRightMenuDeleteAction()));
    }
}

void PackagesListView::onListViewShowContextMenu(QModelIndex index)
{
    Q_UNUSED(index)

    m_bShortcutDelete = false;
    m_currModelIndex = index;
    DMenu *rightMenu = m_rightMenu;
    /*
    connect(rightMenu, &DMenu::aboutToHide, this, [ = ] {
        this->grabKeyboard();//在packagelist中添加焦点
    });
    connect(rightMenu, &DMenu::aboutToShow, this, [ = ] {
        this->releaseKeyboard();//在packagelist中移除焦点
    });
    */

    const int operate_stat = index.data(DebListModel::PackageOperateStatusRole).toInt();
    if ((DebListModel::Success == operate_stat ||
            DebListModel::Waiting == operate_stat ||
            DebListModel::Operating == operate_stat) || DebListModel::Failed == operate_stat) {
        return;
    }

    //在当前鼠标位置显示右键菜单
    rightMenu->exec(QCursor::pos());
}

void PackagesListView::onShortcutDeleteAction()
{
    if (-1 == m_currModelIndex.row() || m_rightMenu->isVisible()) {
        return;
    }

    emit onRemoveItemClicked(m_currModelIndex);
}

void PackagesListView::onRightMenuDeleteAction()
{
    if (-1 == m_currModelIndex.row()) {
        return;
    }

    emit onRemoveItemClicked(m_currModelIndex);
}

void PackagesListView::paintEvent(QPaintEvent *event)
{
    //获取cuurentIndex并保存，用于右键菜单的定位。
    m_currentIndex = this->currentIndex().row();
    DListView::paintEvent(event);
}

void PackagesListView::setInitConfig()
{
    if (this->count() > 0)
        m_currModelIndex = this->model()->index(0, 0);
}

bool PackagesListView::eventFilter(QObject *watched, QEvent *event)
{
    if (QEvent::WindowDeactivate == event->type()) {
        if (this->focusWidget() != nullptr) {
            this->focusWidget()->clearFocus();
        }
        emit OutOfFocus(false);
        return QObject::eventFilter(watched, event);
    }
    if (QEvent::WindowActivate == event->type()) {
        this->repaint();
        this->update();
        emit OutOfFocus(false);
        return QObject::eventFilter(watched, event);
    }

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *key_event = static_cast < QKeyEvent *>(event); //将事件转化为键盘事件
        if (key_event->key() == Qt::Key_Escape) {
            if (m_rightMenu->isVisible())
                m_rightMenu->hide();
            return true;
        }
        if ((key_event->modifiers() == Qt::AltModifier) && (key_event->key() == Qt::Key_M)) {
            if (this->selectionModel()->currentIndex().row() > -1) {
                m_bShortcutDelete = false;
                if (this->hasFocus()) {
                    // 右键菜单的触发位置为当前item的位置。此前为鼠标的位置。
                    m_rightMenu->exec(mapToGlobal(m_rightMenuPos));
                }
            }
            return true;
        }

        if (key_event->key() == Qt::Key_Return) {
            if (m_rightMenu->isActiveWindow()) {
                m_rightMenu->activeAction()->trigger();
                m_rightMenu->hide();
                this->releaseKeyboard();
            }
        }

        if (key_event->key() == Qt::Key_Down || key_event->key() == Qt::Key_Up) {
            if (m_rightMenu->isVisible()) {
                m_rightMenu->setActiveAction(m_rightMenu->actions().at(0));
                return true;
            } else {
                if (key_event->key() == Qt::Key_Down) {
                    int row = this->model()->rowCount();
                    int currentNum = this->currentIndex().row();
                    if (currentNum < (row - 1)) {
                        this->setCurrentIndex(this->model()->index(currentNum + 1, 0));
                    }
                }
                if (key_event->key() == Qt::Key_Up) {
                    int currentNum = this->currentIndex().row();
                    if (currentNum > 0) {
                        this->setCurrentIndex(this->model()->index(currentNum - 1, 0));
                    }
                }
            }
        }
        return QObject::eventFilter(watched, event);
    }
    return QObject::eventFilter(watched, event);
}

/**
 * @brief PackagesListView::getPos
 * @param rect 某一个Item的rect
 * @param index Item的index.row()
 *
 * 获取某一个Item的rect,与index。用于保存位置参数触发右键菜单
 */
void PackagesListView::getPos(QRect rect, int index)
{
    if (index == m_currentIndex) {
        m_rightMenuPos.setX(rect.x() + rect.width() - m_rightMenu->width());
        m_rightMenuPos.setY(rect.y() + rect.height() / 2);
    }
}
