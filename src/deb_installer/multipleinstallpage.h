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

#ifndef MULTIPLEINSTALLPAGE_H
#define MULTIPLEINSTALLPAGE_H

#include "infocontrolbutton.h"
#include "droundbgframe.h"
#include "installprocessinfoview.h"

#include <QPropertyAnimation>
#include <QWidget>

#include <DPushButton>
#include <DProgressBar>
#include <DSpinner>
#include <DCommandLinkButton>

class PackagesListView;
class DebListModel;
class WorkerProgress;
class MultipleInstallPage : public QWidget
{
    Q_OBJECT

public:
    explicit MultipleInstallPage(DebListModel *model, QWidget *parent = nullptr);

    /**
     * @brief setEnableButton 根据授权框的弹出/取消，设置安装按钮禁用/请
     * @param bEnable 按钮启用与禁用的标识
     */
    void setEnableButton(bool bEnable);

    /**
     * @brief afterGetAutherFalse 授权结束或取消后处理界面情况
     */
    void afterGetAutherFalse();

    /**
     * @brief setScrollBottom 滚动item到底部或index的位置
     * @param index 要滚到的index
     */
    void setScrollBottom(int index);

    /**
     * @brief DealDependResult  处理依赖处理授权的结果
     * @param iAuthRes          依赖处理的结果
     * @param dependName        依赖的名称
     */
    void DealDependResult(int iAuthRes, QString dependName);

signals:
    /**
     * @brief back 返回到fileChooseWidget界面的信号
     */
    void back() const;

    /**
     * @brief requestRemovePackage  删除包的信号
     * @param index 要删除的包的下标
     */
    void requestRemovePackage(const int index) const;

private slots:

    /**
     * @brief onScrollSlotFinshed   滚动item的槽函数
     */
    void onScrollSlotFinshed();

    /**
     * @brief onWorkerFinshed   批量安装结束，调整页面展示效果
     */
    void onWorkerFinshed();

    /**
     * @brief onOutputAvailable 批量安装过程中添加安装信息
     * @param output    安装过程信息
     */
    void onOutputAvailable(const QString &output);

    /**
     * @brief onProgressChanged 批量安装进度变化
     * @param progress 已经增加的进度
     */
    void onProgressChanged(const int progress);

    /**
     * @brief onRequestRemoveItemClicked 在批量安装的包的时候删除某个包
     * @param index 要删除的包的下标
     */
    void onRequestRemoveItemClicked(const QModelIndex &index);

    /**
     * @brief showInfo  处理展示详细信息的界面展示
     */
    void showInfo();

    /**
     * @brief hideInfo  处理隐藏详细信息的界面展示
     */
    void hideInfo();

    /**
     * @brief onAutoScrollInstallList  自动滚动到当前正在安装的包
     * @param opIndex  当前正在安装的包的位置
     */
    void onAutoScrollInstallList(int opIndex);

    /**
     * @brief hiddenCancelButton    安装开始后处理界面展示情况
     */
    void hiddenCancelButton();

private:

    /**
     * @brief initUI 初始化界面布局
     */
    void initUI();

    /**
     * @brief initConnections 初始化链接
     */
    void initConnections();

    /**
     * @brief initContentLayout 初始化中心layout
     */
    void initContentLayout();

    /**
     * @brief initTabOrder 初始化切换顺序
     */
    void initTabOrder();

    /**
     * @brief setButtonFocusPolicy 设置按钮的焦点策略
     * @param focusPolicy 是否启用焦点
     */
    void setButtonFocusPolicy();

    /**
     * @brief setButtonAutoDefault 设置按钮可以被enter和Return键触发
     */
    void setButtonAutoDefault();

    /**
     * @brief initControlAccessibleName 给控件添加AccessibleName
     */
    void initControlAccessibleName();


private:
    DebListModel *m_debListModel;                       //listModel类

    DRoundBgFrame *m_appsListViewBgFrame;               //listView的背景frame 修改listView的背景样式
    QWidget *m_contentFrame;                            //applistview  infoControlButton的frame
    QWidget *m_processFrame;                            //process 的frame
    QVBoxLayout *m_contentLayout;                       //applistview  infoControlButton的布局
    QVBoxLayout *m_centralLayout;                       //主布局

    PackagesListView *m_appsListView;                   //listView

    InstallProcessInfoView *m_installProcessInfoView;   //安装进程信息显示窗口

    WorkerProgress *m_installProgress;                  //进度显示
    QPropertyAnimation *m_progressAnimation;            //进度动画

    InfoControlButton *m_infoControlButton;             //展开收缩控制按钮
    DPushButton *m_installButton;                       //安装按钮
    DPushButton *m_backButton;                          //返回文件选择窗口的按钮
    DPushButton *m_acceptButton;                        //确认按钮

    // fix bug:33999 change DebInfoLabel to DCommandLinkButton for Activity color
    DCommandLinkButton *m_tipsLabel;                    //依赖安装提示按钮
    DSpinner *m_dSpinner;                               //依赖安装动画

    int m_index = -1;                                   //当前添加的index

    bool m_upDown = true;                               //展开收缩的标识
    //install:1    finish:2
    int m_currentFlag = 1;                              //此变量被废弃
    int m_MouseBtnRelease = 0;                          //此变量被废弃
};

#endif // MULTIPLEINSTALLPAGE_H
