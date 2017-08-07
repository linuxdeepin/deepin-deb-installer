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

    DTitlebar *tb = titlebar();
    tb->setIcon(QIcon::fromTheme("deepin-deb-installer").pixmap(24, 24));
    tb->setTitle(QString());
    tb->setWindowFlags(tb->windowFlags() & ~Qt::WindowMaximizeButtonHint);

    setCentralWidget(wrapWidget);
    setFixedSize(480, 380);
    setWindowTitle(tr("Deepin Package Manager"));
    setWindowIcon(QIcon::fromTheme("deepin-deb-installer"));
    move(qApp->primaryScreen()->geometry().center() - geometry().center());

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
    default:;
    }
}

void DebInstaller::onPackagesSelected(const QStringList &packages)
{
    Q_ASSERT(m_fileListModel->preparedPackages().isEmpty());

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

    const int packageCount = m_fileListModel->preparedPackages().size();
    // no packages found
    if (packageCount == 0)
        return;

    if (packageCount == 1)
    {
        // single package install
        SingleInstallPage *singlePage = new SingleInstallPage(m_fileListModel);

        connect(singlePage, &SingleInstallPage::requestUninstallConfirm, this, &DebInstaller::showUninstallConfirmPage);

        m_centralLayout->addWidget(singlePage);
    } else {
        // multiple packages install
        titlebar()->setTitle(tr("Bulk Install"));

        MultipleInstallPage *multiplePage = new MultipleInstallPage(m_fileListModel);

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
