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

#ifndef SINGLEINSTALLPAGE_H
#define SINGLEINSTALLPAGE_H

#include "view/widgets/infocontrolbutton.h"
#include "view/widgets/installprocessinfoview.h"
#include "view/widgets/debinfolabel.h"

#include <DLabel>
#include <DProgressBar>
#include <DPushButton>
#include <DTextEdit>
#include <DSpinner>
#include <DDialog>
#include <DCommandLinkButton>

#include <QWidget>
#include <QProcess>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include <QApt/DebFile>
#include <unistd.h>
#include <stdlib.h>

class DebListModel;
class WorkerProgress;
class SingleInstallPage : public QWidget
{
    Q_OBJECT

public:
    explicit SingleInstallPage(DebListModel *model, QWidget *parent = nullptr);

public:
    /**
     * @brief afterGetAutherFalse 授权失败或者取消后的界面处理函数
     */
    void afterGetAutherFalse();

    /**
     * @brief setEnableButton 授权框弹出/取消后 ，设置按钮禁用/可用
     * @param bEnable
     */
    void setEnableButton(bool bEnable);

    /**
     * @brief DealDependResult 根据依赖安装的进程标识，处理依赖安装的流程显示
     * @param iAuthRes  依赖安装的集成标识
     * @param dependName    安装的依赖名称
     */
    void DealDependResult(int iAuthRes, QString dependName);

public slots:
    /**
     * @brief uninstallCurrentPackage 显示卸载页面
     */
    void uninstallCurrentPackage();

protected:
    void paintEvent(QPaintEvent *event) override;

signals:

    /**
     * @brief back 返回fileChooseWidget界面
     */
    void back() const;

    /**
     * @brief requestUninstallConfirm 卸载按钮信号，显示卸载页面
     */
    void requestUninstallConfirm() const;

private:

    /**
     * @brief The Operate enum 当前进行的操作
     */
    enum Operate {
        Install,        //安装
        Uninstall,      //卸载
        Reinstall       //重新安装
    };

private:

    /**
     * @brief initUI 初始化UI
     */
    void initUI();

    /**
     * @brief initContentLayout 初始化主布局
     */
    void initContentLayout();

    /**
     * @brief initInstallWineLoadingLayout 初始化wine依赖安装加载动画布局
     */
    void initInstallWineLoadingLayout();

    /**
     * @brief initPkgInfoView 初始化 包信息视图布局
     * @param fontinfosize  字体大小
     */
    void initPkgInfoView(int fontinfosize);

    /**
     * @brief initPkgInstallProcessView 初始化安装进程信息布局
     * @param fontinfosize  字体大小
     */
    void initPkgInstallProcessView(int fontinfosize);

    /**
     * @brief initConnections 初始化链接信号与槽
     */
    void initConnections();

    /**
     * @brief initLabelWidth 根据字体的大小调整标签的宽度
     * @param fontinfo 字体大小
     * @return  label的宽度
     */
    int initLabelWidth(int fontinfo);

    /**
     * @brief initControlAccessibleName 初始化控件的AccessibleName
     */
    void initControlAccessibleName();

private:
    /**
     * @brief initTabOrder 设置tab切换焦点的顺序
     */
    void initTabOrder();

    /**
     * @brief initButtonFocusPolicy 设置按钮的焦点策略
     */
    void initButtonFocusPolicy();

    /**
     * @brief initButtonAutoDefault 设置按钮的回车触发
     */
    void initButtonAutoDefault();

private:
    /**
     * @brief showPackageInfo 获取并显示deb包的信息
     */
    void showPackageInfo();

    /**
     * @brief setAuthConfirm 依赖下载授权确认后的界面处理函数
     * @param dependName
     */
    void setAuthConfirm(QString dependName);

    /**
     * @brief setAuthBefore 依赖下载授权前的界面处理函数
     */
    void setAuthBefore();

    /**
     * @brief setCancelAuthOrAuthDependsErr 授权取消或依赖安装结果显示的界面处理函数
     */
    void setCancelAuthOrAuthDependsErr();

    /**
     * @brief setAuthDependsSuccess 暂未使用
     */
    void setAuthDependsSuccess();

private slots:

    /**
     * @brief install 点击安装按钮后的界面处理，并触发安装进程
     */
    void install();

    /**
     * @brief reinstall 点击重新安装按钮后的界面处理，并触发安装进程
     */
    void reinstall();

    /**
     * @brief showInfomation 显示安装详细信息的视图
     */
    void showInfomation();

    /**
     * @brief hideInfomation 隐藏安装过程详细信息的视图
     */
    void hideInfomation();

    /**
     * @brief showInfo 显示信息的界面调整
     */
    void showInfo();

    /**
     * @brief onOutputAvailable 向安装详细信息展示窗口添加安装进度信息
     * @param output 安装过程信息
     */
    void onOutputAvailable(const QString &output);

    /**
     * @brief onWorkerFinished 安装或卸载结束后的界面调整
     */
    void onWorkerFinished();

    /**
     * @brief onWorkerProgressChanged 安装进度变化槽函数
     * @param progress 实际的安装进度
     */
    void onWorkerProgressChanged(const int progress);

private:
    Operate m_operate;                                  //当前的操作
    bool m_workerStarted;                               //安装是否开始
    bool m_upDown;                                      //当前是详细信息是展开还是收缩

    DebListModel *m_packagesModel;                      //model类

    QWidget *m_contentFrame;                            //主布局
    QWidget *m_itemInfoFrame;                           //包信息框架
    QWidget *m_progressFrame;                           //安装进度框架

    DLabel *m_packageIcon;                              //包的图标
    DebInfoLabel *m_packageName;                        //包名
    DebInfoLabel *m_packageVersion;                     //包的版本
    DLabel *m_packageDescription;                       //包的描述
    DebInfoLabel *m_tipsLabel;                          //包的状态提示label

    WorkerProgress *m_progress;                         //安装进度条
    InstallProcessInfoView *m_installProcessView;       //安装进度的详细信息

    InfoControlButton *m_infoControlButton;             //安装信息显示/隐藏控制按钮
    DPushButton *m_installButton;                       //安装按钮
    DPushButton *m_uninstallButton;                     //卸载按钮
    DPushButton *m_reinstallButton;                     //重新安装按钮
    DPushButton *m_confirmButton;                       //确认按钮
    DPushButton *m_backButton;                          //返回按钮
    DPushButton *m_doneButton;                          //完成按钮

private:
    QVBoxLayout *m_contentLayout;                       //
    QVBoxLayout *m_centralLayout;                       //主布局

    QString m_description;                              //包的描述文本
    QString packagename_description;                    //包名的文本
    QString packageversion_description;                 //包版本的文本

    DSpinner *m_pDSpinner;                              //依赖安装加载动画

    // fix bug:33999 change DebInfoLabel to DCommandLinkButton for Activity color
    DCommandLinkButton *m_pLoadingLabel;                //依赖安装提示信息
    int dependAuthStatu = -1; //存储依赖授权状态
};

#endif  // SINGLEINSTALLPAGE_H
