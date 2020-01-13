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

class PackagesListDelegate : public DStyledItemDelegate {
    Q_OBJECT

public:
    explicit PackagesListDelegate(QAbstractItemView *parent = nullptr);
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void refreshDebItemStatus(const int operate_stat,
                           QRect install_status_rect,
                           QPainter *painter,
                           const QModelIndex &index) const;

private:
    QPixmap m_packageIcon;
    QSettings m_qsettings;
    QAbstractItemView *m_parentView;
    int m_itemHeight;
};

#endif  // PACKAGESLISTDELEGATE_H
