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

#ifndef PACKAGESLISTDELEGATE_H
#define PACKAGESLISTDELEGATE_H

#include <DStyledItemDelegate>
#include <QSettings>
#include "packagelistview.h"
#include "deblistmodel.h"
class PackagesListDelegate : public DStyledItemDelegate
{
    Q_OBJECT

public:
    //delegate使用传入的model而非重新new一个对象 解决多次创建model packagemanager导致崩溃的问题
    explicit PackagesListDelegate(DebListModel *m_model, QAbstractItemView *parent = nullptr);
    bool eventFilter(QObject *watched, QEvent *event) override;
    /**
     * @brief getItemHeight item高度
     */
    void getItemHeight(int height);

signals:
    // 用来发送当前Item的位置参数和row。确定右键菜单的位置。
    void sigIndexAndRect(QRect rect, int index) const;

private:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    /**
     * @brief refreshDebItemStatus 刷新每个item 包的状态
     * @param operate_stat         操作状态
     * @param install_status_rect  安装状态的位置
     * @param painter
     * @param isSelect              是否被选中
     * @param isEnable              是否可用
     */
    void refreshDebItemStatus(const int operate_stat,
                              QRect install_status_rect,
                              QPainter *painter,
                              bool isSelect, bool isEnable) const;

private:
    QPixmap m_packageIcon;                  //包的图标
    QSettings m_qsettings;                  //废弃变量
    QAbstractItemView *m_parentView;
    int m_itemHeight;                       //item的高度
    DebListModel *m_fileListModel;          //传入的model
    int dependsStat_temp;                   //依赖状态
};

#endif  // PACKAGESLISTDELEGATE_H
