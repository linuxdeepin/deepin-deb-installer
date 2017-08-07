#include "multipleinstallpage.h"
#include "packagelistview.h"
#include "packageslistdelegate.h"
#include "deblistmodel.h"
#include "workerprogress.h"
#include "widgets/bluebutton.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QPropertyAnimation>
#include <QTimer>

MultipleInstallPage::MultipleInstallPage(DebListModel *model, QWidget *parent)
    : QWidget(parent),
      m_debListModel(model),
      m_appsView(new PackagesListView),
      m_infoArea(new QTextEdit),
      m_infoControlButton(new InfoControlButton(tr("Display details"), tr("Collapse"))),
      m_installProgress(new WorkerProgress),
      m_progressAnimation(new QPropertyAnimation(m_installProgress, "value", this)),
      m_installButton(new BlueButton),
      m_acceptButton(new BlueButton)
{
    m_appsView->setModel(model);
    m_appsView->setFixedHeight(236);
    m_appsView->setItemDelegate(new PackagesListDelegate);
    m_appsView->setStyleSheet("QListView {"
                              "border: 1px solid #eee;"
                              "margin: 36px 0 0 0;"
                              "}");

    m_installButton->setText(tr("Install"));
    m_acceptButton->setText(tr("OK"));
    m_acceptButton->setVisible(false);

    m_infoArea->setReadOnly(true);
    m_infoArea->setVisible(false);
    m_infoArea->setAcceptDrops(false);
    m_infoArea->setFixedHeight(200);
    m_infoArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_infoArea->setStyleSheet("QTextEdit {"
                              "color: #609dc9;"
                              "border: 1px solid #eee;"
                              "}");

    m_infoControlButton->setVisible(false);
    m_installProgress->setVisible(false);

    QHBoxLayout *btnsLayout = new QHBoxLayout;
    btnsLayout->addStretch();
    btnsLayout->addWidget(m_installButton);
    btnsLayout->addWidget(m_acceptButton);
    btnsLayout->addStretch();
    btnsLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_appsView);
    centralLayout->addWidget(m_infoControlButton);
    centralLayout->setAlignment(m_infoControlButton, Qt::AlignHCenter);
    centralLayout->addWidget(m_infoArea);
    centralLayout->addStretch();
    centralLayout->addWidget(m_installProgress);
    centralLayout->setAlignment(m_installProgress, Qt::AlignHCenter);
    centralLayout->addLayout(btnsLayout);
    centralLayout->setContentsMargins(20, 0, 20, 30);

    setLayout(centralLayout);

    connect(m_infoControlButton, &InfoControlButton::expand, this, &MultipleInstallPage::showInfo);
    connect(m_infoControlButton, &InfoControlButton::shrink, this, &MultipleInstallPage::hideInfo);
    connect(m_installButton, &QPushButton::clicked, m_debListModel, &DebListModel::installAll);
    connect(m_acceptButton, &QPushButton::clicked, qApp, &QApplication::quit);

//    connect(model, &DebListModel::workerStarted, this, &MultipleInstallPage::onWorkerStarted);
//    connect(model, &DebListModel::workerFinished, this, &MultipleInstallPage::onWorkerFinshed);
    connect(model, &DebListModel::workerProgressChanged, this, &MultipleInstallPage::onProgressChanged);
    connect(model, &DebListModel::appendOutputInfo, this, &MultipleInstallPage::onOutputAvailable);
}

//void MultipleInstallPage::onWorkerStarted()
//{
//    m_installButton->setVisible(false);

//    m_installProgress->setVisible(true);
//    m_infoControlButton->setVisible(true);
//}

void MultipleInstallPage::onWorkerFinshed()
{
    m_acceptButton->setVisible(true);
    m_installProgress->setVisible(false);
}

void MultipleInstallPage::onOutputAvailable(const QString &output)
{
    m_infoArea->append(output.trimmed());

    // change to install
    if (m_installButton->isVisible())
    {
        m_installButton->setVisible(false);

        m_installProgress->setVisible(true);
        m_infoControlButton->setVisible(true);
    }
}

void MultipleInstallPage::onProgressChanged(const int progress)
{
    m_progressAnimation->setStartValue(m_installProgress->value());
    m_progressAnimation->setEndValue(progress);
    m_progressAnimation->start();

    // finished
    if (progress == 100)
    {
        onOutputAvailable(QString());
        QTimer::singleShot(m_progressAnimation->duration(), this, &MultipleInstallPage::onWorkerFinshed);
    }
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
