#include "multipleinstallpage.h"
#include "packagelistview.h"
#include "packageslistdelegate.h"
#include "deblistmodel.h"

#include <QVBoxLayout>

MultipleInstallPage::MultipleInstallPage(DebListModel *model, QWidget *parent)
    : QWidget(parent),
      m_debListModel(model),
      m_appsView(new PackagesListView),
      m_installButton(new QPushButton),
      m_acceptButton(new QPushButton)
{
    m_appsView->setModel(model);
    m_appsView->setItemDelegate(new PackagesListDelegate);
    m_installButton->setText(tr("Install"));
    m_installButton->setFixedWidth(120);
    m_acceptButton->setText(tr("OK"));
    m_acceptButton->setFixedWidth(120);
    m_acceptButton->setVisible(false);

    QHBoxLayout *btnsLayout = new QHBoxLayout;
    btnsLayout->addWidget(m_installButton);
    btnsLayout->addWidget(m_acceptButton);
    btnsLayout->setContentsMargins(0, 0, 0, 15);

    QVBoxLayout *centralLayout = new QVBoxLayout;

    centralLayout->addWidget(m_appsView);
    centralLayout->addLayout(btnsLayout);
    centralLayout->setAlignment(m_installButton, Qt::AlignHCenter);
    centralLayout->setSpacing(0);
    centralLayout->setMargin(0);

    setLayout(centralLayout);

    connect(m_installButton, &QPushButton::clicked, m_debListModel, &DebListModel::installAll);

    connect(model, &DebListModel::workerStarted, this, &MultipleInstallPage::onWorkerStarted);
    connect(model, &DebListModel::workerFinished, this, &MultipleInstallPage::onWorkerFinshed);
}

void MultipleInstallPage::onWorkerStarted()
{

}

void MultipleInstallPage::onWorkerFinshed()
{

}
