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

#include "workerprogress.h"

WorkerProgress::WorkerProgress(QWidget *parent)
    : QProgressBar(parent)
{
    setMinimum(0);
    setMaximum(100);
    setFixedHeight(8);
    setFixedWidth(240);
    setTextVisible(false);
//    setStyleSheet(
//        "QProgressBar {"
//        "border: 1px solid rgba(0, 0, 0, .03);"
//        "border-radius: 4px;"
//        "background-color: rgba(0, 0, 0, .05);"
//        "}");
}
