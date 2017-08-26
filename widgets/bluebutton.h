/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef BLUEBUTTON_H
#define BLUEBUTTON_H

#include <QPushButton>

class BlueButton : public QPushButton
{
    Q_OBJECT

public:
    explicit BlueButton(QWidget *parent = 0);
};

#endif // BLUEBUTTON_H
