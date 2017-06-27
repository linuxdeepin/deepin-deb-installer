#include "singleinstallpage.h"
#include "deblistmodel.h"

#include <QVBoxLayout>
#include <QDebug>
#include <QTimer>

#include <QApt/DebFile>
#include <QApt/Transaction>

using QApt::DebFile;
using QApt::Transaction;


const QString holdTextInRect(const QFontMetrics &fm, const QString &text, const QRect &rect)
{
    const int textFlag = Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop;

    if (rect.contains(fm.boundingRect(rect, textFlag, text)))
        return text;

    QString str(text + "...");

    while (true)
    {
        if (str.size() < 4)
            break;

        QRect boundingRect = fm.boundingRect(rect, textFlag, str);
        if (rect.contains(boundingRect))
            break;

        str.remove(str.size() - 4, 1);
    }

    return str;
}

SingleInstallPage::SingleInstallPage(DebListModel *model, QWidget *parent)
    : QWidget(parent),

      m_packagesModel(model),

      m_packageIcon(new QLabel),
      m_packageName(new QLabel),
      m_packageVersion(new QLabel),
      m_packageDescription(new QLabel),
      m_installButton(new QPushButton),
      m_uninstallButton(new QPushButton),
      m_reinstallButton(new QPushButton)
{
    m_packageIcon->setText("icon");
    m_packageIcon->setFixedSize(64, 64);
    m_packageName->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    m_packageVersion->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_installButton->setText(tr("Install"));
    m_uninstallButton->setText(tr("Remove"));
    m_reinstallButton->setText(tr("Reinstall"));
    m_packageDescription->setWordWrap(true);
    m_packageDescription->setMaximumHeight(80);
    m_packageDescription->setFixedWidth(220);

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

    QHBoxLayout *btnsLayout = new QHBoxLayout;
    btnsLayout->addStretch();
    btnsLayout->addWidget(m_installButton);
    btnsLayout->addWidget(m_uninstallButton);
    btnsLayout->addWidget(m_reinstallButton);
    btnsLayout->addStretch();
    btnsLayout->setSpacing(30);
    btnsLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->addStretch();
    contentLayout->addLayout(itemLayout);
    contentLayout->addSpacing(30);
    contentLayout->addWidget(m_packageDescription);
    contentLayout->addStretch();
    contentLayout->addLayout(btnsLayout);
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
    connect(m_reinstallButton, &QPushButton::clicked, this, &SingleInstallPage::install);

    QTimer::singleShot(1, this, &SingleInstallPage::setPackageInfo);
}

void SingleInstallPage::install()
{
    m_packagesModel->installAll();
}

void SingleInstallPage::setPackageInfo()
{
    DebFile *package = m_packagesModel->preparedPackages().first();

    const QIcon icon = QIcon::fromTheme("application-vnd.debian.binary-package", QIcon::fromTheme("debian-swirl"));

    m_packageIcon->setPixmap(icon.pixmap(m_packageIcon->size()));
    m_packageName->setText(package->packageName());
    m_packageVersion->setText(package->version());

    // set package description
    const QRect boundingRect = QRect(0, 0, m_packageDescription->width(), m_packageDescription->maximumHeight());
    const QFontMetrics fm(m_packageDescription->font());
    m_packageDescription->setText(holdTextInRect(fm, package->longDescription(), boundingRect));

    // package install status
    const QModelIndex index = m_packagesModel->index(0);
    const int installStat = index.data(DebListModel::PackageVersionStatusRole).toInt();

    const bool installed = installStat != DebListModel::NotInstalled;
    m_installButton->setVisible(!installed);
    m_uninstallButton->setVisible(installed);
    m_reinstallButton->setVisible(installed);

    // package depends status
    const int dependsStat = index.data(DebListModel::PackageDependsStatusRole).toInt();
    qDebug() << dependsStat;
}
