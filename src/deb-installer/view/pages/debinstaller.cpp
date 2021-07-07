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


#include "debinstaller.h"
#include "model/deblistmodel.h"
#include "view/widgets/filechoosewidget.h"
#include "view/pages/multipleinstallpage.h"
#include "view/pages/singleinstallpage.h"
#include "view/pages/uninstallconfirmpage.h"
#include "view/pages/AptConfigMessage.h"
#include "utils/utils.h"

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
    , m_fileChooseWidget(new FileChooseWidget(this))
    , m_centralLayout(new QStackedLayout())
{
    initUI();
    initConnections();
}

DebInstaller::~DebInstaller() {}


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
    if (tb) {
        tb->setIcon(QIcon::fromTheme("deepin-deb-installer"));
        tb->setTitle("");
        tb->setAutoFillBackground(true);
        tb->setDisableFlags(Qt::CustomizeWindowHint);
    }


    setCentralWidget(wrapWidget);  //将给定的小部件设置为主窗口的中心小部件。
    setAcceptDrops(true);          //启用了drop事件
    setFixedSize(480, 380);
    setWindowTitle(tr("Package Installer"));
    setWindowIcon(QIcon::fromTheme("deepin-deb-installer"));  //仅仅适用于windows系统
    move(qApp->primaryScreen()->geometry().center() - geometry().center());
}

void DebInstaller::initConnections()
{
    //接收到添加无效包的信号则弹出无效包的弹窗
    connect(m_fileListModel, &DebListModel::signalInvalidPackage, this, &DebInstaller::slotShowInvalidePackageMessage);

    //接收到添加无效包的信号则弹出无效包的弹窗
    connect(m_fileListModel, &DebListModel::signalNotLocalPackage, this, &DebInstaller::slotShowNotLocalPackageMessage);

    //接收到包已经添加的信号则弹出已添加的弹窗
    connect(m_fileListModel, &DebListModel::signalPackageAlreadyExists, this, &DebInstaller::slotShowPkgExistMessage);

    //刷新单包安装界面的信号
    connect(m_fileListModel, &DebListModel::signalRefreshSinglePage, this, &DebInstaller::refreshSingle);

    //刷新批量安装mode的信号
    connect(m_fileListModel, &DebListModel::signalRefreshMultiPage, this, &DebInstaller::refreshMulti);

    //刷新批量安装界面的信号
    connect(m_fileListModel, &DebListModel::signalSingle2MultiPage, this, &DebInstaller::single2Multi);

    connect(m_fileListModel, &DebListModel::signalRefreshFileChoosePage, this, &DebInstaller::slotReset);

    //正在添加的信号
    connect(m_fileListModel, &DebListModel::signalAppendStart, this, &DebInstaller::appendPackageStart);

    //此次添加完成的信号
    connect(m_fileListModel, &DebListModel::signalAppendFinished, this, &DebInstaller::appendFinished);

    //Append packages via file-choose-widget's file-choose-button
    connect(m_fileChooseWidget, &FileChooseWidget::packagesSelected, this, &DebInstaller::slotPackagesSelected);

    //Select the focus of the page

    //Determine the status of the current application based on the status of the authorization box.
    connect(m_fileListModel, &DebListModel::signalLockForAuth, this, &DebInstaller::slotSetAuthingStatus);

    //During installing/uninstalling, drag is not allowed
    connect(m_fileListModel, &DebListModel::signalWorkerFinished, this, &DebInstaller::slotChangeDragFlag);
    connect(m_fileListModel, &DebListModel::signalAuthCancel, this, &DebInstaller::slotShowHiddenButton);
    connect(m_fileListModel, &DebListModel::signalStartInstall, this, &DebInstaller::disableCloseAndExit);
    connect(m_fileListModel, &DebListModel::signalEnableReCancelBtn, this, &DebInstaller::slotSetEnableButton);
    connect(m_fileListModel, &DebListModel::signalDependResult, this, &DebInstaller::slotDealDependResult);
    connect(m_fileListModel, &DebListModel::signalEnableCloseButton, this, &DebInstaller::slotEnableCloseButton);
    connect(m_fileListModel, &DebListModel::signalPackageCannotFind, this, &DebInstaller::slotShowPkgRemovedMessage);
}

void DebInstaller::slotEnableCloseButton(bool enable)
{
    if (enable) {
        enableCloseAndExit();                           //启用关闭按钮
    } else {
        disableCloseAndExit();                          //禁用关闭按钮
    }
}

void DebInstaller::disableCloseAndExit()
{
    titlebar()->setDisableFlags(Qt::WindowCloseButtonHint);             //设置标题栏中的关闭按钮不可用
    DTitlebar *tbar = this->titlebar();
    if(tbar){
        tbar->setQuitMenuDisabled(true);
    }

}

void DebInstaller::enableCloseAndExit()
{
    titlebar()->setDisableFlags(titlebar()->windowFlags() &
                                ~Qt::WindowMinimizeButtonHint &
                                ~Qt::WindowCloseButtonHint);

    DTitlebar *tbar = this->titlebar();
    if(tbar){
        tbar->setQuitMenuDisabled(false);
    }
}

void DebInstaller::dragEnterEvent(QDragEnterEvent *dragEnterEvent)
{
    this->activateWindow();                                                         //拖入时，激活窗口
    if (m_fileListModel->getWorkerStatus() == DebListModel::WorkerProcessing) {   //如果当前正在安装，不允许拖入包
        this->setAcceptDrops(false);                                                //不允许拖入
    } else {
        m_fileChooseWidget->setAcceptDrops(true);                                   //允许包被拖入
        if (m_dragflag == 0)                                                        //如果当前不允许拖入，则直接返回
            return;

        auto *const mime = dragEnterEvent->mimeData();
        if (!mime->hasUrls())
            return dragEnterEvent->ignore();                                   //如果当前的数据不是一个路径，直接忽略

        for (const auto &item : mime->urls()) {                                     // 循环 获取拖入的路径数据
            const QFileInfo info(item.path());
            if (info.isDir())
                return dragEnterEvent->accept();
            if (checkSuffix(item.path()))
                return dragEnterEvent->accept();//检查拖入包的后缀
        }
        dragEnterEvent->ignore();
    }
}

void DebInstaller::dropEvent(QDropEvent *dropEvent)
{
    auto *const mime = dropEvent->mimeData();
    if (!mime->hasUrls()) return dropEvent->ignore();                   //如果数据不是一个路径，忽略

    dropEvent->accept();

    // find .deb files
    QStringList file_list;                                      //存放文件列表
    for (const auto &url : mime->urls()) {
        if (!url.isLocalFile())
            continue;                       //如果不是本地的文件 忽略
        const QString local_path = url.toLocalFile();
        const QFileInfo info(local_path);

        if (checkSuffix(local_path))//检查拖入包的后缀
            file_list << local_path;
        else if (info.isDir()) {
            for (auto deb : QDir(local_path).entryInfoList(QStringList() << "*.deb", QDir::Files))
                file_list << deb.absoluteFilePath();            //获取文件的绝对路径
        }
    }
    this->activateWindow();                                     //激活窗口
    if (!file_list.isEmpty()) {                                 //处理拖入文件夹的情况
        slotPackagesSelected(file_list);                              //放到安装列表中
    }

}

void DebInstaller::dragMoveEvent(QDragMoveEvent *dragMoveEvent)
{
    dragMoveEvent->accept();
}

void DebInstaller::slotPackagesSelected(const QStringList &packagesPathList)
{
    this->showNormal();                                                 //非特效模式下激活窗口
    this->activateWindow();                                             //特效模式下激活窗口
    // 如果此时 软件包安装器不是处于准备状态且还未初始化完成或此时正处于正在安装或者卸载状态，则不添加
    if ((!m_lastPage.isNull() && m_fileListModel->getWorkerStatus() != DebListModel::WorkerPrepare) ||
            m_fileListModel->getWorkerStatus() == DebListModel::WorkerProcessing ||
            m_fileListModel->getWorkerStatus() == DebListModel::WorkerUnInstall) {
    } else {
        //开始添加包，将要添加的包传递到后端，添加包由后端处理
        m_fileListModel->slotAppendPackage(packagesPathList);
    }
}

void DebInstaller::refreshMulti()
{
    qInfo() << "[DebInstaller]" << "[refreshMulti]" << "add a package to multiple page";
    m_dragflag = 1;                                                                 //之前有多个包，之后又添加了包，则直接刷新listview
    MulRefreshPage();
}

void DebInstaller::slotShowInvalidePackageMessage()
{
    DFloatingMessage *floatingMsg = new DFloatingMessage;
    floatingMsg->setMessage(tr("The deb package may be broken"));
    floatingMsg->setIcon(QIcon::fromTheme("di_warning"));
    DMessageManager::instance()->sendMessage(this, floatingMsg);                        //如果损坏，提示
}

void DebInstaller::slotShowNotLocalPackageMessage()
{
    DFloatingMessage *floatingMsg = new DFloatingMessage;
    floatingMsg->setMessage(tr("You can only install local deb packages"));
    floatingMsg->setIcon(QIcon::fromTheme("di_warning"));
    DMessageManager::instance()->sendMessage(this, floatingMsg);                        //如果损坏，提示
}

void DebInstaller::slotShowPkgExistMessage()
{
    qWarning() << "DebInstaller:" << "package is Exist! ";
    DFloatingMessage *floatingMsg = new DFloatingMessage;
    floatingMsg->setMessage(tr("Already Added"));
    floatingMsg->setIcon(QIcon::fromTheme("di_ok"));
    DMessageManager::instance()->sendMessage(this, floatingMsg);                        //已经添加的包会提示
}

void DebInstaller::slotShowPkgRemovedMessage(QString packageName)
{
    qWarning() << "DebInstaller:" << packageName << "File is not accessible";
    DFloatingMessage *floatingMsg = new DFloatingMessage;
    floatingMsg->setMessage(tr("%1 does not exist, please reselect").arg(packageName));
    floatingMsg->setIcon(QIcon::fromTheme("di_ok"));
    DMessageManager::instance()->sendMessage(this, floatingMsg);                        //已经添加的包会提示
}

void DebInstaller::slotShowUninstallConfirmPage()
{
    m_fileListModel->setWorkerStatus( DebListModel::WorkerUnInstall);                       //刷新当前安装器的工作状态

    this->setAcceptDrops(false);                                                                //卸载页面不允许添加/拖入包

    const QModelIndex index = m_fileListModel->first();                                         //只有单包才有卸载界面

    m_uninstallPage = new UninstallConfirmPage(this);                                   //初始化卸载页面
    m_uninstallPage->setRequiredList(index.data(DebListModel::PackageReverseDependsListRole).toStringList()); //查看是否有包依赖于当前要卸载的包，病获取列表
    m_uninstallPage->setPackage(index.data().toString());                                                     //添加卸载提示语

    m_Filterflag = 3;
    m_centralLayout->addWidget(m_uninstallPage);                                                              //添加卸载页面到主界面中
    m_centralLayout->setCurrentIndex(2);                                                        //显示卸载页面
    m_uninstallPage->setAcceptDrops(false);                                                                   //卸载页面不允许拖入包
    connect(m_uninstallPage, &UninstallConfirmPage::signalUninstallAccepted, this, &DebInstaller::slotUninstallAccepted);      //卸载页面确认卸载
    connect(m_uninstallPage, &UninstallConfirmPage::signalUninstallCanceled, this, &DebInstaller::slotUninstallCancel);        //卸载页面取消卸载
}

void DebInstaller::slotUninstallAccepted()
{
    // uninstall begin
    SingleInstallPage *singlePage = backToSinglePage();                                                  // 获取单包安装界面(卸载页面其实也是单包安装页面的一种)
    if (nullptr == singlePage)
        return;
    m_fileChooseWidget->setAcceptDrops(true);                                                   // 设置文件选择界面可以拖入包
    singlePage->slotUninstallCurrentPackage();                                                               // 显示正在卸载页面

    //set close button disabled while uninstalling
    disableCloseAndExit();                                                                      // 卸载时不允许关闭或退出

    m_Filterflag = m_dragflag;
}

void DebInstaller::slotUninstallCancel()
{
    // Cancel uninstall
    this->setAcceptDrops(true);                                                                 //取消卸载，允许包被拖入
    m_fileListModel->setWorkerStatus(DebListModel::WorkerPrepare);                        //重置工作状态为准备状态
    backToSinglePage();                                                                         //返回单包安装页面

    m_Filterflag = m_dragflag;
}

void DebInstaller::slotSetAuthingStatus(const bool authing)
{
    //The authorization box pops up, the setting button is not available
    setEnabled(!authing);                                                                       //授权框弹出时，按钮不可用  授权框被关闭后，按钮可用
}

void DebInstaller::slotReset()
{
    //reset page status
    m_dragflag = -1;                                                        // 是否被允许拖入或添加
    m_Filterflag = -1;                                                      // 当前显示的页面
    titlebar()->setTitle(QString());                                        // 重置标题栏
    m_fileListModel->reset();                                               // 重置model

    // 删除所有的页面
    if (!m_lastPage.isNull()) {
        m_lastPage->deleteLater();
    }
    m_centralLayout->setCurrentIndex(0);

    this->setAcceptDrops(true);
    m_fileChooseWidget->setAcceptDrops(true);
    // 安装完成后，清除文件选择按钮的焦点
    m_fileChooseWidget->clearChooseFileBtnFocus();
}

void DebInstaller::slotRemovePackage(const int index)
{
    m_fileListModel->slotRemovePackage(index);                                          // 后端删除某个下表的包
    const int packageCount = m_fileListModel->preparedPackages().size();            // 获取删除后的包的数量
    if (packageCount == 1) {                                                        // 删除后包的数量只有一个，从批量安装页面刷新成单包安装页面
        refreshSingle();
    }
    if (packageCount > 1)                                                           // 删除后仍然有多个包，直接刷新批量安装界面
        MulRefreshPage();
}

void DebInstaller::appendPackageStart()
{
    m_packageAppending = true;
    MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
    if (multiplePage)
        multiplePage->setEnableButton(false);
}

void DebInstaller::appendFinished()
{
    qDebug() << "[DebInstaller]" << "[appendFinished]" << "append package finished";
    MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
    if (multiplePage) {
        multiplePage->setEnableButton(true);
    }
    m_packageAppending = false;
}

void DebInstaller::MulRefreshPage()
{
    if (m_dragflag == 1) {
        MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);        //获取批量安装类的指针
        if (multiplePage)
            multiplePage->refreshModel();
    }
}

void DebInstaller::single2Multi()
{
    // 刷新文件的状态，初始化包的状态为准备状态
    m_fileListModel->resetFileStatus();
    m_fileListModel->initPrepareStatus();
    if (!m_lastPage.isNull()) m_lastPage->deleteLater();                    //清除widgets缓存

    // multiple packages install
    titlebar()->setTitle(tr("Bulk Install"));

    MultipleInstallPage *multiplePage = new MultipleInstallPage(m_fileListModel);
    multiplePage->setObjectName("MultipleInstallPage");

    connect(multiplePage, &MultipleInstallPage::signalBackToFileChooseWidget, this, &DebInstaller::slotReset);
    connect(multiplePage, &MultipleInstallPage::signalRequestRemovePackage, this, &DebInstaller::slotRemovePackage);
    multiplePage->refreshModel();
    m_lastPage = multiplePage;
    m_centralLayout->addWidget(multiplePage);
    m_dragflag = 1;
    m_Filterflag = 1;

    m_centralLayout->setCurrentIndex(1);
}

void DebInstaller::refreshSingle()
{
    m_fileChooseWidget->clearChooseFileBtnFocus();

    // 刷新文件的状态，初始化包的状态为准备状态
    m_fileListModel->resetFileStatus();
    m_fileListModel->initPrepareStatus();
    // clear widgets if needed
    if (!m_lastPage.isNull()) m_lastPage->deleteLater();                    //清除widgets缓存
    //安装器中只有一个包，刷新单包安装页面
    //刷新成单包安装界面时，删除标题
    titlebar()->setTitle(QString());

    SingleInstallPage *singlePage = new SingleInstallPage(m_fileListModel);
    singlePage->setObjectName("SingleInstallPage");
    connect(singlePage, &SingleInstallPage::signalBacktoFileChooseWidget, this, &DebInstaller::slotReset);
    connect(singlePage, &SingleInstallPage::signalRequestUninstallConfirm, this, &DebInstaller::slotShowUninstallConfirmPage);

    m_lastPage = singlePage;
    m_centralLayout->addWidget(singlePage);

    // 重置安装器拖入的状态与工作的状态
    m_dragflag = 2;
    m_Filterflag = 2;
    // switch to new page.
    m_centralLayout->setCurrentIndex(1);
}

SingleInstallPage *DebInstaller::backToSinglePage()
{
    // 获取当前的页面并删除
    QWidget *confirmPage = m_centralLayout->widget(2);
    if (nullptr == confirmPage)
        return nullptr;
    m_centralLayout->removeWidget(confirmPage);
    confirmPage->deleteLater();

    SingleInstallPage *singleInstallPage = qobject_cast<SingleInstallPage *>(m_centralLayout->widget(1));           //获取单包安装widget
    if (!singleInstallPage) {
        return nullptr;
    }
    // 返回单包安装页面时，允许添加包
    singleInstallPage->setAcceptDrops(true);
    m_fileChooseWidget->setAcceptDrops(true);
    this->setAcceptDrops(true);

    return singleInstallPage;
}

void DebInstaller::slotChangeDragFlag()
{
    repaint();
    m_dragflag = 0;                                 //允许包被拖入且此时程序中没有包

    enableCloseAndExit();
}

void DebInstaller::slotSetEnableButton(bool bButtonEnabled)
{
    //如果正在添加包，则启用按钮
    if (m_packageAppending)
        return;
    //Set button enabled after installation canceled
    if (2 == m_dragflag ) {//单包安装按钮的启用与禁用
        SingleInstallPage *singlePage = qobject_cast<SingleInstallPage *>(m_lastPage);
        if (singlePage)
            singlePage->setEnableButton(bButtonEnabled);
    } else if (1  == m_dragflag) {//批量安装按钮的启用与禁用
        MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
        if (multiplePage)
            multiplePage->setEnableButton(bButtonEnabled);
    }
}

void DebInstaller::slotShowHiddenButton()
{
    enableCloseAndExit();
    m_fileListModel->resetFileStatus();        //授权取消，重置所有的状态，包括安装状态，依赖状态等
    if (2 == m_dragflag) {// 单包安装显示按钮
        SingleInstallPage *singlePage = qobject_cast<SingleInstallPage *>(m_lastPage);
        if (singlePage)
            singlePage->afterGetAutherFalse();
    } else if (1 == m_dragflag) {//批量安装显示按钮
        MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
        if (multiplePage)
            multiplePage->afterGetAutherFalse();
    }
}

void DebInstaller::closeEvent(QCloseEvent *event)
{
    DMainWindow::closeEvent(event);
}

void DebInstaller::slotDealDependResult(int authDependsStatus, QString dependName)
{
    Q_UNUSED(dependName);
    //Set the display effect according to the status of deepin-wine installation authorization.
    //Before authorization, authorization confirmation, and when the authorization box pops up, it is not allowed to add packages.
    //依赖下载时、授权时不允许拖入
    if (authDependsStatus == DebListModel::AuthBefore || authDependsStatus == DebListModel::AuthConfirm || authDependsStatus == DebListModel::AuthPop) {
        this->setAcceptDrops(false);
    } else {
        this->setAcceptDrops(true);
    }

    if (authDependsStatus == DebListModel::AuthDependsSuccess) { //依赖下载成功
        m_fileListModel->resetFileStatus();//清除包的状态和包的错误原因
        m_fileListModel->initPrepareStatus();//重置包的prepare状态。
    }
    if (authDependsStatus == DebListModel::AuthBefore) {     //授权框弹出时
        this->setEnabled(false);                    //设置界面不可用
    } else {                                        //授权成功或失败后
        this->setEnabled(true);                     //根据授权的结果刷新单包或者批量安装界面
        if (m_fileListModel->preparedPackages().size() == 1) {          //刷新单包安装界面
            SingleInstallPage *singlePage = qobject_cast<SingleInstallPage *>(m_lastPage);
            if (singlePage)
                singlePage->DealDependResult(authDependsStatus, dependName);
        } else if (m_fileListModel->preparedPackages().size() >= 2) {       //刷新批量安装界面
            MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
            if (multiplePage) {
                multiplePage->DealDependResult(authDependsStatus, dependName);
                multiplePage->refreshModel();// 滚动到最后一行。
            }
        }
    }
}

bool DebInstaller::checkSuffix(QString filePath)
{
    const QFileInfo info(filePath);
    if (info.isFile() && info.suffix().toLower() == "deb") {        //大小写不敏感的判断是否为deb后缀
        return true;
    }
    return false;

}
