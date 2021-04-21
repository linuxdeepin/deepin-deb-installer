/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
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
    : DProgressBar(parent)
{
    // 进度条在为设置初始值时，值为-1 安装进度条前面一段显示为方头
    setValue(0);                //设置初始进度为0
    setMinimum(0);
    setMaximum(100);
    setFixedHeight(8);
    setFixedWidth(240);
    setTextVisible(false);
}
