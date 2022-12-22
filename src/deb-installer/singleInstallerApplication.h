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
    /**
     * @brief dbus安装包
     *
     * @param debPathList 包名
     * @return Q_SCRIPTABLE
     */
    Q_SCRIPTABLE QString InstallerDebPackge(const QString &debPath);
    /**
     * @brief dbus卸载包
     *
     * @param debPathList 包名
     * @return Q_SCRIPTABLE
     */
    Q_SCRIPTABLE QString unInstallDebPackge(const QString &debPath);
    /**
     * @brief 包安装状态
     *
     * @param debPath 是否有传递的包
     * @return Q_SCRIPTABLE
     */
    Q_SCRIPTABLE int checkInstallStatus(const QString &debPath);
    /**
     * @brief 包依赖信息
     *
     * @param debPath 是否有传递的包
     * @return Q_SCRIPTABLE
     */
    Q_SCRIPTABLE int checkDependsStatus(const QString &debPath);
    /**
     * @brief 包的数字签名
     *
     * @param debPath 是否有传递的包
     * @return Q_SCRIPTABLE
     */
    Q_SCRIPTABLE int checkDigitalSignature(const QString &debPath);
    /**
     * @brief 包信息
     *
     * @param debPath 是否有传递的包
     * @return Q_SCRIPTABLE
     */
    Q_SCRIPTABLE QString getPackageInfo(const QString &debPath);


private:
    QStringList m_selectedFiles;

    QScopedPointer<DMainWindow> m_qspMainWnd;  // MainWindow ptr

    bool bIsDbus = false;
};

#endif // SINGLEFONTAPPLICATION_H
