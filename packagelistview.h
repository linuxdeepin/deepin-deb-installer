/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef PACKAGESLISTVIEW_H
#define PACKAGESLISTVIEW_H

#include <QListView>

class PackagesListView : public QListView
{
    Q_OBJECT

public:
    explicit PackagesListView(QWidget *parent = 0);

protected:
    void leaveEvent(QEvent *e);
};

#endif // PACKAGESLISTVIEW_H
