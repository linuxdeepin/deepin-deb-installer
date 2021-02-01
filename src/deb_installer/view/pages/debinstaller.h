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

#ifndef DEBINSTALLER_H
#define DEBINSTALLER_H

#include <DMainWindow>

#include <QPointer>
#include <QSettings>
#include <QStackedLayout>
#include <QApt/DebFile>
#include <QWidget>

DWIDGET_USE_NAMESPACE
class FileChooseWidget;
class DebListModel;
class SingleInstallPage;

using QApt::DebFile;

/**
 * @brief The DebInstaller class
 *
 * 软件包安装器的主窗口
 * 承担各安装界面的切换，包的添加，删除等功能
 * M-V-C 中的 View
 */
class DebInstaller : public Dtk::Widget::DMainWindow
{
    Q_OBJECT

public:
    DebInstaller(QWidget *parent = nullptr);
    virtual ~DebInstaller() Q_DECL_OVERRIDE;

protected:
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;           //按键事件
    void dragEnterEvent(QDragEnterEvent *e) Q_DECL_OVERRIDE;    //拖入事件
    void dropEvent(QDropEvent *e) Q_DECL_OVERRIDE;              //拖入放下事件
    void dragMoveEvent(QDragMoveEvent *e) Q_DECL_OVERRIDE;      //拖进事件
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;        //关闭事件 PS:没有实质的用处，之后删除

private slots:
    /**
     * @brief onPackagesSelected
     * @param packages 安装的包的全路径的列表
     * 添加包时，对包进行处理，去除无效的包，提示已经添加过的包，并根据添加包的数量刷新界面
     */
    void onPackagesSelected(const QStringList &packages);

    /**
     * @brief showInvalidePackageMessage 弹出无效包的消息通知
     */
    void showInvalidePackageMessage();

    /**
     * @brief showInvalidePackageMessage 弹出不是本地包的消息通知
     */
    void showNotLocalPackageMessage();


    /**
     * @brief showPkgExistMessage 弹出包已存在的消息通知
     */
    void showPkgExistMessage();

    /**
     * @brief onNewAppOpen
     * @param pid 进程号
     * @param arguments 要安装的包的全路径的列表
     * 桌面或文管中双击或右键打开时的槽函数
     * 会把后缀为.deb的包传递到onPackageSelected中
     */
    void onNewAppOpen(qint64 pid, const QStringList &arguments);

    /**
     * @brief removePackage
     * @param index 要删除的包的下标
     * 根据传入的下表删除某个包。
     */
    void removePackage(const int index);

    /**
     * @brief showUninstallConfirmPage
     * 卸载按钮的槽函数
     * 显示卸载界面
     */
    void showUninstallConfirmPage();

    /**
     * @brief onUninstallAccepted
     * 卸载界面确认卸载按钮的槽函数
     * 卸载开始时，返回singleInstallPage 并显示卸载进程。
     */
    void onUninstallAccepted();

    /**
     * @brief onUninstallCancel
     * 卸载界面取消卸载按钮的槽函数
     * 取消卸载后返回 singleInstallPage
     */
    void onUninstallCancel();

    /**
     * @brief onStartInstallRequested
     * 安装开始后，所有的关闭按钮都会被禁止
     * SP3新增，解决安装开始时焦点闪现的问题。
     */
    void onStartInstallRequested();

    /**
     * @brief setEnableButton
     * @param bEnable 按钮是否可用标志
     * 根据当前的安装/卸载进程来控制singleInstallPage/multiInstallPage按钮的可用性
     */
    void setEnableButton(bool bEnable);

    /**
     * @brief showHiddenButton
     * 授权取消后显示被隐藏的按钮
     */
    void showHiddenButton();

    /**
     * @brief onAuthing
     * @param authing 按钮是否可用的标志
     * 授权框弹出后，设置当前界面为不可用状态
     */
    void onAuthing(const bool authing);

    /**
     * @brief DealDependResult
     * @param iAuthRes
     * 根据deepin-wine依赖安装的结果处理界面显示效果
     */
    void DealDependResult(int iAuthRes, QString dependName);

    /**
     * @brief changeDragFlag
     * 安装卸载结束后，允许包被拖入程序，并设置关闭按钮可用
     */
    void changeDragFlag();

    /**
     * @brief reset
     * 重置当前工作状态、拖入状态、标题栏、页面暂存区，删除卸载页面
     *
     */
    void reset();

    /**
     * @brief enableCloseButton
     * @param enable 是否禁用关闭按钮
     * 下载依赖时，关闭按钮不可用，下载依赖成功后，依赖按钮可用
     */
    void enableCloseButton(bool enable);

private:
    /**
     * @brief initUI
     * 初始化界面
     */
    void initUI();

    /**
     * @brief initConnections
     * 初始化链接信号和槽
     */
    void initConnections();


    /**
     * @brief refreshSingle 刷新单包安装界面
     */
    void refreshSingle();

    /**
     * @brief single2Multi 刷新批量安装界面
     */
    void single2Multi();

    /**
     * @brief refreshMulti 刷新批量安装model
     */
    void refreshMulti();

    /**
     * @brief appendPackageStart 正在添加多个包的界面处理函数
     */
    void appendPackageStart();

    /**
     * @brief appendFinished 批量添加结束界面处理函数
     */
    void appendFinished();

    /**
     * @brief MulRefreshPage 刷新批量安装model
     */
    void MulRefreshPage();

    //Disable/enable close button and exit in menu
    /**
     * @brief disableCloseAndExit
     * 设置退出和关闭按钮为不可用
     */
    void disableCloseAndExit();
    /**
     * @brief enableCloseAndExit
     * 设置退出和关闭按钮可用
     */
    void enableCloseAndExit();

    /**
     * @brief backToSinglePage 返回单包安装界面
     * @return SingleInstallPage* SingleInstallPage的指针
     */
    SingleInstallPage *backToSinglePage();

    /**
     * @brief checkSuffix 检查文件后缀
     * @param filePath 文件路径
     * @return 文件后缀是否是.deb
     */
    bool checkSuffix(QString filePath);

private:
    DebListModel *m_fileListModel;                  //model 类
    FileChooseWidget *m_fileChooseWidget;           //文件选择的widget
    QStackedLayout *m_centralLayout;                //单包、批量、卸载的widget
    QPointer<QWidget> m_lastPage;                   //存放上一个页面的指针

    int m_dragflag = -1;                            //当前是否允许拖入的标志位

    int m_iOptionWindowFlag = 0;                    //判断菜单栏是否手动弹出
    bool bTabFlag = false;                          //Control focus is re-identified from titlebar
    bool bActiveWindowFlag = true;                  //Window activation id
    int m_Filterflag = -1;                          //Determine the current page      choose:-1;multiple:1;single:2;uninstall:3

    QPointer<QWidget> m_UninstallPage;              //Store uninstall page

    bool packageAppending = false;
};

#endif  // DEBINSTALLER_H
