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
#include <QKeyEvent>
#include <DMenu>
#include <QWidget>
#include <DListView>

DWIDGET_USE_NAMESPACE

class PackagesListView : public DListView
{
    Q_OBJECT
public:
    explicit PackagesListView(QWidget *parent = nullptr);
    void setInitConfig();

signals:
    void onShowHideTopBg(bool bShow);
    void onShowHideBottomBg(bool bShow);

    void onClickItemAtIndex(QModelIndex index);
    void onShowContextMenu(QModelIndex index);
    void onRemoveItemClicked(QModelIndex index);

    void OutOfFocus(bool);

public slots:
    void getPos(QRect rect, int index); //获取到的当前Item的位置和row
protected:
    void leaveEvent(QEvent *e);

    void scrollContentsBy(int dx, int dy);

    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);
    void keyPressEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    void initUI();
    void initConnections();
    void initRightContextMenu();
    void initShortcuts();

    void handleHideShowSelection();

private slots:
    void onListViewShowContextMenu(QModelIndex index);
    void onRightMenuDeleteAction();
    void onShortcutDeleteAction();

private:
    bool m_bLeftMouse;
    bool m_bShortcutDelete;
    QModelIndex m_currModelIndex;
    DMenu *m_rightMenu {nullptr};
    QModelIndex m_highlightIndex;

    QPoint m_rightMenuPos; //确定的右键菜单出现的位置
    int m_currentIndex; //当前选中的index.row
};

#endif  // PACKAGESLISTVIEW_H
