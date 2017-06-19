#include "debinstaller.h"
#include "filechoosewidget.h"
#include "debpackage.h"
#include "singleinstallpage.h"

#include <QKeyEvent>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>

DebInstaller::DebInstaller(QWidget *parent)
    : QWidget(parent),

      m_centralLayout(new QStackedLayout),
      m_fileChooseWidget(new FileChooseWidget)
{
    m_centralLayout->addWidget(m_fileChooseWidget);

    setLayout(m_centralLayout);
    resize(800, 600);
    move(qApp->primaryScreen()->geometry().center() - rect().center());

    connect(m_fileChooseWidget, &FileChooseWidget::packagesSelected, this, &DebInstaller::onPackagesSelected);
}

DebInstaller::~DebInstaller()
{
    qDeleteAll(m_preparedPackages);
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
    Q_ASSERT(m_preparedPackages.isEmpty());

    for (const auto &package : packages)
    {
        DebPackage *p = new DebPackage(package);

        m_preparedPackages.append(p);
    }

    m_centralLayout->addWidget(new SingleInstallPage);
    m_centralLayout->setCurrentIndex(1);
}
