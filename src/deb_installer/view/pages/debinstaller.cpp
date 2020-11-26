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

#include "debinstaller.h"
#include "model/deblistmodel.h"
#include "view/widgets/filechoosewidget.h"
#include "view/pages/multipleinstallpage.h"
#include "view/pages/singleinstallpage.h"
#include "view/pages/uninstallconfirmpage.h"
#include "view/pages/AptConfigMessage.h"
#include "view/widgets/TitleBarFocusMonitor.h"
#include "utils/utils.h"
#include "utils/DebugTimeManager.h"

#include <DInputDialog>
#include <DRecentManager>
#include <DMessageManager>
#include <DTitlebar>
#include <DGuiApplicationHelper>

#include <QAction>
#include <QDebug>
#include <QDir>
#include <QDragEnterEvent>
#include <QFileInfo>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMimeData>
#include <QStatusBar>
#include <QLayout>
#include <QProcess>
#include <QScreen>
#include <QStyleFactory>
#include <QApplication>
#include <QDesktopWidget>

using QApt::DebFile;

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

DebInstaller::DebInstaller(QWidget *parent)
    : DMainWindow(parent)
    , m_fileListModel(new DebListModel(this))
    , m_fileChooseWidget(new FileChooseWidget)
    , m_centralLayout(new QStackedLayout())
    , m_dragflag(-1)
{
    initUI();
    initConnections();
}

DebInstaller::~DebInstaller() {}

/**
 * @brief initUI
 * 初始化界面
 */
void DebInstaller::initUI()
{
    //Hide the shadow under the title bar
    setTitlebarShadowEnabled(false);

    this->setObjectName("DebInstaller");
    this->setAccessibleName("DebInstaller");

    //file choose widget settings
    m_fileChooseWidget->setObjectName("FileChooseWidget");
    m_fileChooseWidget->setAccessibleName("FileChooseWidget");

    //初始化 加载文件选择widget
    m_centralLayout->addWidget(m_fileChooseWidget);
    m_centralLayout->setContentsMargins(0, 0, 0, 0);
    m_centralLayout->setSpacing(0);

    //设置当前主窗口
    QWidget *wrapWidget = new QWidget(this);
    wrapWidget->setLayout(m_centralLayout);

//#define SHOWBORDER
#ifdef SHOWBORDER
    wrapWidget->setStyleSheet("QWidget{border:1px solid black;}");
#endif

    //title bar settings
    DTitlebar *tb = titlebar();
    tb->setIcon(QIcon::fromTheme("deepin-deb-installer"));
    tb->setTitle("");
    tb->setAutoFillBackground(true);
    tb->setDisableFlags(Qt::CustomizeWindowHint);
    //fix bug 4329, reset focusPolicy
    handleFocusPolicy();                                                        //获取标题栏的控件按钮

    //标题栏焦点监测线程。
    m_pMonitorFocusThread = new TitleBarFocusMonitor(m_OptionWindow);           //初始化标题栏焦点检测线程

    QString fontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansMedium);
    Utils::bindFontBySizeAndWeight(tb, fontFamily, 14, QFont::Medium);

    setCentralWidget(wrapWidget);  //将给定的小部件设置为主窗口的中心小部件。
    setAcceptDrops(true);          //启用了drop事件
    setFixedSize(480, 380);
    setWindowTitle(tr("Package Installer"));
    setWindowIcon(QIcon::fromTheme("deepin-deb-installer"));  //仅仅适用于windows系统
    move(qApp->primaryScreen()->geometry().center() - geometry().center());
}

/**
 * @brief handleFocusPolicy
 * 获取titleBar的控件 optionButton minButton closeButton
 */
void DebInstaller::handleFocusPolicy()
{
    //Cancel all window focus
    QLayout *layout = titlebar()->layout();
    for (int i = 0; i < layout->count(); ++i) {
        QWidget *widget = layout->itemAt(i)->widget();
        if (widget != nullptr && QString(widget->metaObject()->className()) ==  QString("QWidget")) {
            QLayout *widgetLayout = widget->layout();
            for (int j = 0; j < widgetLayout->count(); ++j) {
                QWidget *topWidget = widgetLayout->itemAt(j)->widget();
                if (topWidget != nullptr && QString(topWidget->metaObject()->className()) ==  QString("QWidget")) {
                    QLayout *wLayout = topWidget->layout();
                    for (int k = 0; k < wLayout->count(); ++k) {
                        QWidget *bottomWidget = wLayout->itemAt(k)->widget();
                        if (bottomWidget != nullptr && QString(bottomWidget->metaObject()->className()).contains("Button")) {
                            //widget->setFocusPolicy(Qt::NoFocus);
                            // 获取菜单栏按钮
                            if ("Dtk::Widget::DWindowOptionButton" == QString(bottomWidget->metaObject()->className())) {
                                m_OptionWindow = bottomWidget;
                            }
                            // 获取最小化窗口按钮
                            if ("Dtk::Widget::DWindowMinButton" == QString(bottomWidget->metaObject()->className())) {
                                m_MinWindow = bottomWidget;
                            }
                            // 获取关闭窗口按钮
                            if ("Dtk::Widget::DWindowCloseButton" == QString(bottomWidget->metaObject()->className())) {
                                m_closeWindow = bottomWidget;
                            }
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief initConnections
 * 初始化链接信号和槽
 */
void DebInstaller::initConnections()
{
    //接收到添加无效包的信号则弹出无效包的弹窗
    connect(m_fileListModel, &DebListModel::invalidPackage, this, &DebInstaller::showInvalidePackageMessage);

    //接收到包已经添加的信号则弹出已添加的弹窗
    connect(m_fileListModel, &DebListModel::packageAlreadyExists, this, &DebInstaller::showPkgExistMessage);

    //刷新单包安装界面的信号
    connect(m_fileListModel, &DebListModel::refreshSinglePage, this, &DebInstaller::refreshSingle);

    //刷新批量安装mode的信号
    connect(m_fileListModel, &DebListModel::refreshMultiPage, this, &DebInstaller::refreshMulti);

    //刷新批量安装界面的信号
    connect(m_fileListModel, &DebListModel::single2MultiPage, this, &DebInstaller::single2Multi);

    //正在添加的信号
    connect(m_fileListModel, &DebListModel::appendStart, this, &DebInstaller::appendPackageStart);

    //此次添加完成的信号
    connect(m_fileListModel, &DebListModel::appendFinished, this, &DebInstaller::appendFinished);

    //Append packages via file-choose-widget's file-choose-button
    connect(m_fileChooseWidget, &FileChooseWidget::packagesSelected, this, &DebInstaller::onPackagesSelected);

    //Select the focus of the page

    //Determine the status of the current application based on the status of the authorization box.
    connect(m_fileListModel, &DebListModel::lockForAuth, this, &DebInstaller::onAuthing);

    //show dpkg details
    connect(m_fileListModel, &DebListModel::appendOutputInfo, this, [ = ](const QString & output) {
        qDebug() << "Process:" << output.trimmed();
    });

    //During installing/uninstalling, drag is not allowed
    connect(m_fileListModel, &DebListModel::workerFinished, this, &DebInstaller::changeDragFlag);

    //When the authorization is revoked, show install/uninstall/reinstall button which hidden during authorizing
    connect(m_fileListModel, &DebListModel::AuthCancel, this, &DebInstaller::showHiddenButton);

    //When starting the installation, the install button is not available
    connect(m_fileListModel, &DebListModel::onStartInstall, this, &DebInstaller::onStartInstallRequested);

    //When the authorization box pops up, the install button is not available.
    connect(m_fileListModel, &DebListModel::EnableReCancelBtn, this, &DebInstaller::setEnableButton);

    //When installing deepin-wine for the first time, set the button display according to the progress of the installation
    connect(m_fileListModel, &DebListModel::DependResult, this, &DebInstaller::DealDependResult);

    connect(m_fileListModel, &DebListModel::enableCloseButton, this, &DebInstaller::enableCloseButton);

    //Append packages via double-clicked or right-click
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::newProcessInstance, this, &DebInstaller::onNewAppOpen);

    //监测到安装进度，停止监测标题栏菜单键焦点。
    connect(m_fileListModel, &DebListModel::appendOutputInfo, this, &DebInstaller::stopMonitorTitleBarFocus);
}

/**
 * @brief DebInstaller::stopMonitorTitleBarFocus 停止监测标题栏焦点。
 */
void DebInstaller::stopMonitorTitleBarFocus()
{
    if (m_pMonitorFocusThread->isRunning()) {
        m_pMonitorFocusThread->stopMonitor();
    }
}


/**
 * @brief DebInstaller::enableCloseButton 当安装、卸载开始或结束后，根据传递的值启用或禁用关闭按钮
 * @param enable 启用还是禁用关闭按钮的标识
 */
void DebInstaller::enableCloseButton(bool enable)
{
    if (enable) {
        enableCloseAndExit();                           //启用关闭按钮
    } else {
        disableCloseAndExit();                          //禁用关闭按钮
    }
}

/**
 * @brief disableCloseAndExit
 * 设置退出和关闭按钮为不可用
 */
// closed is forbidden during install/uninstall
void DebInstaller::disableCloseAndExit()
{
    titlebar()->setDisableFlags(Qt::WindowCloseButtonHint);             //设置标题栏中的关闭按钮不可用
    QMenu *titleMenu = titlebar()->menu();
    QList<QAction *> actions = titleMenu->actions();
    QAction *action = actions.last();                                   //获取标题栏菜单的关闭按钮
    action->setDisabled(true);                                          //设置标题栏菜单中的关闭按钮不可用

    // fix bug: 36125 During the installation process, clicking the window close button has a hover effect
    titlebar()->setFocusPolicy(Qt::NoFocus);
    this->setFocusPolicy(Qt::NoFocus);
}

/**
 * @brief enableCloseAndExit
 * 设置退出和关闭按钮可用
 */
// closed is allowed after install/uninstall
void DebInstaller::enableCloseAndExit()
{
    titlebar()->setDisableFlags(titlebar()->windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowCloseButtonHint);    //设置标题栏的关闭按钮可用

    QMenu *titleMenu = titlebar()->menu();
    QList<QAction *> actions = titleMenu->actions();
    QAction *action = actions.last();                                       //获取标题栏菜单中的关闭按钮
    action->setDisabled(false);                                             //设置标题栏菜单中的关闭按钮不可用

    // fix bug: 36125 During the installation process, clicking the window close button has a hover effect
    titlebar()->setFocusPolicy(Qt::NoFocus);
    this->setFocusPolicy(Qt::NoFocus);
}

/**
 * @brief onStartInstallRequested
 * 安装开始后，所有的关闭按钮都会被禁止
 * SP3新增，解决安装开始时焦点闪现的问题。
 */
//after start installing,all close button is forbidden.
void DebInstaller::onStartInstallRequested()
{
    //开始监测标题栏菜单键的焦点
    m_pMonitorFocusThread->start();
    disableCloseAndExit();                                          //安装开始后，关闭按钮不可用，并开始检测标题栏菜单键的焦点
}

/**
 * @brief onNewAppOpen
 * @param pid 进程号
 * @param arguments 要安装的包的全路径的列表
 * 桌面或文管中双击或右键打开时的槽函数
 * 会把后缀为.deb的包传递到onPackageSelected中
 * packages selected via double-click or right-click
 */
void DebInstaller::onNewAppOpen(qint64 pid, const QStringList &arguments)
{
    Q_UNUSED(pid)
    qDebug() << "onNewAppOpen: pid:" << pid << ", arguments:" << arguments;

    QStringList debFileList;                                    //保存deb包的路径
    for (int i = 0; i < arguments.size(); i++) {
        QString strArg = arguments.at(i);                       //某个deb包
        if (!strArg.contains("deb-installer")) {
            if (checkSuffix(strArg)) {                          //检查拖入包的后缀
                debFileList << strArg;                          //添加到临时列表中
            }
        }
    }

    if (debFileList.size() > 0) {
        onPackagesSelected(debFileList);                        //添加到安装器中
    }

    this->setWindowState(Qt::WindowActive);                     //激活窗口
    this->activateWindow();
}

/**
 * @brief DebInstaller::keyPressEvent 按键事件
 * @param e
 */
void DebInstaller::keyPressEvent(QKeyEvent *e)
{
    // debug 模式下，按下Esc 会退出安装程序
    switch (e->key()) {
#ifdef QT_DEBUG
    case Qt::Key_Escape:
        qApp->quit();
        break;
#endif
    default:
        ;
    }
}

//during install, drag package into application is not allowed.
//If the dragged file which suffix isn't deb, not allowed to append
void DebInstaller::dragEnterEvent(QDragEnterEvent *e)
{
    this->activateWindow();                                                         //拖入时，激活窗口
    if (m_fileListModel->m_workerStatus_temp == DebListModel::WorkerProcessing) {   //如果当前正在安装，不允许拖入包
        this->setAcceptDrops(false);                                                //不允许拖入
    } else {
        m_fileChooseWidget->setAcceptDrops(true);                                   //允许包被拖入
        if (m_dragflag == 0)                                                        //如果当前不允许拖入，则直接返回
            return;

        auto *const mime = e->mimeData();
        if (!mime->hasUrls()) return e->ignore();                                   //如果当前的数据不是一个路径，直接忽略

        for (const auto &item : mime->urls()) {                                     // 循环 获取拖入的路径数据
            const QFileInfo info(item.path());
            if (info.isDir()) return e->accept();
            //fix bug: https://pms.uniontech.com/zentao/bug-view-48801.html
            if (checkSuffix(item.path())) return e->accept();//检查拖入包的后缀
        }
        e->ignore();
    }
}

//Accept the file which suffix is deb and append to application
void DebInstaller::dropEvent(QDropEvent *e)
{
    auto *const mime = e->mimeData();
    if (!mime->hasUrls()) return e->ignore();                   //如果数据不是一个路径，忽略

    e->accept();

    // find .deb files
    QStringList file_list;                                      //存放文件列表
    for (const auto &url : mime->urls()) {
        if (!url.isLocalFile()) continue;                       //如果不是本地的文件 忽略
        const QString local_path = url.toLocalFile();
        const QFileInfo info(local_path);

        //fix bug: https://pms.uniontech.com/zentao/bug-view-48801.html
        if (checkSuffix(local_path))//检查拖入包的后缀
            file_list << local_path;
        else if (info.isDir()) {
            for (auto deb : QDir(local_path).entryInfoList(QStringList() << "*.deb", QDir::Files))
                file_list << deb.absoluteFilePath();            //获取文件的绝对路径
        }
    }
    this->activateWindow();                                     //激活窗口
    onPackagesSelected(file_list);                              //放到安装列表中
}

/**
 * @brief DebInstaller::dragMoveEvent 拖入移动事件
 * @param e
 */
void DebInstaller::dragMoveEvent(QDragMoveEvent *e)
{
    e->accept();
}

/**
 * @brief onPackagesSelected
 * @param packages 安装的包的全路径的列表
 * 添加包时，对包进行处理，去除无效的包，提示已经添加过的包，并根据添加包的数量刷新界面
 * Add packages that are not in the application to the application in sequence
 * After appending ,refresh page according to packages' count
 */

void DebInstaller::onPackagesSelected(const QStringList &packages)
{
    //根据不同的包的数量开启不同的记录点
    if (packages.size() > 1) {             //单包安装记录当前包的大小
        PERF_PRINT_BEGIN("POINT-06", QString::number(packages.size()));
    }
    this->showNormal();                                                 //非特效模式下激活窗口
    this->activateWindow();                                             //特效模式下激活窗口
    qDebug() << "DebInstaller:" << packages.size() << "packages have been selected";

    // 如果此时 软件包安装器不是处于准备状态且还未初始化完成，则不添加
    // 如果此时处于正在安装或者卸载状态，则不添加
    if ((!m_lastPage.isNull() && m_fileListModel->m_workerStatus_temp != DebListModel::WorkerPrepare) ||
            m_fileListModel->m_workerStatus_temp == DebListModel::WorkerProcessing ||
            m_fileListModel->m_workerStatus_temp == DebListModel::WorkerUnInstall) {
        qDebug() << "DebInstaller:" << "The program state is wrong and the package is not allowed to be added to the application";
        if (packages.size() > 1) {
            PERF_PRINT_END("POINT-06");     //添加结束记录点
        }
    } else {
        qDebug() << "DebInstaller:" << "Ready to add the package to the package installer";
        //开始添加包，将要添加的包传递到后端，添加包由后端处理
        m_fileListModel->appendPackage(packages);
    }
}

/**
 * @brief DebInstaller::refreshMulti 刷新批量安装model
 * 在批量安装直接添加包
 */
void DebInstaller::refreshMulti()
{
    qInfo() << "[DebInstaller]" << "[refreshMulti]" << "add a package to multiple page";
    m_dragflag = 1;                                                                 //之前有多个包，之后又添加了包，则直接刷新listview
    MulRefreshPage();
}

/**
 * @brief DebInstaller::showInvalidePackageMessage 弹出无效包的消息通知
 */
void DebInstaller::showInvalidePackageMessage()
{
    //add Floating Message while package invalid
    DFloatingMessage *msg = new DFloatingMessage;
    msg->setMessage(tr("The deb package may be broken"));
    msg->setIcon(QIcon::fromTheme("di_warning"));
    DMessageManager::instance()->sendMessage(this, msg);                        //如果损坏，提示
}

/**
 * @brief DebInstaller::showPkgExistMessage 弹出添加相同包的消息通知
 */
void DebInstaller::showPkgExistMessage()
{
    qWarning() << "DebInstaller:" << "package is Exist! ";
    DFloatingMessage *msg = new DFloatingMessage;
    msg->setMessage(tr("Already Added"));
    msg->setIcon(QIcon::fromTheme("di_ok"));
    DMessageManager::instance()->sendMessage(this, msg);                        //已经添加的包会提示
}


/**
 * @brief showUninstallConfirmPage
 * 卸载按钮的槽函数
 * 显示卸载界面
 */
void DebInstaller::showUninstallConfirmPage()
{
    Q_ASSERT(m_centralLayout->count() == 2);
    m_fileListModel->m_workerStatus_temp = DebListModel::WorkerUnInstall;                       //刷新当前安装器的工作状态

    this->setAcceptDrops(false);                                                                //卸载页面不允许添加/拖入包

    const QModelIndex index = m_fileListModel->first();                                         //只有单包才有卸载界面

    //  Set the display information of the uninstall page
    UninstallConfirmPage *p = new UninstallConfirmPage(this);                                   //初始化卸载页面
    p->setRequiredList(index.data(DebListModel::PackageReverseDependsListRole).toStringList()); //查看是否有包依赖于当前要卸载的包，病获取列表
    p->setPackage(index.data().toString());                                                     //添加卸载提示语

    m_Filterflag = 3; //Uninstall the page
    m_UninstallPage = p;                                                                        //保存卸载页面

    m_centralLayout->addWidget(p);                                                              //添加卸载页面到主界面中
    m_centralLayout->setCurrentIndex(2);                                                        //显示卸载页面
    p->setAcceptDrops(false);                                                                   //卸载页面不允许拖入包

    connect(p, &UninstallConfirmPage::accepted, this, &DebInstaller::onUninstallAccepted);      //卸载页面确认卸载
    connect(p, &UninstallConfirmPage::canceled, this, &DebInstaller::onUninstallCancel);        //卸载页面取消卸载
}

/**
 * @brief onUninstallAccepted
 * 卸载界面确认卸载按钮的槽函数
 * 卸载开始时，返回singleInstallPage 并显示卸载进程。
 */
void DebInstaller::onUninstallAccepted()
{
    // uninstall begin
    m_pMonitorFocusThread->start();                                                             // 确认卸载后开启标题栏焦点监控线程。
    SingleInstallPage *p = backToSinglePage();                                                  // 获取单包安装界面(卸载页面其实也是单包安装页面的一种)
    m_fileChooseWidget->setAcceptDrops(true);                                                   // 设置文件选择界面可以拖入包
    p->uninstallCurrentPackage();                                                               // 显示正在卸载页面

    //set close button disabled while uninstalling
    disableCloseAndExit();                                                                      // 卸载时不允许关闭或退出

    m_Filterflag = m_dragflag;
}

/**
 * @brief onUninstallCancel
 * 卸载界面取消卸载按钮的槽函数
 * 取消卸载后返回 singleInstallPage
 */
void DebInstaller::onUninstallCancel()
{
    // Cancel uninstall
    this->setAcceptDrops(true);                                                                 //取消卸载，允许包被拖入
    m_fileListModel->m_workerStatus_temp = DebListModel::WorkerPrepare;                         //重置工作状态为准备状态
    backToSinglePage();                                                                         //返回单包安装页面

    m_Filterflag = m_dragflag;
}

/**
 * @brief onAuthing
 * @param authing 按钮是否可用的标志
 * 授权框弹出后，设置当前界面为不可用状态
 * PS: 授权超时后，界面会可以被点击，此时按钮需要被禁用
 */
void DebInstaller::onAuthing(const bool authing)
{
    //The authorization box pops up, the setting button is not available
    setEnabled(!authing);                                                                       //授权框弹出时，按钮不可用  授权框被关闭后，按钮可用
}

/**
 * @brief reset
 * 重置当前工作状态、拖入状态、标题栏、页面暂存区，删除卸载页面
 *
 */
void DebInstaller::reset()
{
    //reset page status
    Q_ASSERT(m_centralLayout->count() == 2);
    Q_ASSERT(!m_lastPage.isNull());
    m_fileListModel->m_workerStatus_temp = 0;                               // 当前工作状态
    m_dragflag = -1;                                                        // 是否被允许拖入或添加
    m_Filterflag = -1;                                                      // 当前显示的页面
    titlebar()->setTitle(QString());                                        // 重置标题栏
    m_fileListModel->reset();                                               // 重置model

    // 删除所有的页面
    m_lastPage->deleteLater();
    m_UninstallPage->deleteLater();
    m_centralLayout->setCurrentIndex(0);

    //fix bug: 44801  https://pms.uniontech.com/zentao/bug-view-44801.html
    this->setAcceptDrops(true);
    m_fileChooseWidget->setAcceptDrops(true);
    // 安装完成后，清除文件选择按钮的焦点
    // fix bug:https://pms.uniontech.com/zentao/bug-view-46525.html
    m_fileChooseWidget->clearChooseFileBtnFocus();
}

/**
 * @brief removePackage
 * @param index 要删除的包的下标
 * 根据传入的下表删除某个包。
 */
void DebInstaller::removePackage(const int index)
{
    m_fileListModel->removePackage(index);                                          // 后端删除某个下表的包
    const int packageCount = m_fileListModel->preparedPackages().size();            // 获取删除后的包的数量
    if (packageCount == 1) {                                                        // 删除后包的数量只有一个，从批量安装页面刷新成单包安装页面
        refreshSingle();
    }
    if (packageCount > 1)                                                           // 删除后仍然有多个包，直接刷新批量安装界面
        MulRefreshPage();
}

/**
 * @brief DebInstaller::appendPackageStart 正在添加包
 */
void DebInstaller::appendPackageStart()
{
    packageAppending = true;
    MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
    if (multiplePage)
        multiplePage->setEnableButton(false);
}

/**
 * @brief DebInstaller::appendFinished 添加包结束
 */
void DebInstaller::appendFinished()
{
    qDebug() << "[DebInstaller]" << "[appendFinished]" << "append package finished";
    MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
    if (multiplePage) {
        multiplePage->setEnableButton(true);
    }
    packageAppending = false;
    PERF_PRINT_END("POINT-06");     //添加结束记录点
}

/**
 * @brief MulRefreshPage
 * @param index 某一个包的下标位置
 *
 * 刷新multiInstallPage并在刷新后滚动到下标处
 */
void DebInstaller::MulRefreshPage()
{
    if (m_dragflag == 1) {
        MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);        //获取批量安装类的指针
        multiplePage->refreshModel();
    }
}

/**
 * @brief refreshInstallPage
 * @param index 某一个包的下标
 * 刷新安装界面 多用于singleInstallPage 转换到 multiSinglePage
 */

void DebInstaller::single2Multi()
{
    // 刷新文件的状态，初始化包的状态为准备状态
    qDebug() << "refresh single 2 multiPage";
    m_fileListModel->reset_filestatus();
    m_fileListModel->initPrepareStatus();
    if (!m_lastPage.isNull()) m_lastPage->deleteLater();                    //清除widgets缓存

    // multiple packages install

    titlebar()->setTitle(tr("Bulk Install"));

    MultipleInstallPage *multiplePage = new MultipleInstallPage(m_fileListModel);
    multiplePage->setObjectName("MultipleInstallPage");

    connect(multiplePage, &MultipleInstallPage::back, this, &DebInstaller::reset);
    connect(multiplePage, &MultipleInstallPage::requestRemovePackage, this, &DebInstaller::removePackage);
    multiplePage->refreshModel();
    m_lastPage = multiplePage;
    m_centralLayout->addWidget(multiplePage);
    m_dragflag = 1;
    m_Filterflag = 1;

    // switch to new page.
    m_centralLayout->setCurrentIndex(1);

    // 刷新之后清除标题栏菜单键的焦点，防止在多次安装成功后再次添加包时，焦点偶尔出现在标题栏菜单键上。
    m_OptionWindow->clearFocus();

}

/**
 * @brief DebInstaller::refreshSingle   刷新单包安装界面
 */
void DebInstaller::refreshSingle()
{
    //刷新页面之前先清除掉文件选择按钮的焦点，防止在文件选择按钮有焦点的时候添加包，焦点转移到其他控件
    m_fileChooseWidget->clearChooseFileBtnFocus();

    // 刷新文件的状态，初始化包的状态为准备状态
    m_fileListModel->reset_filestatus();
    m_fileListModel->initPrepareStatus();
    // clear widgets if needed
    if (!m_lastPage.isNull()) m_lastPage->deleteLater();                    //清除widgets缓存
    //安装器中只有一个包，刷新单包安装页面
    // single package install
    titlebar()->setTitle(QString());

    SingleInstallPage *singlePage = new SingleInstallPage(m_fileListModel);
    singlePage->setObjectName("SingleInstallPage");
    connect(singlePage, &SingleInstallPage::back, this, &DebInstaller::reset);
    connect(singlePage, &SingleInstallPage::requestUninstallConfirm, this, &DebInstaller::showUninstallConfirmPage);

    m_lastPage = singlePage;
    m_centralLayout->addWidget(singlePage);

    // 重置安装器拖入的状态与工作的状态
    m_dragflag = 2;
    m_Filterflag = 2;
    // switch to new page.
    m_centralLayout->setCurrentIndex(1);

    // 刷新之后清除标题栏菜单键的焦点，防止在多次安装成功后再次添加包时，焦点偶尔出现在标题栏菜单键上。
    m_OptionWindow->clearFocus();
}

/**
 * @brief backToSinglePage 返回单包安装界面
 * @return SingleInstallPage* SingleInstallPage的指针
 */
SingleInstallPage *DebInstaller::backToSinglePage()
{
    Q_ASSERT(m_centralLayout->count() == 3);

    // 获取当前的页面并删除
    QWidget *confirmPage = m_centralLayout->widget(2);
    m_centralLayout->removeWidget(confirmPage);
    confirmPage->deleteLater();

    SingleInstallPage *p = qobject_cast<SingleInstallPage *>(m_centralLayout->widget(1));           //获取单包安装widget

    // 返回单包安装页面时，允许添加包
    p->setAcceptDrops(true);
    m_fileChooseWidget->setAcceptDrops(true);
    this->setAcceptDrops(true);
    Q_ASSERT(p);

    return p;
}

/**
 * @brief changeDragFlag
 * 安装卸载结束后，允许包被拖入程序，并设置关闭按钮可用
 */
void DebInstaller::changeDragFlag()
{
    repaint();
    m_dragflag = 0;                                 //允许包被拖入且此时程序中没有包

    enableCloseAndExit();
}

/**
 * @brief setEnableButton
 * @param bEnable 按钮是否可用标志
 * 根据当前的安装/卸载进程来控制singleInstallPage/multiInstallPage按钮的可用性
 * 授权超时，禁用按钮
 * 授权框取消，启用按钮
 */
void DebInstaller::setEnableButton(bool bEnable)
{
    //如果正在添加包，则启用按钮
    if (packageAppending)
        return;
    //Set button enabled after installation canceled
    if (m_dragflag == 2) {//单包安装按钮的启用与禁用
        SingleInstallPage *singlePage = qobject_cast<SingleInstallPage *>(m_lastPage);
        singlePage->setEnableButton(bEnable);
    } else if (m_dragflag == 1) {//批量安装按钮的启用与禁用
        MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
        multiplePage->setEnableButton(bEnable);
    }
    //授权取消后 停止监测标题栏菜单键的焦点。
    m_pMonitorFocusThread->stopMonitor();
}

/**
 * @brief showHiddenButton
 * 授权取消后显示被隐藏的按钮
 */
void DebInstaller::showHiddenButton()
{
    m_fileListModel->reset_filestatus();        //授权取消，重置所有的状态，包括安装状态，依赖状态等
    if (m_dragflag == 2) {// 单包安装显示按钮
        SingleInstallPage *singlePage = qobject_cast<SingleInstallPage *>(m_lastPage);
        singlePage->afterGetAutherFalse();
    } else if (m_dragflag == 1) {//批量安装显示按钮
        MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
        multiplePage->afterGetAutherFalse();
    }
}

/**
 * @brief DebInstaller::closeEvent 关闭事件
 * @param event
 */
void DebInstaller::closeEvent(QCloseEvent *event)
{
    DMainWindow::closeEvent(event);
}

/**
 * @brief DealDependResult
 * @param iAuthRes
 * 根据deepin-wine依赖安装的结果处理界面显示效果
 */
void DebInstaller::DealDependResult(int iAuthRes, QString dependName)
{
    //Set the display effect according to the status of deepin-wine installation authorization.
    //Before authorization, authorization confirmation, and when the authorization box pops up, it is not allowed to add packages.
    //依赖下载时、授权时不允许拖入
    if (iAuthRes == DebListModel::AuthBefore || iAuthRes == DebListModel::AuthConfirm || iAuthRes == DebListModel::AuthPop) {
        this->setAcceptDrops(false);
    } else {
        this->setAcceptDrops(true);
    }

    if (iAuthRes == DebListModel::AuthDependsSuccess) { //依赖下载成功
        m_fileListModel->reset_filestatus();//清除包的状态和包的错误原因
        m_fileListModel->initPrepareStatus();//重置包的prepare状态。
    }
    //Refresh the display effect of different pages
    if (m_dragflag == 2) {      //单包安装处理依赖下载授权情况
        SingleInstallPage *singlePage = qobject_cast<SingleInstallPage *>(m_lastPage);
        singlePage->DealDependResult(iAuthRes, dependName);
    } else if (m_dragflag == 1) {//批量安装处理依赖下载授权情况
        MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
        multiplePage->DealDependResult(iAuthRes, dependName);
        multiplePage->refreshModel();// 滚动到最后一行。
    }
}

/**
 * @brief DebInstaller::checkSuffix 检查文件类型是否正确
 * @param filePath 文件路径
 * @return 后缀是否正确
 */
bool DebInstaller::checkSuffix(QString filePath)
{
    const QFileInfo info(filePath);
    if (info.isFile() && info.suffix().toLower() == "deb") {        //大小写不敏感的判断是否为deb后缀
        return true;
    }
    return false;

}

/**
 * @brief DebInstaller::event
 * @param event
 * @return
 */
bool DebInstaller::event(QEvent *event)
{
    //fix bug: https://pms.uniontech.com/zentao/bug-view-55563.html
//    qInfo()<<"event:"<<event->type();
    if (event->type() == QEvent::WindowActivate) {      //重新激活时，清除全部的焦点。
        qInfo()<<"WindowActive";
        if (m_OptionWindow) {
            qInfo()<<"Option Window clear focus";
            m_OptionWindow->clearFocus();
        }
        if (this->focusWidget()) {
            qInfo()<<"focus Widget clear focus";
            qInfo()<<focusWidget()->objectName();
            this->focusWidget()->clearFocus();
        }
    }
    return QWidget::event(event);
}
