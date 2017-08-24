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

using QApt::DebFile;

DWIDGET_USE_NAMESPACE

DebInstaller::DebInstaller(QWidget *parent)
    : DMainWindow(parent),

      m_fileListModel(new DebListModel(this)),

      m_centralLayout(new QStackedLayout),
      m_fileChooseWidget(new FileChooseWidget)
{
    m_centralLayout->addWidget(m_fileChooseWidget);
    m_centralLayout->setContentsMargins(0, 0, 0, 0);
    m_centralLayout->setSpacing(0);

    QWidget *wrapWidget = new QWidget;
    wrapWidget->setLayout(m_centralLayout);
//    wrapWidget->setStyleSheet("background-color: red;");

    QAction *helpAction = new QAction(tr("Help"), this);

    QMenu *titleMenu = new QMenu;
    titleMenu->addAction(helpAction);

    DTitlebar *tb = titlebar();
    tb->setIcon(QIcon::fromTheme("deepin-deb-installer").pixmap(24, 24));
    tb->setTitle(QString());
    tb->setWindowFlags(tb->windowFlags() & ~Qt::WindowMaximizeButtonHint);
    tb->setMenu(titleMenu);

    setCentralWidget(wrapWidget);
    setAcceptDrops(true);
    setFixedSize(480, 380);
    setWindowTitle(tr("Deepin Package Manager"));
    setWindowIcon(QIcon::fromTheme("deepin-deb-installer"));
    move(qApp->primaryScreen()->geometry().center() - geometry().center());

    connect(helpAction, &QAction::triggered, this, &DebInstaller::showHelp);
    connect(m_fileChooseWidget, &FileChooseWidget::packagesSelected, this, &DebInstaller::onPackagesSelected);
    connect(m_fileListModel, &DebListModel::lockForAuth, this, &DebInstaller::onAuthing);
    connect(m_fileListModel, &DebListModel::appendOutputInfo, this, [=](const QString &output) { qDebug() << output.trimmed(); });
}

DebInstaller::~DebInstaller()
{
}

void DebInstaller::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
#ifdef QT_DEBUG
    case Qt::Key_Escape:        qApp->quit();       break;
#endif
    case Qt::Key_F1:            showHelp();         break;
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

        m_fileListModel->appendPackage(p);
    }

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
        SingleInstallPage *singlePage = new SingleInstallPage(m_fileListModel);
        m_lastPage = singlePage;

        connect(singlePage, &SingleInstallPage::requestUninstallConfirm, this, &DebInstaller::showUninstallConfirmPage);

        m_centralLayout->addWidget(singlePage);
    } else {
        // multiple packages install
        titlebar()->setTitle(tr("Bulk Install"));

        MultipleInstallPage *multiplePage = new MultipleInstallPage(m_fileListModel);
        m_lastPage = multiplePage;

        m_centralLayout->addWidget(multiplePage);
    }

    // switch to new page.
    m_centralLayout->setCurrentIndex(1);
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

void DebInstaller::showHelp()
{
    QProcess::startDetached("dman", QStringList() << "deepin-package-manager");
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
