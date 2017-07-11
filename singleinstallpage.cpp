#include "singleinstallpage.h"
#include "deblistmodel.h"
#include "workerprogress.h"

#include <QVBoxLayout>
#include <QDebug>
#include <QTimer>
#include <QApplication>

#include <QApt/DebFile>
#include <QApt/Transaction>

using QApt::DebFile;
using QApt::Transaction;

DWIDGET_USE_NAMESPACE

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

      m_workerStarted(false),
      m_packagesModel(model),

      m_itemInfoWidget(new QWidget),
      m_packageIcon(new QLabel),
      m_packageName(new QLabel),
      m_packageVersion(new QLabel),
      m_packageDescription(new QLabel),
      m_tipsLabel(new QLabel),
      m_progress(new WorkerProgress),
      m_workerInfomation(new QTextEdit),
      m_infoControlButton(new InfoControlButton),
      m_installButton(new QPushButton),
      m_uninstallButton(new QPushButton),
      m_reinstallButton(new QPushButton),
      m_confirmButton(new QPushButton)
{
    m_packageIcon->setText("icon");
    m_packageIcon->setFixedSize(64, 64);
    m_packageName->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    m_packageVersion->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_tipsLabel->setAlignment(Qt::AlignCenter);
    m_tipsLabel->setStyleSheet("QLabel {"
                               "color: #ff5a5a;"
                               "}");

    m_progress->setVisible(false);
    m_infoControlButton->setVisible(false);

    m_workerInfomation->setReadOnly(true);
    m_workerInfomation->setVisible(false);
    m_workerInfomation->setAcceptDrops(false);
    m_workerInfomation->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_workerInfomation->setStyleSheet("QTextEdit {"
                                      "color: #609dc9;"
                                      "border: 1px solid #eee;"
                                      "margin: 10px 0 20px 0;"
                                      "}");

    m_installButton->setText(tr("Install"));
    m_installButton->setFixedSize(120, 36);
    m_installButton->setVisible(false);
    m_uninstallButton->setText(tr("Remove"));
    m_uninstallButton->setFixedSize(120, 36);
    m_uninstallButton->setVisible(false);
    m_uninstallButton->setStyleSheet("QPushButton {"
                                     "color: #303030;"
                                     "}");
    m_reinstallButton->setText(tr("Reinstall"));
    m_reinstallButton->setFixedSize(120, 36);
    m_reinstallButton->setVisible(false);
    m_reinstallButton->setStyleSheet("QPushButton {"
                                     "color: #303030;"
                                     "}");
    m_confirmButton->setText(tr("OK"));
    m_confirmButton->setFixedSize(120, 36);
    m_confirmButton->setVisible(false);
    m_confirmButton->setStyleSheet("QPushButton {"
                                   "color: #2ca7f8;"
                                   "}"
                                   "QPushButton:hover {"
                                   "color: white;"
                                   "}");
    m_packageDescription->setWordWrap(true);
    m_packageDescription->setMaximumHeight(80);
    m_packageDescription->setFixedWidth(320);

    QLabel *packageName = new QLabel;
    packageName->setText(tr("Package: "));
    packageName->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    packageName->setStyleSheet("QLabel {"
                               "color: #797979;"
                               "}");

    QLabel *packageVersion = new QLabel;
    packageVersion->setText(tr("Version: "));
    packageVersion->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    packageVersion->setStyleSheet("QLabel {"
                                  "color: #797979;"
                                  "}");

    QGridLayout *itemInfoLayout = new QGridLayout;
    itemInfoLayout->addWidget(packageName, 0, 0);
    itemInfoLayout->addWidget(m_packageName, 0, 1);
    itemInfoLayout->addWidget(packageVersion, 1, 0);
    itemInfoLayout->addWidget(m_packageVersion, 1, 1);
    itemInfoLayout->setSpacing(0);
    itemInfoLayout->setVerticalSpacing(10);
    itemInfoLayout->setMargin(0);

    QHBoxLayout *itemBlockLayout = new QHBoxLayout;
    itemBlockLayout->addStretch();
    itemBlockLayout->addWidget(m_packageIcon);
    itemBlockLayout->addLayout(itemInfoLayout);
    itemBlockLayout->addStretch();
    itemBlockLayout->setSpacing(10);
    itemBlockLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *btnsLayout = new QHBoxLayout;
    btnsLayout->addStretch();
    btnsLayout->addWidget(m_installButton);
    btnsLayout->addWidget(m_uninstallButton);
    btnsLayout->addWidget(m_reinstallButton);
    btnsLayout->addWidget(m_confirmButton);
    btnsLayout->addStretch();
    btnsLayout->setSpacing(30);
    btnsLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *itemLayout = new QVBoxLayout;
    itemLayout->addSpacing(60);
    itemLayout->addLayout(itemBlockLayout);
    itemLayout->addSpacing(25);
    itemLayout->addWidget(m_packageDescription);
    itemLayout->addStretch();
    itemLayout->setMargin(0);
    itemLayout->setSpacing(0);

    m_itemInfoWidget->setLayout(itemLayout);
    m_itemInfoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_itemInfoWidget->setVisible(false);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_itemInfoWidget);
    centralLayout->setAlignment(m_itemInfoWidget, Qt::AlignHCenter);
    centralLayout->addWidget(m_infoControlButton);
    centralLayout->setAlignment(m_infoControlButton, Qt::AlignHCenter);
    centralLayout->addWidget(m_workerInfomation);
    centralLayout->addWidget(m_tipsLabel);
    centralLayout->addWidget(m_progress);
    centralLayout->setAlignment(m_progress, Qt::AlignHCenter);
    centralLayout->addSpacing(10);
    centralLayout->addLayout(btnsLayout);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(20, 0, 20, 30);

    setLayout(centralLayout);

    connect(m_infoControlButton, &InfoControlButton::expand, this, &SingleInstallPage::showInfomation);
    connect(m_infoControlButton, &InfoControlButton::shrink, this, &SingleInstallPage::hideInfomation);
    connect(m_installButton, &QPushButton::clicked, this, &SingleInstallPage::install);
    connect(m_reinstallButton, &QPushButton::clicked, this, &SingleInstallPage::install);
    connect(m_uninstallButton, &QPushButton::clicked, this, &SingleInstallPage::uninstallCurrentPackage);
    connect(m_confirmButton, &QPushButton::clicked, qApp, &QApplication::quit);

    connect(model, &DebListModel::appendOutputInfo, this, &SingleInstallPage::onOutputAvailable);
    connect(model, &DebListModel::workerFinished, this, &SingleInstallPage::onWorkerFinished);
    connect(model, &DebListModel::transactionProgressChanged, this, &SingleInstallPage::onWorkerProgressChanged);

    if (m_packagesModel->isReady())
        setPackageInfo();
    else
        QTimer::singleShot(120, this, &SingleInstallPage::setPackageInfo);
}

void SingleInstallPage::install()
{
    m_packagesModel->installAll();
}

void SingleInstallPage::uninstallCurrentPackage()
{
    m_packagesModel->uninstallPackage(0);
}

void SingleInstallPage::showInfomation()
{
    m_workerInfomation->setVisible(true);
    m_itemInfoWidget->setVisible(false);
}

void SingleInstallPage::hideInfomation()
{
    m_workerInfomation->setVisible(false);
    m_itemInfoWidget->setVisible(true);
}

void SingleInstallPage::showInfo()
{
    m_infoControlButton->setVisible(true);
    m_progress->setVisible(true);
    m_progress->setValue(0);
    m_tipsLabel->clear();

    m_installButton->setVisible(false);
    m_reinstallButton->setVisible(false);
    m_uninstallButton->setVisible(false);
    m_confirmButton->setVisible(false);
}

void SingleInstallPage::onOutputAvailable(const QString &output)
{
    m_workerInfomation->append(output.trimmed());
}

void SingleInstallPage::onWorkerFinished()
{
    m_progress->setVisible(false);
    m_confirmButton->setVisible(true);
    m_confirmButton->setFocus();

    // TODO: success or fail
    const QModelIndex index = m_packagesModel->first();
    const int stat = index.data(DebListModel::PackageOperateStatusRole).toInt();

    if (stat == DebListModel::Success)
    {
        m_tipsLabel->setText(tr("Installed successfully"));
        m_tipsLabel->setStyleSheet("QLabel {"
                                   "color: #47790c;"
                                   "}");
    } else if (stat == DebListModel::Failed) {
        m_tipsLabel->setText(tr("Install Failed"));
    } else {
        Q_UNREACHABLE();
    }
}

void SingleInstallPage::onWorkerProgressChanged(const int progress)
{
    m_progress->setValue(progress);

    if (!m_workerStarted)
    {
        m_workerStarted = true;
        showInfo();
    }
}

void SingleInstallPage::setPackageInfo()
{
    qApp->processEvents();

    DebFile *package = m_packagesModel->preparedPackages().first();

    const QIcon icon = QIcon::fromTheme("application-vnd.debian.binary-package", QIcon::fromTheme("debian-swirl"));

    m_itemInfoWidget->setVisible(true);
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
    const bool installedSameVersion = installStat == DebListModel::InstalledSameVersion;
    m_installButton->setVisible(!installed || !installedSameVersion);
    m_uninstallButton->setVisible(installedSameVersion);
    m_reinstallButton->setVisible(installedSameVersion);
    m_confirmButton->setVisible(false);

    if (installed)
    {
        if (!installedSameVersion)
            m_tipsLabel->setText(tr("Other version installed"));
        else
            m_tipsLabel->setText(tr("Same version installed"));

        return;
    }

    // package depends status
    const int dependsStat = index.data(DebListModel::PackageDependsStatusRole).toInt();
    if (dependsStat == DebListModel::DependsBreak)
    {
        m_tipsLabel->setText(tr("Broken Dependencies"));
        m_installButton->setVisible(false);
        m_reinstallButton->setVisible(false);
        m_confirmButton->setVisible(true);
    }
}
