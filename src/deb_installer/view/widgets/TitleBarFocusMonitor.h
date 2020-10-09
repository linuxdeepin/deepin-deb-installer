/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
*
* Author:     cuizhen <cuizhen@uniontech.com>
* Maintainer:  cuizhen <cuizhen@uniontech.com>
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
#ifndef TITLEBARFOCUSMONITOR_H
#define TITLEBARFOCUSMONITOR_H

#include <QObject>
#include <QtWidgets>
#include <QThread>

/**
 * @brief The TitleBarFocusMonitor class
 * 监测焦点闪现问题
 * fix bug: https://pms.uniontech.com/zentao/bug-view-46813.html
 * 传入的参数是 Titlebar中的菜单键。
 */
class TitleBarFocusMonitor : public QThread
{
    Q_OBJECT
public:
    TitleBarFocusMonitor(QWidget *widget);
    /**
     * @brief run 线程执行函数
     * 不断清除标题栏菜单键上的焦点，直到开始安装。
     */
    void run();

    /**
     * @brief stopMonitor 停止监测线程。调用此函数后，线程会停止不再监测标题栏菜单键
     */
    void stopMonitor();

private:
    QWidget *m_pOptionWidget;   //传入的标题栏菜单控件
    bool m_monitoring = true;   //线程停止的操控标识
};

#endif // TITLEBARFOCUSMONITOR_H
