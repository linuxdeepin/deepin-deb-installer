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
#include "filechoosewidget.h"
#include "singleinstallpage.h"
#include "multipleinstallpage.h"
#include "deblistmodel.h"
#include "uninstallconfirmpage.h"

#include <QKeyEvent>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>
#include <QProcess>
#include <QAction>
#include <QDragEnterEvent>
#include <QFileInfo>
#include <QMimeData>
#include <QDir>

#include <QApt/DebFile>

#include <DTitlebar>
#include <DRecentManager>
#include <DThemeManager>

using QApt::DebFile;

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

#define THEME_DARK "dark"
#define THEME_LIGHT "light"

DebInstaller::DebInstaller(QWidget *parent)
    : DMainWindow(parent),

      m_fileListModel(new DebListModel(this)),
      m_fileChooseWidget(new FileChooseWidget),
      m_centralLayout(new QStackedLayout),
      m_qsettings(new QSettings(this)),
      m_tbMenu(new QMenu(this)),
      m_darkThemeAction(new QAction(tr("Dark Theme"), this))
{
    m_fileChooseWidget->setObjectName("FileChooseWidget");

    m_centralLayout->addWidget(m_fileChooseWidget);
    m_centralLayout->setContentsMargins(0, 0, 0, 0);
    m_centralLayout->setSpacing(0);

    QWidget *wrapWidget = new QWidget;
    wrapWidget->setLayout(m_centralLayout);

    DTitlebar *tb = titlebar();
    tb->setIcon(QIcon::fromTheme("deepin-deb-installer"));
    tb->setTitle(QString());
#if DTK_VERSION >= 0x02000600
    tb->setBackgroundTransparent(true);
#endif
    tb->setMenu(m_tbMenu);
    m_tbMenu->addAction(m_darkThemeAction);
    m_tbMenu->addSeparator();

    m_darkThemeAction->setCheckable(true);

    setCentralWidget(wrapWidget);
    setAcceptDrops(true);
    setFixedSize(480, 380);
    setWindowTitle(tr("Deepin Package Manager"));
    setWindowIcon(QIcon::fromTheme("deepin-deb-installer"));
    move(qApp->primaryScreen()->geometry().center() - geometry().center());

    connect(m_fileChooseWidget, &FileChooseWidget::packagesSelected, this, &DebInstaller::onPackagesSelected);
    connect(m_fileListModel, &DebListModel::lockForAuth, this, &DebInstaller::onAuthing);
    connect(m_fileListModel, &DebListModel::appendOutputInfo, this, [=](const QString &output) { qDebug() << output.trimmed(); });
    connect(m_darkThemeAction, &QAction::toggled, this, &DebInstaller::toggleDarkTheme);

    reloadTheme();
}

DebInstaller::~DebInstaller()
{
}

void DebInstaller::toggleDarkTheme(bool checked)
{
    m_qsettings->setValue("theme", checked ? THEME_DARK : THEME_LIGHT);
    reloadTheme();
}

void DebInstaller::reloadTheme()
{
    QString theme = m_qsettings->value("theme").toString();
    if (theme.isEmpty()) {
        theme = THEME_LIGHT;
        m_qsettings->setValue("theme", THEME_LIGHT);
    }

    m_darkThemeAction->setChecked(theme == THEME_DARK);

    QFile themeFile(theme == THEME_DARK ? ":/theme/dark/dark.qss" : ":/theme/light/light.qss");
    if (!themeFile.open(QFile::ReadOnly)) {
        qDebug() << "theme file not find!" << themeFile.fileName();
    }

    setStyleSheet(themeFile.readAll());

    DThemeManager::instance()->setTheme(theme);
}

void DebInstaller::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
#ifdef QT_DEBUG
    case Qt::Key_Escape:        qApp->quit();       break;
#endif
    default:;
    }
}

void DebInstaller::dragEnterEvent(QDragEnterEvent *e)
{
    auto * const mime = e->mimeData();
    if (!mime->hasUrls())
        return e->ignore();

    for (const auto &item : mime->urls())
    {
        const QFileInfo info(item.path());
        if (info.isDir())
            return e->accept();
        if (info.isFile() && info.suffix() == "deb")
            return e->accept();
    }

    e->ignore();
}

void DebInstaller::dropEvent(QDropEvent *e)
{
    auto * const mime = e->mimeData();
    if (!mime->hasUrls())
        return e->ignore();

    e->accept();

    // find .deb files
    QStringList file_list;
    for (const auto &url : mime->urls())
    {
        if (!url.isLocalFile())
            continue;
        const QString local_path = url.toLocalFile();
        const QFileInfo info(local_path);

        if (info.isFile() && info.suffix() == "deb")
            file_list << local_path;
        else if (info.isDir())
        {
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
    for (const auto &package : packages)
    {
        DebFile *p = new DebFile(package);
        if (!p->isValid())
        {
            qWarning() << "package invalid: " << package;

            delete p;
            continue;
        }

        DRecentData data;
        data.appName = "Deepin Deb Installer";
        data.appExec = "deepin-deb-installer";
        DRecentManager::addItem(package, data);

        m_fileListModel->appendPackage(p);
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
    if (!m_lastPage.isNull())
        m_lastPage->deleteLater();

    const int packageCount = m_fileListModel->preparedPackages().size();
    // no packages found
    if (packageCount == 0)
        return;

    if (packageCount == 1)
    {
        // single package install
        titlebar()->setTitle(QString());

        SingleInstallPage *singlePage = new SingleInstallPage(m_fileListModel);
        singlePage->setObjectName("SingleInstallPage");
        connect(singlePage, &SingleInstallPage::back, this, &DebInstaller::reset);
        connect(singlePage, &SingleInstallPage::requestUninstallConfirm, this, &DebInstaller::showUninstallConfirmPage);

        m_lastPage = singlePage;
        m_centralLayout->addWidget(singlePage);
    } else {
        // multiple packages install
        titlebar()->setTitle(tr("Bulk Install"));

        MultipleInstallPage *multiplePage = new MultipleInstallPage(m_fileListModel);
        multiplePage->setObjectName("MultipleInstallPage");

        connect(multiplePage, &MultipleInstallPage::back, this, &DebInstaller::reset);
        connect(multiplePage, &MultipleInstallPage::requestRemovePackage, this, &DebInstaller::removePackage);

        m_lastPage = multiplePage;
        m_centralLayout->addWidget(multiplePage);
    }

    // switch to new page.
    m_centralLayout->setCurrentIndex(1);
}

SingleInstallPage *DebInstaller::backToSinglePage()
{
    Q_ASSERT(m_centralLayout->count() == 3);
    QWidget *confirmPage = m_centralLayout->widget(2);
    m_centralLayout->removeWidget(confirmPage);
    confirmPage->deleteLater();

    SingleInstallPage *p = qobject_cast<SingleInstallPage *>(m_centralLayout->widget(1));
    Q_ASSERT(p);

    return p;
}
