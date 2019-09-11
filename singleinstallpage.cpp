/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "singleinstallpage.h"
#include "deblistmodel.h"
#include "widgets/bluebutton.h"
#include "widgets/graybutton.h"
#include "workerprogress.h"

#include <QApplication>
#include <QDebug>
#include <QRegularExpression>
#include <QTextLayout>
#include <QTimer>
#include <QVBoxLayout>

#include <QApt/DebFile>
#include <QApt/Transaction>

using QApt::DebFile;
using QApt::Transaction;

DWIDGET_USE_NAMESPACE

const QString holdTextInRect(const QFont &font, QString text, const QSize &size)
{
    QFontMetrics fm(font);
    QTextLayout layout(text);

    layout.setFont(font);

    QStringList lines;
    QTextOption &text_option = *const_cast<QTextOption *>(&layout.textOption());

    text_option.setWrapMode(QTextOption::WordWrap);
    text_option.setAlignment(Qt::AlignTop | Qt::AlignLeft);

    layout.beginLayout();

    QTextLine line = layout.createLine();
    int height = 0;
    int lineHeight = fm.height();

    while (line.isValid()) {
        height += lineHeight;

        if (height + lineHeight > size.height()) {
            const QString &end_str = fm.elidedText(text.mid(line.textStart()), Qt::ElideRight, size.width());

            layout.endLayout();
            layout.setText(end_str);

            text_option.setWrapMode(QTextOption::NoWrap);
            layout.beginLayout();
            line = layout.createLine();
            line.setLineWidth(size.width() - 1);
            text = end_str;
        } else {
            line.setLineWidth(size.width());
        }

        lines.append(text.mid(line.textStart(), line.textLength()));

        if (height + lineHeight > size.height()) break;

        line = layout.createLine();
    }

    layout.endLayout();

    return lines.join("");
}

SingleInstallPage::SingleInstallPage(DebListModel *model, DWidget *parent)
    : DWidget(parent)
    , m_operate(Install)
    , m_workerStarted(false)
    , m_packagesModel(model)
    , m_itemInfoWidget(new DWidget)
    , m_packageIcon(new DLabel)
    , m_packageName(new DLabel)
    , m_packageVersion(new DLabel)
    , m_packageDescription(new DLabel)
    , m_tipsLabel(new DLabel)
    , m_progress(new WorkerProgress)
    , m_workerInfomation(new DTextEdit)
    , m_strengthWidget(new DWidget)
    , m_infoControlButton(new InfoControlButton(tr("Display install details"), tr("Collapse")))
    , m_installButton(new DPushButton)
    , m_uninstallButton(new DPushButton)
    , m_reinstallButton(new DPushButton)
    , m_confirmButton(new DPushButton)
    , m_backButton(new DPushButton)
    , m_doneButton(new DPushButton) {

    const QFont font_const = this->font();
    QFont font_use = font_const;
    m_packageName->setObjectName("PackageName");
    m_packageVersion->setObjectName("PackageVersion");
    m_infoControlButton->setObjectName("InfoControlButton");
    m_workerInfomation->setObjectName("WorkerInformation");
    m_packageDescription->setObjectName("PackageDescription");

    m_packageIcon->setText("icon");
    m_packageIcon->setFixedSize(80, 90);
    font_use.setPixelSize(14);
    m_packageName->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    m_packageName->setFont(font_use);
    m_packageVersion->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_packageVersion->setFont(font_use);
    m_tipsLabel->setAlignment(Qt::AlignCenter);

    m_progress->setVisible(false);
    m_infoControlButton->setVisible(false);

    font_use.setPixelSize(11);
    m_workerInfomation->setFont(font_use);
    m_workerInfomation->setTextColor(QColor("#609DC8"));
    m_workerInfomation->setReadOnly(true);
    m_workerInfomation->setVisible(false);
    m_workerInfomation->setAcceptDrops(false);
    m_workerInfomation->setFixedHeight(210);
    m_workerInfomation->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_installButton->setText(tr("Install"));
    m_installButton->setVisible(false);
    m_uninstallButton->setText(tr("Remove"));
    m_uninstallButton->setVisible(false);
    m_reinstallButton->setText(tr("Reinstall"));
    m_reinstallButton->setVisible(false);
    m_confirmButton->setText(tr("OK"));
    m_confirmButton->setVisible(false);
    m_backButton->setText(tr("Back"));
    m_backButton->setVisible(false);
    m_doneButton->setText(tr("Done"));
    m_doneButton->setVisible(false);
    m_packageDescription->setWordWrap(true);

    m_installButton->setFixedSize(120,36);
    m_uninstallButton->setFixedSize(120,36);
    m_reinstallButton->setFixedSize(120,36);
    m_confirmButton->setFixedSize(120,36);
    m_backButton->setFixedSize(120,36);
    m_doneButton->setFixedSize(120,36);

    m_packageDescription->setFixedHeight(80);
    font_use.setPixelSize(12);
    m_packageDescription->setFont(font_use);
    m_packageDescription->setFixedWidth(320);
    m_packageDescription->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    font_use.setPixelSize(12);
    m_packageDescription->setFont(font_use);

    DLabel *packageName = new DLabel;
    packageName->setText(tr("Name: "));
    font_use.setPixelSize(14);
    packageName->setFont(font_use);
    packageName->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    packageName->setObjectName("PackageNameTitle");

    DLabel *packageVersion = new DLabel;
    packageVersion->setText(tr("Version: "));
    font_use.setPixelSize(14);
    packageVersion->setFont(font_use);
    packageVersion->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    packageVersion->setObjectName("PackageVersionTitle");

    QGridLayout *itemInfoLayout = new QGridLayout;
    itemInfoLayout->addWidget(packageName, 0, 0);
    itemInfoLayout->addWidget(m_packageName, 0, 1);
    itemInfoLayout->addWidget(packageVersion, 1, 0);
    itemInfoLayout->addWidget(m_packageVersion, 1, 1);
    itemInfoLayout->setSpacing(0);
    itemInfoLayout->setVerticalSpacing(0);
    itemInfoLayout->setMargin(0);

    QHBoxLayout *itemBlockLayout = new QHBoxLayout;
    itemBlockLayout->setSpacing(112);
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
    btnsLayout->addWidget(m_backButton);
    btnsLayout->addWidget(m_confirmButton);
    btnsLayout->addWidget(m_doneButton);
    btnsLayout->addStretch();
    btnsLayout->setSpacing(0);
    btnsLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *itemLayout = new QVBoxLayout;
    itemLayout->addSpacing(45);
    itemLayout->addLayout(itemBlockLayout);
    itemLayout->addSpacing(20);
    itemLayout->addWidget(m_packageDescription);
    itemLayout->addStretch();
    itemLayout->setMargin(0);
    itemLayout->setSpacing(0);

    m_itemInfoWidget->setLayout(itemLayout);
    m_itemInfoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_itemInfoWidget->setVisible(false);

    m_strengthWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_strengthWidget->setVisible(false);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_itemInfoWidget);
    centralLayout->setAlignment(m_itemInfoWidget, Qt::AlignHCenter);
    centralLayout->addWidget(m_infoControlButton);
    centralLayout->setAlignment(m_infoControlButton, Qt::AlignHCenter);
    centralLayout->addWidget(m_workerInfomation);
    centralLayout->addWidget(m_strengthWidget);
    centralLayout->addWidget(m_tipsLabel);
    centralLayout->addWidget(m_progress);
    centralLayout->setAlignment(m_progress, Qt::AlignHCenter);
    centralLayout->addSpacing(8);
    centralLayout->addLayout(btnsLayout);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(20, 0, 20, 30);

    setLayout(centralLayout);

    connect(m_infoControlButton, &InfoControlButton::expand, this, &SingleInstallPage::showInfomation);
    connect(m_infoControlButton, &InfoControlButton::shrink, this, &SingleInstallPage::hideInfomation);
    connect(m_installButton, &DPushButton::clicked, this, &SingleInstallPage::install);
    connect(m_reinstallButton, &DPushButton::clicked, this, &SingleInstallPage::install);
    connect(m_uninstallButton, &DPushButton::clicked, this, &SingleInstallPage::requestUninstallConfirm);
    connect(m_backButton, &DPushButton::clicked, this, &SingleInstallPage::back);
    connect(m_confirmButton, &DPushButton::clicked, qApp, &QApplication::quit);
    connect(m_doneButton, &DPushButton::clicked, qApp, &QApplication::quit);

    connect(model, &DebListModel::appendOutputInfo, this, &SingleInstallPage::onOutputAvailable);
    connect(model, &DebListModel::transactionProgressChanged, this, &SingleInstallPage::onWorkerProgressChanged);

    if (m_packagesModel->isReady())
        setPackageInfo();
    else
        QTimer::singleShot(120, this, &SingleInstallPage::setPackageInfo);
}

void SingleInstallPage::install()
{
    m_backButton->setVisible(false);
    m_installButton->setVisible(false);

    m_operate = Install;
    m_packagesModel->installAll();
}

void SingleInstallPage::uninstallCurrentPackage()
{
    m_infoControlButton->setShowText(tr("Display uninstall details"));
    m_backButton->setVisible(false);
    m_reinstallButton->setVisible(false);
    m_uninstallButton->setVisible(false);

    m_operate = Uninstall;
    m_packagesModel->uninstallPackage(0);
}

void SingleInstallPage::showInfomation()
{
    m_workerInfomation->setVisible(true);
    m_strengthWidget->setVisible(true);
    m_itemInfoWidget->setVisible(false);
}

void SingleInstallPage::hideInfomation()
{
    m_workerInfomation->setVisible(false);
    m_strengthWidget->setVisible(false);
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
    m_doneButton->setVisible(false);
    m_backButton->setVisible(false);
}

void SingleInstallPage::onOutputAvailable(const QString &output)
{
    m_workerInfomation->append(output.trimmed());

    // pump progress
    if (m_progress->value() < 90) m_progress->setValue(m_progress->value() + 10);

    if (!m_workerStarted) {
        m_workerStarted = true;
        showInfo();
    }
}

void SingleInstallPage::onWorkerFinished()
{
    QFont font = this->font();
    font.setPixelSize(12);
    m_tipsLabel->setFont(font);
    m_progress->setVisible(false);
    m_uninstallButton->setVisible(false);
    m_reinstallButton->setVisible(false);
    m_backButton->setVisible(true);

   QModelIndex index = m_packagesModel->first();
   const int stat = index.data(DebListModel::PackageOperateStatusRole).toInt();

    if (stat == DebListModel::Success) {
        m_doneButton->setVisible(true);
        m_doneButton->setFocus();

        if (m_operate == Install) {
            m_infoControlButton->setShowText(tr("Display install details"));
            m_tipsLabel->setText(tr("Installed successfully"));
            QPalette pe;
            pe.setColor(QPalette::WindowText, QColor("#47790C"));
            m_tipsLabel->setPalette(pe);
        } else {
            m_infoControlButton->setShowText(tr("Display uninstall details"));
            m_tipsLabel->setText(tr("Uninstalled successfully"));
            QPalette pe;
            pe.setColor(QPalette::WindowText, QColor("#FF5A5A"));
            m_tipsLabel->setPalette(pe);
        }

    } else if (stat == DebListModel::Failed) {
        m_confirmButton->setVisible(true);
        m_confirmButton->setFocus();
        QPalette pe;
        pe.setColor(QPalette::WindowText, QColor("#FF5A5A"));
        m_tipsLabel->setPalette(pe);

        if (m_operate == Install)
            m_tipsLabel->setText(index.data(DebListModel::PackageFailReasonRole).toString());
        else
        {
            m_tipsLabel->setText(tr("Uninstall Failed"));
        }
    } else {
        Q_UNREACHABLE();
    }
}

void SingleInstallPage::onWorkerProgressChanged(const int progress)
{
    if (progress < m_progress->value()) return;

    m_progress->setValue(progress);

    if (progress == m_progress->maximum()) QTimer::singleShot(100, this, &SingleInstallPage::onWorkerFinished);
}

void SingleInstallPage::setPackageInfo()
{
    qApp->processEvents();

    DebFile *package = m_packagesModel->preparedPackages().first();

    const QIcon icon = QIcon::fromTheme("application-vnd.debian.binary-package", QIcon::fromTheme("debian-swirl"));
    const QPixmap iconPix = icon.pixmap(m_packageIcon->size());

    m_itemInfoWidget->setVisible(true);
    m_packageIcon->setPixmap(iconPix);
    m_packageName->setText(package->packageName());
    m_packageVersion->setText(package->version());

    // set package description
    //    const QRegularExpression multiLine("\n+", QRegularExpression::MultilineOption);
    //    const QString description = package->longDescription().replace(multiLine, "\n");
    const QString description = package->longDescription();
    const QSize boundingSize = QSize(m_packageDescription->width(), m_packageDescription->maximumHeight());
    m_packageDescription->setText(holdTextInRect(m_packageDescription->font(), description, boundingSize));

    // package install status
    const QModelIndex index = m_packagesModel->index(0);
    const int installStat = index.data(DebListModel::PackageVersionStatusRole).toInt();

    const bool installed = installStat != DebListModel::NotInstalled;
    const bool installedSameVersion = installStat == DebListModel::InstalledSameVersion;
    m_installButton->setVisible(!installed);
    m_uninstallButton->setVisible(installed);
    m_reinstallButton->setVisible(installed);
    m_confirmButton->setVisible(false);
    m_doneButton->setVisible(false);
    m_backButton->setVisible(true);

    if (installed) {
        if (installedSameVersion) {
            m_tipsLabel->setText(tr("Same version installed"));
            QPalette pe;
            pe.setColor(QPalette::WindowText, QColor("#FF5A5A"));
            m_tipsLabel->setPalette(pe);
        }

        else
            m_tipsLabel->setText(tr("Other version installed: %1")
                                 .arg(index.data(DebListModel::PackageInstalledVersionRole).toString()));
        return;
    }

    // package depends status
    const int dependsStat = index.data(DebListModel::PackageDependsStatusRole).toInt();
    if (dependsStat == DebListModel::DependsBreak) {
        m_tipsLabel->setText(index.data(DebListModel::PackageFailReasonRole).toString());
        QPalette pe;
        pe.setColor(QPalette::WindowText, QColor("#FF5A5A"));
        m_tipsLabel->setPalette(pe);
        m_installButton->setVisible(false);
        m_reinstallButton->setVisible(false);
        m_confirmButton->setVisible(true);
        m_backButton->setVisible(true);
    }
}
