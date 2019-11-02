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

#ifndef PACKAGESLISTVIEW_H
#define PACKAGESLISTVIEW_H

#include <QMouseEvent>

#include <DMenu>
#include <DWidget>
#include <DListView>

DWIDGET_USE_NAMESPACE

class PackagesListView : public DListView {
    Q_OBJECT
public:
    explicit PackagesListView(DWidget *parent = nullptr);

signals:
    void onShowContextMenu(QModelIndex index);
    void onItemRemoveClicked(QModelIndex index);

protected:
    void leaveEvent(QEvent *e);

    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);

private:
    bool m_bLeftMouse;
    QModelIndex m_currModelIndex;
    DMenu *m_rightMenu {nullptr};

    void initUI();
    void initConnections();
    void initRightContextMenu();
    void initShortcuts();

private slots:
    void onListViewShowContextMenu(QModelIndex index);
    void onRightMenuDeleteAction();
};

#endif  // PACKAGESLISTVIEW_H
