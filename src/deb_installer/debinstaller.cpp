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
#include "deblistmodel.h"
#include "filechoosewidget.h"
#include "multipleinstallpage.h"
#include "singleinstallpage.h"
#include "uninstallconfirmpage.h"
#include "utils.h"

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
    , m_centralLayout(new QStackedLayout(this))
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

    qApp->installEventFilter(this);

    //file choose widget settings
    m_fileChooseWidget->setObjectName("FileChooseWidget");
    m_centralLayout->addWidget(m_fileChooseWidget);
    m_centralLayout->setContentsMargins(0, 0, 0, 0);
    m_centralLayout->setSpacing(0);
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
    handleFocusPolicy();

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
                QWidget *widget = widgetLayout->itemAt(j)->widget();
                if (widget != nullptr && QString(widget->metaObject()->className()) ==  QString("QWidget")) {
                    QLayout *wLayout = widget->layout();
                    for (int k = 0; k < wLayout->count(); ++k) {
                        QWidget *widget = wLayout->itemAt(k)->widget();
                        if (widget != nullptr && QString(widget->metaObject()->className()).contains("Button")) {
                            //widget->setFocusPolicy(Qt::NoFocus);
                            if ("Dtk::Widget::DWindowOptionButton" == QString(widget->metaObject()->className())) {
                                m_OptionWindow = widget;
                            }
                            if ("Dtk::Widget::DWindowMinButton" == QString(widget->metaObject()->className())) {
                                m_MinWindow = widget;
                            }
                            if ("Dtk::Widget::DWindowCloseButton" == QString(widget->metaObject()->className())) {
                                m_closeWindow = widget;
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
    //Append packages via file-choose-widget's file-choose-button
    connect(m_fileChooseWidget, &FileChooseWidget::packagesSelected, this, &DebInstaller::onPackagesSelected);

    //Select the focus of the page
    connect(m_fileChooseWidget, &FileChooseWidget::OutOfFocus, this, &DebInstaller::ResetFocus);

    //Determine the status of the current application based on the status of the authorization box.
    connect(m_fileListModel, &DebListModel::lockForAuth, this, &DebInstaller::onAuthing);

    //show dpkg details
    connect(m_fileListModel, &DebListModel::appendOutputInfo, this, [ = ](const QString & output) {
        qDebug() << "append output info:*****" << output.trimmed();
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

    //Append packages via double-clicked or right-click
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::newProcessInstance, this, &DebInstaller::onNewAppOpen);

    connect(m_fileListModel, &DebListModel::enableCloseButton, this, &DebInstaller::enableCloseButton);
}

void DebInstaller::enableCloseButton(bool enable)
{
    qDebug() << "enable close and exit? " << enable;
    if (enable) {
        enableCloseAndExit();
    } else {
        disableCloseAndExit();
    }
}

/**
 * @brief disableCloseAndExit
 * 设置退出和关闭按钮为不可用
 */
// closed is forbidden during install/uninstall
void DebInstaller::disableCloseAndExit()
{

    qDebug() << "disableCloseAndExit";
    titlebar()->setDisableFlags(Qt::WindowCloseButtonHint);
    QMenu *titleMenu = titlebar()->menu();
    QList<QAction *> actions = titleMenu->actions();
    QAction *action = actions.last();
    action->setDisabled(true);

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
    qDebug() << "enableCloseAndExit";
    titlebar()->setDisableFlags(titlebar()->windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowCloseButtonHint);

    QMenu *titleMenu = titlebar()->menu();
    QList<QAction *> actions = titleMenu->actions();
    QAction *action = actions.last();
    action->setDisabled(false);

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
    //Solve flash problems when installing and reinstalling
    m_OptionWindow->clearFocus();

    disableCloseAndExit();
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

    QStringList debFileList;
    for (int i = 0; i < arguments.size(); i++) {
        QString strArg = arguments.at(i);
        if (!strArg.contains("deb-installer")) {
            const QFileInfo info(strArg);
            if (info.isFile() && info.suffix() == "deb") {
                debFileList << strArg;
            }
        }
    }

    if (debFileList.size() > 0) {
        qDebug() << debFileList << endl;
        onPackagesSelected(debFileList);
    }

    this->setWindowState(Qt::WindowActive);
    this->activateWindow();
}


void DebInstaller::keyPressEvent(QKeyEvent *e)
{
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
    this->activateWindow();
    if (m_fileListModel->m_workerStatus_temp == DebListModel::WorkerProcessing) {
        this->setAcceptDrops(false);
    } else {
        m_fileChooseWidget->setAcceptDrops(true);
        if (m_dragflag == 0)
            return;

        auto *const mime = e->mimeData();
        if (!mime->hasUrls()) return e->ignore();

        for (const auto &item : mime->urls()) {
            const QFileInfo info(item.path());
            if (info.isDir()) return e->accept();
            if (info.isFile() && info.suffix() == "deb") return e->accept();
        }

        e->ignore();
    }
}

//Accept the file which suffix is deb and append to application
void DebInstaller::dropEvent(QDropEvent *e)
{
    auto *const mime = e->mimeData();
    if (!mime->hasUrls()) return e->ignore();

    e->accept();

    // find .deb files
    QStringList file_list;
    for (const auto &url : mime->urls()) {
        if (!url.isLocalFile()) continue;
        const QString local_path = url.toLocalFile();
        const QFileInfo info(local_path);

        if (info.isFile() && info.suffix() == "deb")
            file_list << local_path;
        else if (info.isDir()) {
            for (auto deb : QDir(local_path).entryInfoList(QStringList() << "*.deb", QDir::Files))
                file_list << deb.absoluteFilePath();
        }
    }
    this->activateWindow();
    onPackagesSelected(file_list);
}

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
    this->showNormal();
    this->activateWindow();
    int packageCountInit = m_fileListModel->preparedPackages().size();
    qDebug() << "m_fileListModel->m_workerStatus_temp+++++++" << m_fileListModel->m_workerStatus_temp;
    if ((!m_lastPage.isNull() && m_fileListModel->m_workerStatus_temp != DebListModel::WorkerPrepare) ||
            m_fileListModel->m_workerStatus_temp == DebListModel::WorkerProcessing ||
            m_fileListModel->m_workerStatus_temp == DebListModel::WorkerUnInstall) {
        qDebug() << "return";
        return;
    } else {
        qDebug() << "append Package";
        for (const auto &package : packages) {
            QApt::DebFile *m_pDebPackage = new QApt::DebFile(package);
            bool isValid =  m_pDebPackage->isValid();
            delete m_pDebPackage;
            if (!isValid) {
                qWarning() << "package invalid: " << package;
                // this is a suggestion, add Floating Message while package invalid
//                DFloatingMessage *msg = new DFloatingMessage;
//                msg->setMessage(tr("Package Invalid"));
//                DMessageManager::instance()->sendMessage(this, msg);
                continue;
            }
            DRecentData data;
            data.appName = "Deepin Deb Installer";
            data.appExec = "deepin-deb-installer";
            DRecentManager::addItem(package, data);
            // Decide how to add according to the number of packages in the application
            if (!m_fileListModel->appendPackage(package)) {
                qWarning() << "package is Exist! ";

                DFloatingMessage *msg = new DFloatingMessage;
                msg->setMessage(tr("Already Added"));
                DMessageManager::instance()->sendMessage(this, msg);
                if (packages.size() == 1) {
                    return;
                }
            }
        }
        //fix bug29948 服务器版
        const int packageCount = m_fileListModel->preparedPackages().size();
        // There is already one package and there will be multiple packages to be added
        if (packageCount == packageCountInit) {
            return ;
        }
        if (packageCount == 1 || packages.size() > 1) {
            refreshInstallPage(packageCount);
            return;
        }
        // There was a package from the beginning and it was added
        if (packageCountInit == 1 && packageCount > 1) {
            refreshInstallPage(packageCount);
        } else {
            m_dragflag = 1;
            MulRefreshPage(packageCount);
            m_fileListModel->initDependsStatus(packageCountInit);
            MulRefreshPage(packageCount);
        }
    }
}

/**
 * @brief showUninstallConfirmPage
 * 卸载按钮的槽函数
 * 显示卸载界面
 */
void DebInstaller::showUninstallConfirmPage()
{
    Q_ASSERT(m_centralLayout->count() == 2);
    m_fileListModel->m_workerStatus_temp = DebListModel::WorkerUnInstall;

    this->setAcceptDrops(false);

    const QModelIndex index = m_fileListModel->first();

    //  Set the display information of the uninstall page
    UninstallConfirmPage *p = new UninstallConfirmPage(this);
    p->setRequiredList(index.data(DebListModel::PackageReverseDependsListRole).toStringList());
    p->setPackage(index.data().toString());

    m_Filterflag = 3; //Uninstall the page
    m_UninstallPage = p;

    m_centralLayout->addWidget(p);
    m_centralLayout->setCurrentIndex(2);
    p->setAcceptDrops(false);

    connect(p, &UninstallConfirmPage::OutOfFocus, this, &DebInstaller::ResetFocus);
    connect(p, &UninstallConfirmPage::accepted, this, &DebInstaller::onUninstallAccepted);
    connect(p, &UninstallConfirmPage::canceled, this, &DebInstaller::onUninstallCancel);
}

/**
 * @brief onUninstallAccepted
 * 卸载界面确认卸载按钮的槽函数
 * 卸载开始时，返回singleInstallPage 并显示卸载进程。
 */
void DebInstaller::onUninstallAccepted()
{
    // uninstall begin
    SingleInstallPage *p = backToSinglePage();
    m_fileChooseWidget->setAcceptDrops(true);
    p->uninstallCurrentPackage();

    //set close button disabled while uninstalling
    disableCloseAndExit();

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
    this->setAcceptDrops(true);
    m_fileListModel->m_workerStatus_temp = DebListModel::WorkerPrepare;
    backToSinglePage();

    m_Filterflag = m_dragflag;
}

/**
 * @brief onAuthing
 * @param authing 按钮是否可用的标志
 * 授权框弹出后，设置当前界面为不可用状态
 */
void DebInstaller::onAuthing(const bool authing)
{
    //The authorization box pops up, the setting button is not available
    setEnabled(!authing);
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
    m_fileListModel->m_workerStatus_temp = 0;
    m_dragflag = -1;
    m_Filterflag = -1;
    titlebar()->setTitle(QString());
    m_fileListModel->reset();
    m_lastPage->deleteLater();
    m_UninstallPage->deleteLater();
    m_centralLayout->setCurrentIndex(0);
}

/**
 * @brief removePackage
 * @param index 要删除的包的下标
 * 根据传入的下表删除某个包。
 */
void DebInstaller::removePackage(const int index)
{
    m_fileListModel->removePackage(index);
    const int packageCount = m_fileListModel->preparedPackages().size();
    if (packageCount == 1) {
        refreshInstallPage(index);
    }
    if (packageCount > 1)
        MulRefreshPage(index);
}

/**
 * @brief MulRefreshPage
 * @param index 某一个包的下标位置
 *
 * 刷新multiInstallPage并在刷新后滚动到下标处
 */
void DebInstaller::MulRefreshPage(int index)
{
    if (m_dragflag == 1) {
        MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
        multiplePage->setScrollBottom(index);
    }
}

/**
 * @brief refreshInstallPage
 * @param index 某一个包的下标
 * 刷新安装界面 多用于singleInstallPage 转换到 multiSinglePage
 * 如果传入的下标不为-1， 则刷新时滚动到下标处
 */
void DebInstaller::refreshInstallPage(int index)
{
    m_fileListModel->reset_filestatus();
    m_fileListModel->initPrepareStatus();
    // clear widgets if needed
    if (!m_lastPage.isNull()) m_lastPage->deleteLater();

    const int packageCount = m_fileListModel->preparedPackages().size();
    // no packages found
    if (packageCount == 0) return;

    if (packageCount == 1) {
        // single package install
        titlebar()->setTitle(QString());

        SingleInstallPage *singlePage = new SingleInstallPage(m_fileListModel);
        singlePage->setObjectName("SingleInstallPage");
        connect(singlePage, &SingleInstallPage::OutOfFocus, this, &DebInstaller::ResetFocus);
        connect(singlePage, &SingleInstallPage::back, this, &DebInstaller::reset);
        connect(singlePage, &SingleInstallPage::requestUninstallConfirm, this, &DebInstaller::showUninstallConfirmPage);

        m_lastPage = singlePage;
        m_fileListModel->DebInstallFinishedFlag = 0;
        m_centralLayout->addWidget(singlePage);
        m_dragflag = 2;
        m_Filterflag = 2;
    } else {
        // multiple packages install
        titlebar()->setTitle(tr("Bulk Install"));

        MultipleInstallPage *multiplePage = new MultipleInstallPage(m_fileListModel);
        multiplePage->setObjectName("MultipleInstallPage");

        connect(multiplePage, &MultipleInstallPage::OutOfFocus, this, &DebInstaller::ResetFocus);
        connect(multiplePage, &MultipleInstallPage::back, this, &DebInstaller::reset);
        connect(multiplePage, &MultipleInstallPage::requestRemovePackage, this, &DebInstaller::removePackage);
        multiplePage->setScrollBottom(index);
        m_lastPage = multiplePage;
        m_centralLayout->addWidget(multiplePage);
        m_dragflag = 1;
        m_Filterflag = 1;
    }
    // switch to new page.
    m_centralLayout->setCurrentIndex(1);
}

/**
 * @brief backToSinglePage 返回单包安装界面
 * @return SingleInstallPage* SingleInstallPage的指针
 */
SingleInstallPage *DebInstaller::backToSinglePage()
{
    Q_ASSERT(m_centralLayout->count() == 3);
    QWidget *confirmPage = m_centralLayout->widget(2);
    m_centralLayout->removeWidget(confirmPage);
    confirmPage->deleteLater();

    SingleInstallPage *p = qobject_cast<SingleInstallPage *>(m_centralLayout->widget(1));
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
    m_dragflag = 0;

    enableCloseAndExit();
}

/**
 * @brief setEnableButton
 * @param bEnable 按钮是否可用标志
 * 根据当前的安装/卸载进程来控制singleInstallPage/multiInstallPage按钮的可用性
 */
void DebInstaller::setEnableButton(bool bEnable)
{
    //Set button enabled after installation canceled
    if (m_dragflag == 2) {
        SingleInstallPage *singlePage = qobject_cast<SingleInstallPage *>(m_lastPage);
        singlePage->setEnableButton(bEnable);
    } else if (m_dragflag == 1) {
        MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
        multiplePage->setEnableButton(bEnable);
    }
}

/**
 * @brief showHiddenButton
 * 授权取消后显示被隐藏的按钮
 */
void DebInstaller::showHiddenButton()
{
    //After the installation is complete, the hidden button is displayed and the close button is available
    enableCloseAndExit();
    m_fileListModel->reset_filestatus();

    if (m_dragflag == 2) {
        SingleInstallPage *singlePage = qobject_cast<SingleInstallPage *>(m_lastPage);
        singlePage->afterGetAutherFalse();
    } else if (m_dragflag == 1) {
        MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
        multiplePage->afterGetAutherFalse();
    }
}

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
    if (iAuthRes == DebListModel::AuthBefore || iAuthRes == DebListModel::AuthConfirm || iAuthRes == DebListModel::AuthPop) {
        this->setAcceptDrops(false);
    } else {
        this->setAcceptDrops(true);
    }
    //Refresh the page after successful installation.
    if (iAuthRes == DebListModel::AuthDependsSuccess)
        refreshInstallPage();
    //Refresh the display effect of different pages
    if (m_dragflag == 2) {
        SingleInstallPage *singlePage = qobject_cast<SingleInstallPage *>(m_lastPage);
        singlePage->DealDependResult(iAuthRes, dependName);
    } else if (m_dragflag == 1) {
        MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
        multiplePage->DealDependResult(iAuthRes, dependName);
    }
}

bool DebInstaller::eventFilter(QObject *watched, QEvent *event)
{
    if (QEvent::WindowDeactivate == event->type()) {
        m_OptionWindow->clearFocus();
        bActiveWindowFlag = true;
        return QObject::eventFilter(watched, event);
    }
    if (QEvent::WindowActivate == event->type()) {
        m_OptionWindow->clearFocus();
        m_MinWindow->clearFocus();
        m_closeWindow->clearFocus();
        bActiveWindowFlag = false;
        bTabFlag = false;
        //Refresh when focus is restored
        this->repaint();
        this->update();
        titlebar()->repaint();
        titlebar()->update();
        return QObject::eventFilter(watched, event);
    }

    //Filtering events are not handled when focus is lost
    if (bActiveWindowFlag)
        return QObject::eventFilter(watched, event);

    //Remove focus when clicking on the title bar
    if (watched == titlebar()) {
        if (QEvent::MouseButtonRelease == event->type()) {
            ResetFocus(false);
        }
    }

    if (QEvent::MouseButtonRelease == event->type()) {
        if (watched == m_OptionWindow) {
            if (m_iOptionWindowFlag != 1) {
                return QObject::eventFilter(watched, event);
            } else {
                QPoint pos = m_OptionWindow->pos();
                pos.setY(pos.y() + 49);
                titlebar()->menu()->exec(m_OptionWindow->mapToGlobal(pos));
            }
        } else if (watched == m_MinWindow) {
            if (m_iOptionWindowFlag != 2) {
                return QObject::eventFilter(watched, event);
            } else {
                this->showMinimized();
            }
        } else if (watched == m_closeWindow) {
            if (m_iOptionWindowFlag != 3) {
                return QObject::eventFilter(watched, event);
            } else {
                this->close();
            }
        } else {
            m_iOptionWindowFlag = 0;
            if (m_OptionWindow->hasFocus()) m_iOptionWindowFlag = 1;
            if (m_MinWindow->hasFocus()) m_iOptionWindowFlag = 2;
            if (m_closeWindow->hasFocus()) m_iOptionWindowFlag = 3;
            if (this->focusWidget() != nullptr) {
                this->focusWidget()->clearFocus();
                bTabFlag = false;
                return QObject::eventFilter(watched, event);
            }
        }
    }

    //Fixed flash problem after single install click cancel uninstall
    if (event->type() == QEvent::FocusIn) {
        if (m_OptionWindow == watched) {
            if (!bTabFlag) {
                m_OptionWindow->clearFocus();
                return true;
            }
            return QObject::eventFilter(watched, event);
        }
    }

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *key_event = static_cast < QKeyEvent *>(event); //Convert events to keyboard events
        if (key_event->key() == Qt::Key_Escape) {
            if (titlebar()->menu()->isVisible()) {
                titlebar()->menu()->hide();
            }
            return true;
        }
        if (key_event->key() == Qt::Key_Tab) {
            if (!bTabFlag) {
                bTabFlag = true;
                m_OptionWindow->setFocus();
                return true;
            }
            if (m_OptionWindow->hasFocus()) {
                m_MinWindow->setFocus();
                return true;
            }
            if (m_MinWindow->hasFocus()) {
                //During installation, when the close button is not available, switch TAB from scratch
                if (m_closeWindow->isEnabled())
                    m_closeWindow->setFocus();
                else {
                    bTabFlag = false;
                    m_MinWindow->clearFocus();

                    if (m_Filterflag == 2) {
                        SingleInstallPage *singlePage = qobject_cast<SingleInstallPage *>(m_lastPage);
                        singlePage->grabKeyboard();
                        qApp->installEventFilter(singlePage);
                        singlePage->setInfoControlButtonFocus();
                    } else if (m_Filterflag == 1) {
                        MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
                        qApp->installEventFilter(multiplePage);
                        multiplePage->setInitSelect();
                    }
                }
                return true;
            }

            if (m_closeWindow->hasFocus()) {
                switch (m_Filterflag) {
                case -1: {
                    //Initial selection of file interface
                    qApp->installEventFilter(m_fileChooseWidget);
                    m_fileChooseWidget->setChooseBtnFocus();
                    break;
                }
                case 1: {
                    //The batch installation
                    MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
                    qApp->installEventFilter(multiplePage);
                    multiplePage->setInitSelect();
                    break;
                }
                case 2: {
                    //Single installation interface
                    SingleInstallPage *singlePage = qobject_cast<SingleInstallPage *>(m_lastPage);
                    singlePage->grabKeyboard();
                    qApp->installEventFilter(singlePage);
                    if (singlePage->m_currentFlag == 1)
                        singlePage->m_installButton->setFocus();
                    if (singlePage->m_currentFlag == 2)
                        singlePage->m_uninstallButton->setFocus();
                    if (singlePage->m_currentFlag == 3 || singlePage->m_currentFlag == 4)
                        singlePage->m_infoControlButton->setFocus();
                    break;
                }
                case 3: {
                    //Uninstall interface
                    if (!m_UninstallPage.isNull()) {
                        UninstallConfirmPage *uninstallPage = qobject_cast<UninstallConfirmPage *>(m_UninstallPage);
                        uninstallPage->grabKeyboard();
                        qApp->installEventFilter(uninstallPage);
                        uninstallPage->initSetFocus();
                    }
                    break;
                }
                default:
                    break;
                }
            }
            return true;
        } else {
            if (key_event->key() == Qt::Key_Up || key_event->key() == Qt::Key_Left) {
                if (m_MinWindow == watched)
                    bTabFlag = true;
            }
            if (key_event->key() == Qt::Key_Up || key_event->key() == Qt::Key_Left) {
                if (m_OptionWindow == watched)
                    bTabFlag = false;
            }
            if (key_event->key() == Qt::Key_Down || key_event->key() == Qt::Key_Right) {
                if (m_closeWindow == watched)
                    bTabFlag = false;
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

/**
 * @brief ResetFocus
 * @param bFlag
 */
void DebInstaller::ResetFocus(bool bFlag)
{
    this->repaint();
    this->update();

    //Remove the corresponding filter
    switch (m_Filterflag) {
    case -1: {
        qApp->removeEventFilter(m_fileChooseWidget);
        break;
    }
    case 1: {
        MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
        qApp->removeEventFilter(multiplePage);
        multiplePage->releaseKeyboard();
        break;
    }
    case 2: {
        SingleInstallPage *singlePage = qobject_cast<SingleInstallPage *>(m_lastPage);
        qApp->removeEventFilter(singlePage);
        break;
    }
    case 3: {
        if (!m_UninstallPage.isNull()) {
            UninstallConfirmPage *uninstallPage = qobject_cast<UninstallConfirmPage *>(m_UninstallPage);
            qApp->removeEventFilter(uninstallPage);
        }
        break;
    }
    default:
        break;
    }

    if (bFlag) {
        m_OptionWindow->setFocus();
    } else {
        bTabFlag = false;
        m_OptionWindow->clearFocus();
    }
}
