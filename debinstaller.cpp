#include "debinstaller.h"
#include "filechoosewidget.h"

#include <QKeyEvent>
#include <QCoreApplication>

DebInstaller::DebInstaller(QWidget *parent)
    : QWidget(parent),

      m_centralLayout(new QStackedLayout),
      m_fileChooseWidget(new FileChooseWidget)
{
    m_centralLayout->addWidget(m_fileChooseWidget);

    setLayout(m_centralLayout);
    resize(800, 600);
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
