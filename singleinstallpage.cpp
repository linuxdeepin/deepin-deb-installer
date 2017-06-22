#include "singleinstallpage.h"
#include "deblistmodel.h"

#include <QVBoxLayout>
#include <QDebug>
#include <QTimer>

#include <QApt/DebFile>
#include <QApt/Transaction>

using QApt::DebFile;
using QApt::Transaction;

SingleInstallPage::SingleInstallPage(DebListModel *model, QWidget *parent)
    : QWidget(parent),

      m_packagesModel(model),

      m_packageIcon(new QLabel),
      m_packageName(new QLabel),
      m_packageVersion(new QLabel),
      m_packageDescription(new QLabel),
      m_installButton(new QPushButton)
{
    m_packageIcon->setText("icon");
    m_packageIcon->setFixedSize(64, 64);
    m_packageName->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    m_packageVersion->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_installButton->setText(tr("Install"));
    m_installButton->setFixedWidth(120);
    m_packageDescription->setWordWrap(true);

    QLabel *packageName = new QLabel;
    packageName->setText(tr("Package: "));
    packageName->setAlignment(Qt::AlignBottom | Qt::AlignLeft);

    QLabel *packageVersion = new QLabel;
    packageVersion->setText(tr("Version: "));
    packageVersion->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    QGridLayout *itemInfoLayout = new QGridLayout;
    itemInfoLayout->addWidget(packageName, 0, 0);
    itemInfoLayout->addWidget(m_packageName, 0, 1);
    itemInfoLayout->addWidget(packageVersion, 1, 0);
    itemInfoLayout->addWidget(m_packageVersion, 1, 1);
    itemInfoLayout->setSpacing(0);
    itemInfoLayout->setVerticalSpacing(10);
    itemInfoLayout->setMargin(0);

    QHBoxLayout *itemLayout = new QHBoxLayout;
    itemLayout->addStretch();
    itemLayout->addWidget(m_packageIcon);
    itemLayout->addLayout(itemInfoLayout);
    itemLayout->addStretch();
    itemLayout->setSpacing(10);
    itemLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->addStretch();
    contentLayout->addLayout(itemLayout);
    contentLayout->addSpacing(30);
    contentLayout->addWidget(m_packageDescription);
    contentLayout->addStretch();
    contentLayout->addWidget(m_installButton);
    contentLayout->setAlignment(m_installButton, Qt::AlignHCenter);
    contentLayout->setSpacing(0);
    contentLayout->setMargin(0);

    QHBoxLayout *centralLayout = new QHBoxLayout;
    centralLayout->addStretch();
    centralLayout->addLayout(contentLayout);
    centralLayout->addStretch();
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(100, 0, 100, 20);

    setLayout(centralLayout);

    connect(m_installButton, &QPushButton::clicked, this, &SingleInstallPage::install);
    connect(m_packagesModel, &DebListModel::appendOutputInfo, this, &SingleInstallPage::appendOutputInfo);

    QTimer::singleShot(1, this, &SingleInstallPage::setPackageInfo);
}

void SingleInstallPage::install()
{
    m_packagesModel->installAll();
//    Transaction *transaction = m_aptBackend->installFile(*m_debFile);
//    qDebug() << transaction->filePath() << transaction->status();

//    connect(transaction, &Transaction::statusChanged, this, &SingleInstallPage::onTransactionStatusChanged);
//    connect(transaction, &Transaction::statusDetailsChanged, this, [this](const QString &str) { qDebug() << str; });

    //    transaction->run();
}

void SingleInstallPage::appendOutputInfo(const QString &output)
{
    qDebug() << output;
}

void SingleInstallPage::setPackageInfo()
{
    DebFile *package = m_packagesModel->preparedPackages().first();

    const QIcon icon = QIcon::fromTheme("application-vnd.debian.binary-package", QIcon::fromTheme("debian-swirl"));

    m_packageIcon->setPixmap(icon.pixmap(m_packageIcon->size()));
    m_packageName->setText(package->packageName());
    m_packageVersion->setText(package->version());
    m_packageDescription->setText(package->longDescription());
}
