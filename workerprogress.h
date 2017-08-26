/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef WORKERPROGRESS_H
#define WORKERPROGRESS_H

#include <QProgressBar>

class WorkerProgress : public QProgressBar
{
    Q_OBJECT

public:
    explicit WorkerProgress(QWidget *parent = 0);
};

#endif // WORKERPROGRESS_H
