#include "multipleinstallpage.h"
#include "packagelistview.h"
#include "packageslistdelegate.h"
#include "deblistmodel.h"

#include <QVBoxLayout>
#include <QLabel>

MultipleInstallPage::MultipleInstallPage(DebListModel *model, QWidget *parent)
    : QWidget(parent),
      m_debListModel(model),
      m_appsView(new PackagesListView),
      m_installProgress(new QProgressBar),
      m_installButton(new QPushButton),
      m_acceptButton(new QPushButton)
{
    m_appsView->setModel(model);
    m_appsView->setItemDelegate(new PackagesListDelegate);
    m_appsView->setStyleSheet("QListView {"
                              "border: 1px solid #eee;"
                              "}");

    m_installButton->setText(tr("Install"));
    m_installButton->setFixedWidth(120);
    m_installButton->setFixedSize(120, 36);
    m_installButton->setStyleSheet("QPushButton {"
                                   "color: #2ca7f8;"
                                   "}");
    m_acceptButton->setText(tr("OK"));
    m_acceptButton->setFixedWidth(120);
    m_acceptButton->setFixedSize(120, 36);
    m_acceptButton->setVisible(false);

    m_installProgress->setMinimum(0);
    m_installProgress->setMaximum(100);
    m_installProgress->setValue(0);
    m_installProgress->setFixedHeight(8);
    m_installProgress->setFixedWidth(250);
    m_installProgress->setTextVisible(false);
//    m_installProgress->setVisible(false);

    QHBoxLayout *btnsLayout = new QHBoxLayout;
    btnsLayout->addStretch();
    btnsLayout->addWidget(m_installButton);
    btnsLayout->addWidget(m_acceptButton);
    btnsLayout->addStretch();
    btnsLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *topTips = new QLabel;
    topTips->setText(tr("Bulk Install"));
    topTips->setAlignment(Qt::AlignCenter);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(topTips);
    centralLayout->addWidget(m_appsView);
    centralLayout->addWidget(m_installProgress);
    centralLayout->setAlignment(m_installProgress, Qt::AlignHCenter);
    centralLayout->addLayout(btnsLayout);
    centralLayout->setSpacing(10);
    centralLayout->setContentsMargins(20, 0, 20, 30);

    setLayout(centralLayout);

    connect(m_installButton, &QPushButton::clicked, m_debListModel, &DebListModel::installAll);

    connect(model, &DebListModel::workerStarted, this, &MultipleInstallPage::onWorkerStarted);
    connect(model, &DebListModel::workerFinished, this, &MultipleInstallPage::onWorkerFinshed);
    connect(model, &DebListModel::workerProgressChanged, m_installProgress, &QProgressBar::setValue);
}

void MultipleInstallPage::onWorkerStarted()
{

}

void MultipleInstallPage::onWorkerFinshed()
{

}
