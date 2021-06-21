/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      zhangkai <zhangkai@uniontech.com>
* Maintainer:  zhangkai <zhangkai@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "debinstaller.h"

#include <DDialog>
#include <DMainWindow>

DWIDGET_USE_NAMESPACE

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void initUI();

    void setWindowTransparent();

signals:

public slots:
    /**
     * @brief onPackagesSelected
     * @param packages 安装的包的全路径的列表
     * 添加包时，对包进行处理，去除无效的包，提示已经添加过的包，并根据添加包的数量刷新界面
     */
    void onPackagesSelected(const QStringList &packages);
private:
    DebInstaller *m_mainWindow;

};

#endif // MAINWINDOW_H
