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
#include "TitleBarFocusMonitor.h"

TitleBarFocusMonitor::TitleBarFocusMonitor(QWidget *widget)
{
    m_pOptionWidget = widget;
}

/**
 * @brief TitleBarFocusMonitor::run 监测标题栏菜单键Focus 解决标题栏焦点闪现的问题
 */
void TitleBarFocusMonitor::run()
{
    m_monitoring = true;
    qDebug() << "TitleBarFocusMonitor:" << "Monitor option Widget";
    while (m_monitoring) {
        if (m_pOptionWidget->hasFocus()) {
            m_pOptionWidget->clearFocus();
            qDebug() << "TitleBarFocusMonitor:" << "clear Focus";
        }
    }
    qDebug() << "TitleBarFocusMonitor:" "stop monitor titlebar focus";
}

/**
 * @brief TitleBarFocusMonitor::stopMonitor 停止监测焦点。
 */
void TitleBarFocusMonitor::stopMonitor()
{
    m_monitoring = false;
}
