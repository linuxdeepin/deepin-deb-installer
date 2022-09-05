// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
