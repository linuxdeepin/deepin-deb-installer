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

#ifndef DEBINSTALLER_H
#define DEBINSTALLER_H

#include <DMainWindow>

#include <QPointer>
#include <QSettings>
#include <QStackedLayout>
#include <QApt/DebFile>
#include <QWidget>

class FileChooseWidget;
class DebListModel;
class SingleInstallPage;
class UninstallConfirmPage;

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
    void dragEnterEvent(QDragEnterEvent *e) Q_DECL_OVERRIDE;    //拖入事件
    void dropEvent(QDropEvent *e) Q_DECL_OVERRIDE;              //拖入放下事件
    void dragMoveEvent(QDragMoveEvent *e) Q_DECL_OVERRIDE;      //拖进事件
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;        //关闭事件 PS:没有实质的用处，之后删除

private slots:
    /**
     * @brief slotPackagesSelected
     * @param packages 安装的包的全路径的列表
     * 添加包时，对包进行处理，去除无效的包，提示已经添加过的包，并根据添加包的数量刷新界面
     */
    void slotPackagesSelected(const QStringList &packages);

    /**
     * @brief slotShowInvalidePackageMessage 弹出无效包的消息通知
     */
    void slotShowInvalidePackageMessage();

    /**
     * @brief slotShowNotLocalPackageMessage 弹出不是本地包的消息通知
     */
    void slotShowNotLocalPackageMessage();

    /**
     * @brief slotShowPkgExistMessage 弹出包已存在的消息通知
     */
    void slotShowPkgExistMessage();

    /**
     * @brief slotShowPkgRemovedMessage 弹出包被移动的消息通知
     */
    void slotShowPkgRemovedMessage(QString packageName);

    /**
     * @brief slotRemovePackage
     * @param index 要删除的包的下标
     * 根据传入的下表删除某个包。
     */
    void slotRemovePackage(const int index);

    /**
     * @brief slotShowUninstallConfirmPage
     * 卸载按钮的槽函数
     * 显示卸载界面
     */
    void slotShowUninstallConfirmPage();

    /**
     * @brief slotUninstallAccepted
     * 卸载界面确认卸载按钮的槽函数
     * 卸载开始时，返回singleInstallPage 并显示卸载进程。
     */
    void slotUninstallAccepted();

    /**
     * @brief slotUninstallCancel
     * 卸载界面取消卸载按钮的槽函数
     * 取消卸载后返回 singleInstallPage
     */
    void slotUninstallCancel();


    /**
     * @brief slotSetEnableButton
     * @param bEnable 按钮是否可用标志
     * 根据当前的安装/卸载进程来控制singleInstallPage/multiInstallPage按钮的可用性
     */
    void slotSetEnableButton(bool bEnable);

    /**
     * @brief slotShowHiddenButton
     * 授权取消后显示被隐藏的按钮
     */
    void slotShowHiddenButton();

    /**
     * @brief slotSetAuthingStatus
     * @param authing 按钮是否可用的标志
     * 授权框弹出后，设置当前界面为不可用状态
     */
    void slotSetAuthingStatus(const bool authing);

    /**
     * @brief slotDealDependResult
     * @param iAuthRes
     * 根据deepin-wine依赖安装的结果处理界面显示效果
     */
    void slotDealDependResult(int iAuthRes, QString dependName);

    /**
     * @brief slotChangeDragFlag
     * 安装卸载结束后，允许包被拖入程序，并设置关闭按钮可用
     */
    void slotChangeDragFlag();

    /**
     * @brief slotReset
     * 重置当前工作状态、拖入状态、标题栏、页面暂存区，删除卸载页面
     *
     */
    void slotReset();

    /**
     * @brief slotEnableCloseButton
     * @param enable 是否禁用关闭按钮
     * 下载依赖时，关闭按钮不可用，下载依赖成功后，依赖按钮可用
     */
    void slotEnableCloseButton(bool enable);

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
    DebListModel        *m_fileListModel      = nullptr;                  //model 类
    FileChooseWidget    *m_fileChooseWidget   = nullptr;           //文件选择的widget
    UninstallConfirmPage *m_uninstallPage     = nullptr;

    QPointer<QWidget>   m_lastPage;                   //存放上一个页面的指针
    QStackedLayout      *m_centralLayout      = nullptr;                //单包、批量、卸载的widget

    int m_dragflag          = -1;                            //当前是否允许拖入的标志位

    int m_iOptionWindowFlag = 0;                    //判断菜单栏是否手动弹出
    bool bTabFlag           = false;                          //Control focus is re-identified from titlebar
    bool bActiveWindowFlag  = true;                  //Window activation id
    int m_Filterflag        = -1;                          //Determine the current page      choose:-1;multiple:1;single:2;uninstall:3

    bool m_packageAppending = false;
};

#endif  // DEBINSTALLER_H
