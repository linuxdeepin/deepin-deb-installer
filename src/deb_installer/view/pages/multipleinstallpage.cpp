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

#include "multipleinstallpage.h"
#include "model/deblistmodel.h"
#include "model/packagelistview.h"
#include "model/packageslistdelegate.h"
#include "view/widgets/workerprogress.h"
#include "utils/utils.h"

#include <QApplication>
#include <QPropertyAnimation>
#include <QTimer>
#include <QVBoxLayout>

#include <DLabel>

MultipleInstallPage::MultipleInstallPage(DebListModel *model, QWidget *parent)
    : QWidget(parent)
    , m_debListModel(model)
    , m_appsListViewBgFrame(new DRoundBgFrame(this, 10, 0))
    , m_contentFrame(new QWidget(this))
    , m_processFrame(new QWidget(this))
    , m_contentLayout(new QVBoxLayout())
    , m_centralLayout(new QVBoxLayout())
    , m_appsListView(new PackagesListView(this))
    , m_installProcessInfoView(new InstallProcessInfoView(440, 190, this))
    , m_installProgress(nullptr)
    , m_progressAnimation(nullptr)
    , m_infoControlButton(new InfoControlButton(tr("Show details"), tr("Collapse"), this))
    , m_installButton(new DPushButton(this))
    , m_backButton(new DPushButton(this))
    , m_acceptButton(new DPushButton(this))

      // fix bug:33999 change DButton to DCommandLinkButton for Activity color
    , m_tipsLabel(new DCommandLinkButton("", this))
    , m_dSpinner(new DSpinner(this))
{
    initControlAccessibleName();                //自动化测试
    initContentLayout();                        //初始化主布局
    initUI();                                   //初始化控件
    initConnections();                          //初始化链接
    initTabOrder();                             //初始化焦点切换顺序
    //添加后默认可以调用右键删除菜单。
    m_appsListView->setRightMenuShowStatus(true);//设置批量安装右键删除菜单可用
}

/**
 * @brief MultipleInstallPage::initControlAccessibleName 添加控件的AccessibleName
 */
void MultipleInstallPage::initControlAccessibleName()
{
    //添加contentFrame AccessibleName
    m_contentFrame->setObjectName("contentFrame");
    m_contentFrame->setAccessibleName("contentFrame");

    //添加processFrame AccessibleName
    m_processFrame->setObjectName("processFrame");
    m_processFrame->setAccessibleName("processFrame");

    //添加 m_installProcessInfoView AccessibleName
    m_installProcessInfoView->setObjectName("InstallProcessInfoView");
    m_installProcessInfoView->setAccessibleName("InstallProcessInfoView");

    //添加 infoControlButton AccessibleName
    m_infoControlButton->setObjectName("InfoControlButton");
    m_infoControlButton->setAccessibleName("InfoControlButton");

    //添加 installButton AccessibleName
    m_installButton->setObjectName("MultipageInstallButton");
    m_installButton->setAccessibleName("MultipageInstallButton");

    //添加 backButton AccessibleName
    m_backButton->setObjectName("MultipageBackButton");
    m_backButton->setAccessibleName("MultipageBackButton");

    //添加 acceptButton AccessibleName
    m_acceptButton->setObjectName("MultipageAcceptButton");
    m_acceptButton->setAccessibleName("MultipageAcceptButton");

    //添加 m_infoControlButton AccessibleName
    m_infoControlButton->setObjectName("MultipageInfoControlButton");
    m_infoControlButton->setAccessibleName("MultipageInfoControlButton");

    //添加 appListViewFrame AccessibleName
    m_appsListViewBgFrame->setObjectName("AppListViewBgFrame");
    m_appsListViewBgFrame->setAccessibleName("AppListViewBgFrame");

    //添加 tipsLabel AccessibleName
    m_tipsLabel->setObjectName("TipsCommandLinkButton");
    m_tipsLabel->setAccessibleName("TipsCommandLinkButton");
}

/**
 * @brief MultipleInstallPage::initContentLayout 初始化主布局
 */
void MultipleInstallPage::initContentLayout()
{
    //子布局添加10px 的空格
    m_contentLayout->addSpacing(10);
    //子布局设置控件间的间距为0
    m_contentLayout->setSpacing(0);
    //子布局设定上下左右的编剧
    m_contentLayout->setContentsMargins(10, 0, 10, 0);

    //添加子布局到主布局中
    m_contentFrame->setLayout(m_contentLayout);
    m_centralLayout->addWidget(m_contentFrame);

    //主布局添加间距
    m_centralLayout->setSpacing(0);

    //主布局设置边距
    m_centralLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(m_centralLayout);

//#define SHOWBGCOLOR
#ifdef SHOWBGCOLOR
    m_contentFrame->setStyleSheet("QFrame{background: cyan}");
#endif
}

/**
 * @brief MultipleInstallPage::initUI 初始化各个UI控件
 * 对PackageListView 进程重绘
 * 设置appListView的背景样式
 * 设置appListView的展示效果
 *
 * 初始化安装按钮、确认按钮、返回按钮
 * 设置各个按钮的显示效果
 * 设置各个按钮的字体的展示效果
 * 处理各个控件的焦点
 * 初始化安装进度控件
 * 初始化依赖安装动画控件
 *
 * 各个小布局的规制
 */
void MultipleInstallPage::initUI()
{
    this->setFocusPolicy(Qt::NoFocus);
    //直接传入debListModel 修复多次创建packageManager导致崩溃的问题
    PackagesListDelegate *delegate = new PackagesListDelegate(m_debListModel, m_appsListView);

    //获取currentIndex的坐标位置，用于键盘触发右键菜单
    connect(delegate, &PackagesListDelegate::sigIndexAndRect, m_appsListView, &PackagesListView::getPos);
    //fix bug:33730
    m_appsListViewBgFrame->setFixedSize(460, 186/* + 10*/ + 5);

    // listview的布局
    QVBoxLayout *appsViewLayout = new QVBoxLayout();
    appsViewLayout->setSpacing(0);

    //设置边距
    appsViewLayout->setContentsMargins(0, 0, 0, 0);
    m_appsListViewBgFrame->setLayout(appsViewLayout);

    //applistview 绑定model
    m_appsListView->setModel(m_debListModel);
    m_appsListView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //监听字体大小变化,设置高度
    connect(m_appsListView, &PackagesListView::setItemHeight, [=](int height) { delegate->getItemHeight(height); });

    //使用代理重绘listView
    m_appsListView->setItemDelegate(delegate);

    //设置焦点策略
    m_appsListView->setFocusPolicy(Qt::TabFocus);
    appsViewLayout->addSpacing(10);
    appsViewLayout->addWidget(m_appsListView);

    m_installButton->setMinimumSize(120, 36);     //设置安装按钮的大小
    m_acceptButton->setMinimumSize(120, 36);      //设置确认按钮的大小
    m_backButton->setMinimumSize(120, 36);        //设置返回按钮的大小

    m_installButton->setText(tr("Install"));    //设置安装按钮的提示语
    m_acceptButton->setText(tr("Done"));        //设置完成按钮的提示
    m_acceptButton->setVisible(false);          //默认隐藏完成按钮
    m_backButton->setText(tr("Back"));          //设置返回按钮的提示
    m_backButton->setVisible(false);            //隐藏返回按钮


    QString mediumFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansMedium);

    Utils::bindFontBySizeAndWeight(m_installButton, mediumFontFamily, 14, QFont::Medium);       //调整安装按钮的字体
    Utils::bindFontBySizeAndWeight(m_acceptButton, mediumFontFamily, 14, QFont::Medium);        //调整完成按钮的字体
    Utils::bindFontBySizeAndWeight(m_backButton, mediumFontFamily, 14, QFont::Medium);          //调整返回按钮的字体

    //把按钮的焦点策略整合到一起，便于解决焦点闪现的问题
    setButtonFocusPolicy();

    //设置按钮可以被回车触发
    setButtonAutoDefault();

    //修复依赖安装提示语会有焦点的问题。
    m_tipsLabel->setFocusPolicy(Qt::NoFocus);
    m_tipsLabel->setMinimumHeight(24);
    QString fontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    Utils::bindFontBySizeAndWeight(m_tipsLabel, fontFamily, 12, QFont::ExtraLight);             //调整wine依赖安装提示的字体

    m_dSpinner->setMinimumSize(20, 20);                                                         //设置wine安装依赖的安装动画的大小
    m_dSpinner->hide();                                                                         //隐藏安装动画

    // 设置安装信息的信息展示框
    m_installProcessInfoView->setVisible(false);
    m_installProcessInfoView->setAcceptDrops(false);
    m_installProcessInfoView->setFixedHeight(200);                                              //设置固定高度
    m_installProcessInfoView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_infoControlButton->setVisible(false);                                                     //详细按钮展开收缩默认不可见

    //进度布局
    QVBoxLayout *progressFrameLayout = new QVBoxLayout();
    progressFrameLayout->setSpacing(0);
    progressFrameLayout->setContentsMargins(0, 0, 0, 0);                                        //设置边距
    m_processFrame->setLayout(progressFrameLayout);
    m_installProgress = new WorkerProgress(this);                                               //进度条初始化
    m_progressAnimation = new QPropertyAnimation(m_installProgress, "value", this);             //进度条动画
    progressFrameLayout->addStretch();
    progressFrameLayout->addWidget(m_installProgress);
    progressFrameLayout->addSpacing(28);
    progressFrameLayout->setAlignment(m_installProgress, Qt::AlignHCenter);                     //进度条居中

    m_processFrame->setVisible(false);                                                          //进度条默认隐藏
    m_processFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_processFrame->setMinimumHeight(53);                                                         //设置固定高度

    // 按钮布局
    QVBoxLayout *btnsFrameLayout = new QVBoxLayout();
    btnsFrameLayout->setSpacing(0);
    btnsFrameLayout->setContentsMargins(0, 0, 0, 0);                                            //设置边距

    //按钮布局为水平布局
    QHBoxLayout *btnsLayout = new QHBoxLayout();
    btnsLayout->addStretch();                                                                   //前后增加弹簧
    btnsLayout->addWidget(m_installButton);                                                     //添加安装按钮
    btnsLayout->addWidget(m_backButton);                                                        //添加返回按钮
    btnsLayout->addWidget(m_acceptButton);                                                      //添加完成按钮
    btnsLayout->setSpacing(20);                                                                 //设置按钮间的间距为20px
    btnsLayout->addStretch();
    btnsLayout->setContentsMargins(0, 0, 0, 20);                                                //底部间距30

    QWidget *btnsFrame = new QWidget(this);
    btnsFrameLayout->addWidget(m_processFrame);                                                 //进度布局添加到btn布局中（二者互斥，一定不能同时出现）
    btnsFrameLayout->addStretch();
    btnsFrameLayout->addLayout(btnsLayout);                                                     //添加按钮布局
    btnsFrame->setLayout(btnsFrameLayout);

    m_contentLayout->addWidget(m_appsListViewBgFrame, Qt::AlignHCenter);                        //主布局添加listView frame并居中
    m_contentLayout->addWidget(m_infoControlButton);                                            //主布局添加infoControlButton
    m_contentLayout->setAlignment(m_infoControlButton, Qt::AlignHCenter);                       //居中显示infoControlButton
    m_contentLayout->addWidget(m_installProcessInfoView);                                       //添加详细信息框

    m_contentLayout->addSpacing(37);
    m_contentLayout->addWidget(m_dSpinner);                                                     //添加依赖安装加载动画
    m_contentLayout->addSpacing(4);
    m_contentLayout->addWidget(m_tipsLabel);                                                    //添加依赖安装提示
    m_contentLayout->addSpacing(20);

    //fix bug:33999 keep tips in the middle
    m_contentLayout->setAlignment(m_tipsLabel, Qt::AlignCenter);                                //设置提示居中
    m_tipsLabel->setVisible(false);                                                             //依赖安装提示默认不可见

    m_contentLayout->addWidget(btnsFrame);
}
/**
 * @brief MultipleInstallPage::initTabOrder 初始化tab焦点切换顺序。
 */
void MultipleInstallPage::initTabOrder()
{
    // 修改焦点切换的顺序
    QWidget::setTabOrder(m_appsListView, m_infoControlButton);
    //焦点从infoCommandLinkButton直接切换到安装或返回按钮
    QWidget::setTabOrder(m_infoControlButton->controlButton(), m_installButton);
    QWidget::setTabOrder(m_infoControlButton->controlButton(), m_backButton);
    //焦点从返回按钮可以直接切换到确认按钮
    QWidget::setTabOrder(m_backButton, m_acceptButton);
}

/**
 * @brief MultipleInstallPage::setButtonFocusPolicy 设置按钮的焦点策略
 * @param focusPolicy 是否启用焦点
 */
void MultipleInstallPage::setButtonFocusPolicy()
{
    // 修改焦点控制 启用焦点设置为TabFouce
    auto focus = Qt::TabFocus;

    //设置安装、返回、确认按钮的焦点策略
    m_installButton->setFocusPolicy(focus);
    m_acceptButton->setFocusPolicy(focus);
    m_backButton->setFocusPolicy(focus);

    //设置ControlButton的焦点策略
    m_infoControlButton->controlButton()->setFocusPolicy(focus);
}

/**
 * @brief MultipleInstallPage::setButtonAutoDefault  设置按钮可以被enter和Return键触发
 */
void MultipleInstallPage::setButtonAutoDefault()
{
    //增加键盘enter控制按钮
    m_installButton->setAutoDefault(true);
    m_acceptButton->setAutoDefault(true);
    m_backButton->setAutoDefault(true);
}

void MultipleInstallPage::initConnections()
{
    // 详细信息展开
    connect(m_infoControlButton, &InfoControlButton::expand, this, &MultipleInstallPage::showInfo);

    //详细信息收缩
    connect(m_infoControlButton, &InfoControlButton::shrink, this, &MultipleInstallPage::hideInfo);

    //开始安装
    connect(m_installButton, &DPushButton::clicked, m_debListModel, &DebListModel::installPackages);

    //开始安装后隐藏安装按钮等
    connect(m_installButton, &DPushButton::clicked, this, &MultipleInstallPage::hiddenCancelButton);

    //返回到文件选择窗口
    connect(m_backButton, &DPushButton::clicked, this, &MultipleInstallPage::back);

    //退出应用程序
    connect(m_acceptButton, &DPushButton::clicked, qApp, &QApplication::quit);

    //在listView中删除了某一个包，需要在PackageManager中同步删除
    connect(m_appsListView, &PackagesListView::onRemoveItemClicked, this, &MultipleInstallPage::onRequestRemoveItemClicked);

    //安装过程中进度发生变化
    connect(m_debListModel, &DebListModel::workerProgressChanged, this, &MultipleInstallPage::onProgressChanged);

    //安装过程中安装信息变化
    connect(m_debListModel, &DebListModel::appendOutputInfo, this, &MultipleInstallPage::onOutputAvailable);

    //开始安装时，显示安装进度条
    connect(m_debListModel, &DebListModel::onStartInstall, this, [ = ] {
        m_processFrame->setVisible(true);
    });

    //一个包安装结束后 listView滚动到其在listview中的位置
    connect(m_debListModel, &DebListModel::onChangeOperateIndex, this, &MultipleInstallPage::onAutoScrollInstallList);

    //wine依赖安装结果处理
    connect(m_debListModel, &DebListModel::DependResult, this, &MultipleInstallPage::DealDependResult);
}

/**
 * @brief MultipleInstallPage::onWorkerFinshed 安装结束的界面展示效果处理
 */
void MultipleInstallPage::onWorkerFinshed()
{
    m_currentFlag = 2;   //install finish

    //安装结束显示返回和确认按钮
    m_acceptButton->setVisible(true);
    m_backButton->setVisible(true);
    m_processFrame->setVisible(false);                  //隐藏进度条
    //当前安装结束后，不允许调出右键菜单
    m_appsListView->setRightMenuShowStatus(false);      //安装结束不允许删除包
}

/**
 * @brief MultipleInstallPage::onOutputAvailable 向安装详细信息展示窗口添加安装进度信息
 * @param output 要添加的信息
 */
void MultipleInstallPage::onOutputAvailable(const QString &output)
{
    m_installProcessInfoView->appendText(output.trimmed());     //添加信息到窗口

    // change to install
    if (!m_installButton->isVisible()) {
        m_infoControlButton->setVisible(true);                  //如果有信息添加说明此时正在安装，进度条要显示出来
    }
}

/**
 * @brief MultipleInstallPage::onProgressChanged 安装进度变化显示处理函数
 * @param progress 安装的进度
 */
void MultipleInstallPage::onProgressChanged(const int progress)
{
    m_progressAnimation->setStartValue(m_installProgress->value()); //设置动画开始的进度
    m_progressAnimation->setEndValue(progress);                     //设置进度条动画结束的进度
    m_progressAnimation->start();                                   //开始动画

    // finished
    if (progress == 100) {
        onOutputAvailable(QString());
        QTimer::singleShot(m_progressAnimation->duration(), this, &MultipleInstallPage::onWorkerFinshed);       //当前安装完成
    }
}

/**
 * @brief MultipleInstallPage::onAutoScrollInstallList 安装某一个包结束后，跳动到安装完成的包的位置
 * @param opIndex 安装完成的包的位置下标
 */
void MultipleInstallPage::onAutoScrollInstallList(int opIndex)
{
    //当前安装包的下标是合法的
    if (opIndex > 1 && opIndex < m_debListModel->getInstallFileSize()) {
        QModelIndex currIndex = m_debListModel->index(opIndex - 1);
        m_appsListView->scrollTo(currIndex, QAbstractItemView::PositionAtTop);  //跳动到下标的位置
    } else if (opIndex == -1) { //to top            //下标不合法
        QModelIndex currIndex = m_debListModel->index(0);
        m_appsListView->scrollTo(currIndex);        //跳动到顶部
    }
}

/**
 * @brief MultipleInstallPage::onRequestRemoveItemClicked 接受到applistView删除包后告知PackageManager删除包
 * @param index
 */
void MultipleInstallPage::onRequestRemoveItemClicked(const QModelIndex &index)
{
    if (!m_debListModel->isWorkerPrepare()) return;     //当前未处于准备阶段，不允许删除

    const int r = index.row();                          //要删除的包的下标转换

    emit requestRemovePackage(r);                       //发送删除信号
}

/**
 * @brief MultipleInstallPage::showInfo 显示安装信息的界面展示
 */
void MultipleInstallPage::showInfo()
{
    m_appsListView->setFocusPolicy(Qt::NoFocus);            //详细信息出现后 设置appListView不接受焦点
    m_upDown = false;                                       //未使用
    m_contentLayout->setContentsMargins(20, 0, 20, 0);      //设置上下左右边距
    m_appsListViewBgFrame->setVisible(false);               //隐藏applistView
    m_appsListView->setVisible(false);
    m_installProcessInfoView->setVisible(true);             //显示相关安装进度信息
}

/**
 * @brief MultipleInstallPage::hideInfo  隐藏详细信息
 */
void MultipleInstallPage::hideInfo()
{
    m_appsListView->setFocusPolicy(Qt::TabFocus);           //隐藏详细信息后，设置appListView的焦点策略
    m_upDown = true;
    m_contentLayout->setContentsMargins(10, 0, 10, 0);      //设置边距
    m_appsListViewBgFrame->setVisible(true);                //显示appListView
    m_appsListView->setVisible(true);
    m_installProcessInfoView->setVisible(false);            //隐藏安装过程信息
}

/**
 * @brief MultipleInstallPage::hiddenCancelButton 授权成功后隐藏按钮
 */
void MultipleInstallPage::hiddenCancelButton()
{
    //安装开始后不允许调出右键菜单。
    //安装按钮点击后清除其焦点。解决授权框消失后标题栏菜单键被focus的问题
    m_installButton->clearFocus();
    m_appsListView->setRightMenuShowStatus(false);      //安装开始时，不允许调用右键菜单
    m_backButton->setVisible(false);                    //隐藏返回按钮
    m_installButton->setVisible(false);                 //隐藏安装按钮
}

/**
 * @brief MultipleInstallPage::setEnableButton 授权框取消/显示后，设置安装按钮可用/禁用
 * @param bEnable   按钮可用/禁用的标识
 */
void MultipleInstallPage::setEnableButton(bool bEnable)
{
    m_installButton->setEnabled(bEnable);//设置按钮是否可用
    if (bEnable) {                     //按钮可用时刷新一次model 保证所有的包都能显示出来
        m_appsListView->reset();
    }
}

/**
 * @brief MultipleInstallPage::afterGetAutherFalse 授权失败或取消后的界面处理函数
 */
void MultipleInstallPage::afterGetAutherFalse()
{
    m_processFrame->setVisible(false);          //隐藏进度条
    m_infoControlButton->setVisible(false);     //隐藏infoControlButton
    m_installButton->setVisible(true);          //显示安装按钮
    m_infoControlButton->shrink();              //收缩安装信息，下次出现时是收缩的

    //授权取消或授权失败后，允许右键菜单弹出
    m_appsListView->setRightMenuShowStatus(true);
}

/**
 * @brief MultipleInstallPage::refreshModel 刷新model
 */
void MultipleInstallPage::refreshModel()
{
    m_appsListView->reset();
}

/**
 * @brief MultipleInstallPage::DealDependResult 批量处理授权结果
 * @param iAuthRes      授权的类型
 * @param dependName    需要安装的依赖的名字
 */
void MultipleInstallPage::DealDependResult(int iAuthRes, QString dependName)
{
    qDebug() << "批量处理鉴权结果：" << iAuthRes;
    switch (iAuthRes) {
    case DebListModel::AuthBefore:              //授权之前
        m_appsListView->setEnabled(false);      //listView不可用
        m_installButton->setVisible(true);      //显示安装按钮但是不可用
        m_installButton->setEnabled(false);
        m_dSpinner->stop();                     //安装动画不可见
        m_dSpinner->hide();
        m_tipsLabel->setVisible(false);         //隐藏提示
        break;
    case DebListModel::CancelAuth:              //授权被取消
        m_appsListView->setEnabled(true);       //appListView可用
        m_installButton->setVisible(true);      //安装按钮可见并可用
        m_installButton->setEnabled(true);
        break;
    case DebListModel::AuthConfirm:             //确认授权
        m_appsListView->setEnabled(false);      //listView不可用
        m_tipsLabel->setText(tr("Installing dependencies: %1").arg(dependName));    //设置提示语
        m_tipsLabel->setVisible(true);          //提示可见
        m_dSpinner->show();                     //显示动画并开始动画
        m_dSpinner->start();
        m_installButton->setVisible(false);     //隐藏安装按钮
        break;
    case DebListModel::AuthDependsSuccess:      //依赖安装成功
    case DebListModel::AuthDependsErr:          //依赖安装错误
    case DebListModel::AnalysisErr:             //解析错误
        m_appsListView->setEnabled(true);       //listView可以操作
        m_installButton->setVisible(true);      //显示安装按钮
        m_installButton->setEnabled(true);      //安装按钮可用
        m_dSpinner->stop();                     //隐藏并停止安装动画
        m_dSpinner->hide();
        m_tipsLabel->setVisible(false);         //隐藏依赖安装提示
        break;
    default:
        break;
    }
}
