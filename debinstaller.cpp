#include "debinstaller.h"
#include "filechoosewidget.h"
#include "debpackage.h"
#include "singleinstallpage.h"
#include "debinstallworker.h"
#include "deblistmodel.h"

#include <QKeyEvent>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>

DebInstaller::DebInstaller(QWidget *parent)
    : QWidget(parent),

      m_centralLayout(new QStackedLayout),
      m_fileChooseWidget(new FileChooseWidget),
      m_installWorker(new DebInstallWorker(this)),
      m_fileListModel(new DebListModel(this))
{
    m_centralLayout->addWidget(m_fileChooseWidget);

    setLayout(m_centralLayout);
    resize(800, 600);
    move(qApp->primaryScreen()->geometry().center() - rect().center());

    connect(m_fileChooseWidget, &FileChooseWidget::packagesSelected, this, &DebInstaller::onPackagesSelected);
}

DebInstaller::~DebInstaller()
{
}

void DebInstaller::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Escape:        qApp->quit();       break;
    default:;
    }
}

void DebInstaller::onPackagesSelected(const QStringList &packages)
{
    Q_ASSERT(m_fileListModel->preparedPackages().isEmpty());

    for (const auto &package : packages)
    {
        DebPackage *p = new DebPackage(package);

        m_fileListModel->appendPackage(p);
    }

    m_centralLayout->addWidget(new SingleInstallPage);
    m_centralLayout->setCurrentIndex(1);
}
