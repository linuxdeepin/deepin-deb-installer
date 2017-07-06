#include "multipleinstallpage.h"
#include "packagelistview.h"
#include "packageslistdelegate.h"
#include "deblistmodel.h"
#include "workerprogress.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QApplication>

MultipleInstallPage::MultipleInstallPage(DebListModel *model, QWidget *parent)
    : QWidget(parent),
      m_debListModel(model),
      m_appsView(new PackagesListView),
      m_infoArea(new QTextEdit),
      m_infoControlButton(new InfoControlButton),
      m_installProgress(new WorkerProgress),
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

    m_infoArea->setReadOnly(true);
    m_infoArea->setVisible(false);
    m_infoArea->setAcceptDrops(false);
    m_infoArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_infoArea->setStyleSheet("QTextEdit {"
                              "color: #2c77ab;"
                              "border: 1px solid #eee;"
                              "margin: 0 0 20px 0;"
                              "}");

    m_infoControlButton->setVisible(false);
    m_installProgress->setVisible(false);

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
    centralLayout->addWidget(m_infoControlButton);
    centralLayout->setAlignment(m_infoControlButton, Qt::AlignHCenter);
    centralLayout->addWidget(m_infoArea);
    centralLayout->addWidget(m_installProgress);
    centralLayout->setAlignment(m_installProgress, Qt::AlignHCenter);
    centralLayout->addLayout(btnsLayout);
    centralLayout->setSpacing(10);
    centralLayout->setContentsMargins(20, 0, 20, 30);

    setLayout(centralLayout);

    connect(m_infoControlButton, &InfoControlButton::expand, this, &MultipleInstallPage::showInfo);
    connect(m_infoControlButton, &InfoControlButton::shrink, this, &MultipleInstallPage::hideInfo);
    connect(m_installButton, &QPushButton::clicked, m_debListModel, &DebListModel::installAll);
    connect(m_acceptButton, &QPushButton::clicked, qApp, &QApplication::quit);

    connect(model, &DebListModel::workerStarted, this, &MultipleInstallPage::onWorkerStarted);
    connect(model, &DebListModel::workerFinished, this, &MultipleInstallPage::onWorkerFinshed);
    connect(model, &DebListModel::appendOutputInfo, m_infoArea, &QTextEdit::append);
    connect(model, &DebListModel::workerProgressChanged, m_installProgress, &QProgressBar::setValue);
}

void MultipleInstallPage::onWorkerStarted()
{
    m_installButton->setVisible(false);

    m_installProgress->setVisible(true);
    m_infoControlButton->setVisible(true);
}

void MultipleInstallPage::onWorkerFinshed()
{
    m_acceptButton->setVisible(true);
}

void MultipleInstallPage::showInfo()
{
    m_appsView->setVisible(false);
    m_infoArea->setVisible(true);
}

void MultipleInstallPage::hideInfo()
{
    m_appsView->setVisible(true);
    m_infoArea->setVisible(false);
}
