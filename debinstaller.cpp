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

#include <DRecentManager>
#include <DMessageManager>
#include <DTitlebar>
#include <DApplication>

using QApt::DebFile;

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

DebInstaller::DebInstaller(DWidget *parent)
    : DMainWindow(parent)
    , m_fileListModel(new DebListModel(this))
    , m_fileChooseWidget(new FileChooseWidget)
    , m_centralLayout(new QStackedLayout)
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

    DWidget *wrapWidget = new DWidget;
    wrapWidget->setLayout(m_centralLayout);

//#define SHOWBORDER
#ifdef SHOWBORDER
    wrapWidget->setStyleSheet("QWidget{border:1px solid black;}");
#endif

//    const auto ratio = devicePixelRatioF();
//    QPixmap iconPix = Utils::renderSVG(":/images/logo.svg", QSize(32, 32));
//    iconPix.setDevicePixelRatio(ratio);

    QString fontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansMedium);
    QFont font = Utils::loadFontBySizeAndWeight(fontFamily, 14, QFont::Medium);

    DTitlebar *tb = titlebar();
    tb->setIcon(QIcon::fromTheme("deepin-deb-installer"));
    tb->setTitle("");
    tb->setFont(font);
    tb->setAutoFillBackground(false);

    setCentralWidget(wrapWidget);  //将给定的小部件设置为主窗口的中心小部件。
    setAcceptDrops(true);          //启用了drop事件
    setFixedSize(480, 380);
    setWindowTitle(tr("Deepin Package Manager"));
    setWindowIcon(QIcon::fromTheme("deepin-deb-installer"));  //仅仅适用于windows系统
    move(qApp->primaryScreen()->geometry().center() - geometry().center());
}

void DebInstaller::initConnections()
{
    connect(m_fileChooseWidget, &FileChooseWidget::packagesSelected, this, &DebInstaller::onPackagesSelected);
    connect(m_fileListModel, &DebListModel::lockForAuth, this, &DebInstaller::onAuthing);
    connect(m_fileListModel, &DebListModel::appendOutputInfo, this,
    [ = ](const QString & output) {
        qDebug() << output.trimmed();
    });

    connect(m_fileListModel, &DebListModel::workerFinished, this, &DebInstaller::changeDragFlag);
    connect(m_fileListModel, &DebListModel::AuthCancel, this, &DebInstaller::showHiddenButton);
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

    onPackagesSelected(file_list);
}

void DebInstaller::dragMoveEvent(QDragMoveEvent *e)
{
    e->accept();
}

void DebInstaller::onPackagesSelected(const QStringList &packages)
{
    for (const auto &package : packages) {
        DebFile *p = new DebFile(package);
        if (!p->isValid()) {
            qWarning() << "package invalid: " << package;

            delete p;
            continue;
        }

        DRecentData data;
        data.appName = "Deepin Deb Installer";
        data.appExec = "deepin-deb-installer";
        DRecentManager::addItem(package, data);

        if (!m_fileListModel->appendPackage(p)) {
            qWarning() << "package is Exist! ";

            DFloatingMessage *msg = new DFloatingMessage;
            msg->setMessage(tr("Already Added"));
            DMessageManager::instance()->sendMessage(this, msg);
        }
    }

    refreshInstallPage();
}

void DebInstaller::showUninstallConfirmPage()
{
    Q_ASSERT(m_centralLayout->count() == 2);

    const QModelIndex index = m_fileListModel->first();

    UninstallConfirmPage *p = new UninstallConfirmPage;
    p->setPackage(index.data().toString());
    p->setRequiredList(index.data(DebListModel::PackageReverseDependsListRole).toStringList());

    m_centralLayout->addWidget(p);
    m_centralLayout->setCurrentIndex(2);

    connect(p, &UninstallConfirmPage::accepted, this, &DebInstaller::onUninstallAccepted);
    connect(p, &UninstallConfirmPage::canceled, this, &DebInstaller::onUninstallCalceled);
}

void DebInstaller::onUninstallAccepted()
{
    SingleInstallPage *p = backToSinglePage();

    p->uninstallCurrentPackage();
}

void DebInstaller::onUninstallCalceled()
{
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

    m_dragflag = -1;
    titlebar()->setTitle(QString());
    m_fileListModel->reset();
    m_lastPage->deleteLater();
    m_centralLayout->setCurrentIndex(0);
}

void DebInstaller::removePackage(const int index)
{
    m_fileListModel->removePackage(index);
    refreshInstallPage();
}

void DebInstaller::refreshInstallPage()
{
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
        connect(singlePage, &SingleInstallPage::back, this, &DebInstaller::reset);
        connect(singlePage, &SingleInstallPage::requestUninstallConfirm, this, &DebInstaller::showUninstallConfirmPage);

        m_lastPage = singlePage;
        m_centralLayout->addWidget(singlePage);
        m_dragflag = 2;
    } else {
        // multiple packages install
        titlebar()->setTitle(tr("Bulk Install"));

        MultipleInstallPage *multiplePage = new MultipleInstallPage(m_fileListModel);
        multiplePage->setObjectName("MultipleInstallPage");

        connect(multiplePage, &MultipleInstallPage::back, this, &DebInstaller::reset);
        connect(multiplePage, &MultipleInstallPage::requestRemovePackage, this, &DebInstaller::removePackage);

        m_lastPage = multiplePage;
        m_centralLayout->addWidget(multiplePage);
        m_dragflag = 1;
    }

    // switch to new page.
    m_centralLayout->setCurrentIndex(1);
}

SingleInstallPage *DebInstaller::backToSinglePage()
{
    Q_ASSERT(m_centralLayout->count() == 3);
    DWidget *confirmPage = m_centralLayout->widget(2);
    m_centralLayout->removeWidget(confirmPage);
    confirmPage->deleteLater();

    SingleInstallPage *p = qobject_cast<SingleInstallPage *>(m_centralLayout->widget(1));
    Q_ASSERT(p);

    return p;
}

void DebInstaller::changeDragFlag()
{
    repaint();
    m_dragflag = 0;
}

void DebInstaller::showHiddenButton()
{
    if (m_dragflag == 2) {
        SingleInstallPage *singlePage = qobject_cast<SingleInstallPage *>(m_lastPage);
        singlePage->afterGetAutherFalse();
    } else if (m_dragflag == 1) {
        MultipleInstallPage *multiplePage = qobject_cast<MultipleInstallPage *>(m_lastPage);
        multiplePage->afterGetAutherFalse();
    }
}
