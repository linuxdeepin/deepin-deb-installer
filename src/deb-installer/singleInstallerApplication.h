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


#ifndef SINGLEFONTAPPLICATION_H
#define SINGLEFONTAPPLICATION_H

#include <DApplication>
#include <DMainWindow>
#include <QCommandLineParser>

DWIDGET_USE_NAMESPACE

class SingleInstallerApplication : public DApplication
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.DebInstaller")
public:
    explicit SingleInstallerApplication(int &argc, char **argv);
    /**
     * @brief 激活软件包安装器窗口
     * 
     */
    void activateWindow();

    /**
     * @brief 解析命令行参数
     * 
     * @return true 
     * @return false 
     */
    bool parseCmdLine();

public slots:

    /**
     * @brief 启动软件包安装器
     * 
     * @param debPathList 是否有传递的包
     * @return Q_SCRIPTABLE 
     */
    Q_SCRIPTABLE void InstallerDeb(const QStringList &debPathList);

private:
    QStringList m_selectedFiles;

    QScopedPointer<DMainWindow> m_qspMainWnd;  // MainWindow ptr
};

#endif // SINGLEFONTAPPLICATION_H
