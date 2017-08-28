/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef PACKAGESLISTDELEGATE_H
#define PACKAGESLISTDELEGATE_H

#include <QAbstractItemDelegate>

class PackagesListDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit PackagesListDelegate(QObject *parent = 0);

public slots:
    void setCurrentIndex(const QModelIndex &idx);

private:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    QPixmap m_packageIcon;
    QPixmap m_removeIcon;
    QModelIndex m_currentIdx;
};

#endif // PACKAGESLISTDELEGATE_H
