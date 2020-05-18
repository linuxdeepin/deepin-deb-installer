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
#include "AddPackageThread.h"
#include "AppendLoadingWidget.h"

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
#include <QApt/DebFile>

#include <QApplication>
#include <QDesktopWidget>

#include <DInputDialog>
#include <DRecentManager>
#include <DMessageManager>
#include <DTitlebar>
#include <DGuiApplicationHelper>

using QApt::DebFile;

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

DebInstaller::DebInstaller(QWidget *parent)
    : DMainWindow(parent)
    , m_fileListModel(new DebListModel(this))
    , m_fileChooseWidget(new FileChooseWidget(this))
    , m_centralLayout(new QStackedLayout(this))
    , m_dragflag(-1)
{
    initUI();
    initConnections();
}

DebInstaller::~DebInstaller() {}

void DebInstaller::initUI()
{
    //隐藏标题栏下的阴影
    setTitlebarShadowEnabled(false);

    m_fileChooseWidget->setObjectName("FileChooseWidget");
    m_centralLayout->addWidget(m_fileChooseWidget);
    m_centralLayout->setContentsMargins(0, 0, 0, 0);
    m_centralLayout->setSpacing(0);

    QWidget *wrapWidget = new QWidget;
    wrapWidget->setLayout(m_centralLayout);

//#define SHOWBORDER
#ifdef SHOWBORDER
    wrapWidget->setStyleSheet("QWidget{border:1px solid black;}");
#endif

    DTitlebar *tb = titlebar();
    tb->setIcon(QIcon::fromTheme("deepin-deb-installer"));
    tb->setTitle("");
    tb->setAutoFillBackground(false);

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

    checkWhiteList();
}

void DebInstaller::checkWhiteList()
{
    QFile whiteFile("/usr/share/deepin-elf-verify/whitelist");
    if (!whiteFile.exists()) {
        popErrorWindow();
    }
}
void DebInstaller::popErrorWindow()
{
    DDialog *Ddialog = new DDialog();
    Ddialog->setModal(true);
    Ddialog->setWindowFlag(Qt::WindowStaysOnTopHint);
    Ddialog->setTitle(tr("Unable to install"));
    Ddialog->setMessage(QString(tr("No whitelist file was detected in the system, please check again.")));
    Ddialog->setIcon(QIcon(Utils::renderSVG(":/images/warning.svg", QSize(32, 32))));
    Ddialog->addButton(QString(tr("OK")), true, DDialog::ButtonNormal);
    Ddialog->show();
    QPushButton *btnOK = qobject_cast<QPushButton *>(Ddialog->getButton(0));
    connect(Ddialog, &DDialog::aboutToClose, this, [ = ] {
        exit(0);
    });
    connect(btnOK, &DPushButton::clicked, this, [ = ] {
        exit(0);
    });
}

void DebInstaller::handleFocusPolicy()
{
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
                            widget->setFocusPolicy(Qt::NoFocus);
                        }
                    }
                }
            }
        }
    }
}

void DebInstaller::initConnections()
{
    connect(m_fileChooseWidget, &FileChooseWidget::packagesSelected, this, &DebInstaller::onPackagesSelected);
    connect(m_fileListModel, &DebListModel::lockForAuth, this, &DebInstaller::onAuthing);
    connect(m_fileListModel, &DebListModel::appendOutputInfo, this, [ = ](const QString & output) {
        qDebug() << "append output info:*****" << output.trimmed();
    });

    connect(m_fileListModel, &DebListModel::workerFinished, this, &DebInstaller::changeDragFlag);
    connect(m_fileListModel, &DebListModel::AuthCancel, this, &DebInstaller::showHiddenButton);
    connect(m_fileListModel, &DebListModel::onStartInstall, this, &DebInstaller::onStartInstallRequested);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::newProcessInstance, this, &DebInstaller::onNewAppOpen);
    connect(m_fileListModel, &DebListModel::EnableReCancelBtn, this, &DebInstaller::setEnableButton);
}

void DebInstaller::disableCloseAndExit()
{
    qDebug() << "disableCloseAndExit";
    titlebar()->setDisableFlags(Qt::WindowCloseButtonHint);
    QMenu *titleMenu = titlebar()->menu();
    QList<QAction *> actions = titleMenu->actions();
    QAction *action = actions.last();
    action->setDisabled(true);
}

void DebInstaller::enableCloseAndExit()
{
    qDebug() << "enableCloseAndExit";
    titlebar()->setDisableFlags(titlebar()->windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowCloseButtonHint);

    QMenu *titleMenu = titlebar()->menu();
    QList<QAction *> actions = titleMenu->actions();
    QAction *action = actions.last();
    action->setDisabled(false);
}

void DebInstaller::onStartInstallRequested()
{
    disableCloseAndExit();
}

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

void DebInstaller::dragEnterEvent(QDragEnterEvent *e)
{
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
    this->setWindowState(Qt::WindowActive);
    this->activateWindow();
    onPackagesSelected(file_list);
}

void DebInstaller::dragMoveEvent(QDragMoveEvent *e)
{
    e->accept();
}

void DebInstaller::onPackagesSelected(const QStringList &packages)
{
    //只有在第一次添加比较大的包或者第一次添加多个包的时候才会有加载动画
    QApt::DebFile *p = new DebFile(packages[0]);
    if (m_fileListModel->preparedPackages().size() == 0) {
        if (packages.size() > 1) {
            packagesSelectedThread(packages, true);
            return;
        } else if (packages.size() == 1 || (p && p->installedSize() > 50000)) {
            packagesSelectedThread(packages, true);
            return;
        }
    }
    packagesSelectedThread(packages, false);
    return;
}

void DebInstaller::packagesSelectedThread(const QStringList &packages, bool animation)
{
    m_fileChooseWidget->hide();

    if (animation) {
        m_pSpinner = new AppendLoadingWidget(this);

        if (!m_lastPage.isNull()) m_lastPage->deleteLater();
        m_pSpinner->setObjectName("AppendLoadingWidget");
        m_centralLayout->addWidget(m_pSpinner);
        m_centralLayout->setContentsMargins(0, 0, 0, 0);
        m_centralLayout->setSpacing(0);

        m_pSpinner->show();
        m_centralLayout->addWidget(m_pSpinner);
        m_centralLayout->setAlignment(m_pSpinner, Qt::AlignBottom);

        m_lastPage = m_pSpinner;
        m_centralLayout->setCurrentWidget(m_pSpinner);
    }

    m_pAddPackageThread = new AddPackageThread(m_fileListModel, m_lastPage, packages, this);
    connect(m_pAddPackageThread, &AddPackageThread::refresh, this, &DebInstaller::refreshInstallPage);
    connect(m_pAddPackageThread, &AddPackageThread::packageAlreadyAdd, this, [ = ] {
        qWarning() << "package is Exist! ";

        DFloatingMessage *msg = new DFloatingMessage;
        msg->setMessage(tr("Already Added"));
        DMessageManager::instance()->sendMessage(this, msg);
    });
    m_pAddPackageThread->start();
}

void DebInstaller::showUninstallConfirmPage()
{
    Q_ASSERT(m_centralLayout->count() == 2);
    m_fileListModel->m_workerStatus_temp = DebListModel::WorkerUnInstall;

    const QModelIndex index = m_fileListModel->first();

    UninstallConfirmPage *p = new UninstallConfirmPage(this);
    p->setRequiredList(index.data(DebListModel::PackageReverseDependsListRole).toStringList());
    p->setPackage(index.data().toString());

    m_centralLayout->addWidget(p);
    m_centralLayout->setCurrentIndex(2);

    connect(p, &UninstallConfirmPage::accepted, this, &DebInstaller::onUninstallAccepted);
    connect(p, &UninstallConfirmPage::canceled, this, &DebInstaller::onUninstallCalceled);

    this->setAcceptDrops(false);
    p->setAcceptDrops(false);
}

void DebInstaller::onUninstallAccepted()
{
    SingleInstallPage *p = backToSinglePage();
    m_fileChooseWidget->setAcceptDrops(true);
    p->uninstallCurrentPackage();
}

void DebInstaller::onUninstallCalceled()
{
    this->setAcceptDrops(true);
    m_fileListModel->m_workerStatus_temp = DebListModel::WorkerPrepare;

    backToSinglePage();
}

void DebInstaller::onAuthing(const bool authing)
{
    setEnabled(!authing);
}

void DebInstaller::reset()
{
    Q_ASSERT(m_centralLayout->count() == 2);
    Q_ASSERT(!m_lastPage.isNull());
    m_fileListModel->m_workerStatus_temp = 0;
    m_dragflag = -1;
    titlebar()->setTitle(QString());
    m_fileListModel->reset();
    m_lastPage->deleteLater();
    m_centralLayout->setCurrentIndex(0);
}

void DebInstaller::removePackage(const int index)
{
    m_fileListModel->removePackage(index);
    qDebug() << "remove";
    refreshInstallPage(index);
}

void DebInstaller::refreshInstallPage(int idx)
{
    qDebug() << "refresh";
    m_fileListModel->reset_filestatus();
    // clear widgets if needed
    if (!m_lastPage.isNull()) m_lastPage->deleteLater();

    const int packageCount = m_fileListModel->preparedPackages().size();
    // no packages found
    if (packageCount == 0) return;

    if (packageCount == 1) {
        // single package install
        titlebar()->setTitle(QString());
        singlePage = new SingleInstallPage(m_fileListModel);
        if (idx == -1) {
            connect(m_pAddPackageThread, &AddPackageThread::addSingleFinish, this, [ = ](bool enable) {
                qDebug() << "single page set enable ";
                singlePage->setEnableButton(enable);
            });
            singlePage->setEnableButton(false);
        }
        singlePage->setObjectName("SingleInstallPage");
        connect(singlePage, &SingleInstallPage::back, this, &DebInstaller::reset);
        connect(singlePage, &SingleInstallPage::requestUninstallConfirm, this, &DebInstaller::showUninstallConfirmPage);
        m_lastPage = singlePage;
        m_fileListModel->DebInstallFinishedFlag = 0;
        m_centralLayout->addWidget(singlePage);
        m_dragflag = 2;
    } else {
        // multiple packages install
        titlebar()->setTitle(tr("Bulk Install"));
        multiplePage = new MultipleInstallPage(m_fileListModel);
        if (idx == -1) {
            connect(m_pAddPackageThread, &AddPackageThread::addMultiFinish, this, [ = ](bool enable) {
                qDebug() << "single page set enable ";
                multiplePage->setEnableButton(enable);
            });
            multiplePage->setEnableButton(false);
        }
        connect(m_pAddPackageThread, &AddPackageThread::addStart, this, [ = ] {
            multiplePage->setEnableButton(false);
        });
        multiplePage->setObjectName("MultipleInstallPage");

        connect(multiplePage, &MultipleInstallPage::back, this, &DebInstaller::reset);
        connect(multiplePage, &MultipleInstallPage::requestRemovePackage, this, &DebInstaller::removePackage);

        if (idx) {
            qDebug() << "setScrollBottom";
            multiplePage->setScrollBottom(idx);
        }

        m_lastPage = multiplePage;
        m_centralLayout->addWidget(multiplePage);
        m_dragflag = 1;
        usleep(250 * 1000);
    }
    // switch to new page.
    m_centralLayout->setCurrentIndex(1);
    qDebug() << "refresh end";
}

SingleInstallPage *DebInstaller::backToSinglePage()
{
    Q_ASSERT(m_centralLayout->count() == 3);
    QWidget *confirmPage = m_centralLayout->widget(2);
    m_centralLayout->removeWidget(confirmPage);
    confirmPage->deleteLater();

    SingleInstallPage *p = qobject_cast<SingleInstallPage *>(m_centralLayout->widget(1));
    Q_ASSERT(p);

    p->setAcceptDrops(true);
    m_fileChooseWidget->setAcceptDrops(true);
    this->setAcceptDrops(true);

    return p;
}

void DebInstaller::changeDragFlag()
{
    repaint();
    m_dragflag = 0;

    enableCloseAndExit();
}

void DebInstaller::setEnableButton(bool bEnable)
{
    if (m_dragflag == 2) {
        SingleInstallPage *singlePage = qobject_cast<SingleInstallPage *>(m_lastPage);
        singlePage->setEnableButton(bEnable);
    } else if (m_dragflag == 1) {
        MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
        multiplePage->setEnableButton(bEnable);
    }

}

void DebInstaller::showHiddenButton()
{
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
