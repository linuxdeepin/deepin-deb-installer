/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "workerprogress.h"

WorkerProgress::WorkerProgress(QWidget *parent)
    : QProgressBar(parent)
{
    setMinimum(0);
    setMaximum(100);
    setFixedHeight(8);
    setFixedWidth(240);
    setTextVisible(false);
    setStyleSheet("QProgressBar {"
                  "border: 1px solid rgba(0, 0, 0, .03);"
                  "border-radius: 4px;"
                  "background-color: rgba(0, 0, 0, .05);"
                  "}");
}
