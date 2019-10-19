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
#include "workerprogress.h"
#include "utils.h"

#include <QApplication>
#include <QDebug>
#include <QRegularExpression>
#include <QTextLayout>
#include <QTimer>
#include <QVBoxLayout>

#include <QApt/DebFile>
#include <QApt/Transaction>
#include <DStyleHelper>
#include <DApplicationHelper>
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
        height += lineHeight + 5;

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

    QString normalFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    QString mediumFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansMedium);
    QString pkgFontFamily1 = Utils::loadFontFamilyByType(Utils::DefautFont);

    m_packageName->setObjectName("PackageName");
    m_packageVersion->setObjectName("PackageVersion");
    m_infoControlButton->setObjectName("InfoControlButton");
    m_workerInfomation->setObjectName("WorkerInformation");
    m_packageDescription->setObjectName("PackageDescription");

    m_packageIcon->setText("icon");
    m_packageIcon->setFixedSize(64, 64);

    QFont pkgFont1 = Utils::loadFontBySizeAndWeight(pkgFontFamily1, 14, QFont::Light);
    QFont pkgFont2 = Utils::loadFontBySizeAndWeight(pkgFontFamily1, 14, QFont::Light);
    qDebug() << pkgFont1.family();
    qDebug() << pkgFont2.family();
    DPalette pkgPalette = DApplicationHelper::instance()->palette(m_packageName);
    pkgPalette.setBrush(DPalette::ToolTipText, pkgPalette.color(DPalette::ToolTipText));
    m_packageName->setPalette(pkgPalette);
    m_packageName->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    m_packageName->setFont(pkgFont1);
    m_packageVersion->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_packageVersion->setFont(pkgFont2);

    QFont tipFont = Utils::loadFontBySizeAndWeight(normalFontFamily, 12, QFont::Normal);
    m_tipsLabel->setFont(tipFont);
    m_tipsLabel->setAlignment(Qt::AlignCenter);
//#define SHOWBORDER
#ifdef SHOWBORDER
    m_packageName->setStyleSheet("QLabel{border: 1px solid blue;}");
    m_packageVersion->setStyleSheet("QLabel{border: 1px solid yellow;}");
    m_workerInfomation->setStyleSheet("QLabel{border: 1px solid purple;}");
    m_packageDescription->setStyleSheet("QLabel{border: 1px solid red;}");
    m_packageIcon->setStyleSheet("QLabel{border: 1px solid black;}");
#endif

    m_progress->setVisible(false);
    m_infoControlButton->setVisible(false);

    QFont infoFont = Utils::loadFontBySizeAndWeight(normalFontFamily, 11, QFont::Normal);
    m_workerInfomation->setFont(infoFont);
    m_workerInfomation->setTextColor(QColor("#609DC8"));
    m_workerInfomation->setReadOnly(true);
    m_workerInfomation->setVisible(false);
    m_workerInfomation->setAcceptDrops(false);
    m_workerInfomation->setFixedHeight(200);
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


    QFont btnFont = Utils::loadFontBySizeAndWeight(mediumFontFamily, 14, QFont::Medium);
    m_installButton->setFixedSize(120,36);
    m_uninstallButton->setFixedSize(120,36);
    m_reinstallButton->setFixedSize(120,36);
    m_confirmButton->setFixedSize(120,36);
    m_backButton->setFixedSize(120,36);
    m_doneButton->setFixedSize(120,36);
    m_installButton->setFont(btnFont);
    m_uninstallButton->setFont(btnFont);
    m_reinstallButton->setFont(btnFont);
    m_confirmButton->setFont(btnFont);
    m_backButton->setFont(btnFont);
    m_doneButton->setFont(btnFont);
    m_uninstallButton->setFocusPolicy(Qt::ClickFocus);
    m_reinstallButton->setFocusPolicy(Qt::ClickFocus);
    m_installButton->setFocusPolicy(Qt::ClickFocus);

    QFont descFont = Utils::loadFontBySizeAndWeight(normalFontFamily, 12, QFont::ExtraLight);
    m_packageDescription->setFixedHeight(70);
    m_packageDescription->setFixedWidth(270);
    m_packageDescription->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_packageDescription->setFont(descFont);
    DPalette palette = DApplicationHelper::instance()->palette(m_packageDescription);
    palette.setBrush(DPalette::ToolTipText, palette.color(DPalette::ItemBackground));
    m_packageDescription->setPalette(palette);

    QFont lblFont = Utils::loadFontBySizeAndWeight(mediumFontFamily, 14, QFont::Medium);
    DLabel *packageName = new DLabel;
    packageName->setText(tr("Name: "));
    packageName->setFont(lblFont);
    packageName->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    packageName->setObjectName("PackageNameTitle");

    DLabel *packageVersion = new DLabel;
    packageVersion->setText(tr("Version: "));
    packageVersion->setFont(lblFont);
    packageVersion->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    packageVersion->setObjectName("PackageVersionTitle");
#ifdef SHOWBORDER
    packageName->setStyleSheet("QLabel{border: 1px solid blue;}");
    packageVersion->setStyleSheet("QLabel{border: 1px solid red;}");
#endif
    QGridLayout *itemInfoLayout = new QGridLayout;
    itemInfoLayout->addWidget(packageName, 0, 0);
    itemInfoLayout->addWidget(m_packageName, 0, 1);
    itemInfoLayout->addWidget(packageVersion, 1, 0);
    itemInfoLayout->addWidget(m_packageVersion, 1, 1);
    itemInfoLayout->setSpacing(0);
    itemInfoLayout->setVerticalSpacing(0);
    itemInfoLayout->setHorizontalSpacing(0);
    itemInfoLayout->setMargin(0);

    QHBoxLayout *itemBlockLayout = new QHBoxLayout;
    itemBlockLayout->addWidget(m_packageIcon);
    itemBlockLayout->addLayout(itemInfoLayout);
    itemBlockLayout->addStretch();
    itemBlockLayout->setSpacing(0);
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
    btnsLayout->setSpacing(20);
    btnsLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *itemLayout = new QVBoxLayout;
    itemLayout->addSpacing(35);
    itemLayout->addLayout(itemBlockLayout);
    itemLayout->addSpacing(28);
    itemLayout->addWidget(m_packageDescription);
    itemLayout->addStretch();
    itemLayout->setMargin(0);
    itemLayout->setSpacing(0);

    m_itemInfoWidget->setLayout(itemLayout);
    m_itemInfoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_itemInfoWidget->setVisible(false);

    m_strengthWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_strengthWidget->setVisible(false);

    centralLayout = new QVBoxLayout;
    centralLayout->addSpacing(10);
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
    centralLayout->setContentsMargins(10, 0, 10, 30);

    setLayout(centralLayout);

    connect(m_infoControlButton, &InfoControlButton::expand, this, &SingleInstallPage::showInfomation);
    connect(m_infoControlButton, &InfoControlButton::shrink, this, &SingleInstallPage::hideInfomation);
    connect(m_installButton, &DPushButton::clicked, this, &SingleInstallPage::install);
    connect(m_reinstallButton, &DPushButton::clicked, this, &SingleInstallPage::reinstall);
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

    m_upDown = true;
}
void SingleInstallPage::reinstall()
{
    m_backButton->setVisible(false);
    m_installButton->setVisible(false);
    m_reinstallButton->setVisible(false);
    m_uninstallButton->setVisible(false);

    m_operate = Reinstall;
    m_packagesModel->installAll();
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
    m_infoControlButton->setExpandTips(tr("Display uninstall details"));
    m_backButton->setVisible(false);
    m_reinstallButton->setVisible(false);
    m_uninstallButton->setVisible(false);

    m_operate = Uninstall;
    m_packagesModel->uninstallPackage(0);
}

void SingleInstallPage::showInfomation()
{
    m_upDown = false;
    centralLayout->setContentsMargins(20, 0, 20, 30);
    m_workerInfomation->setVisible(true);
    m_strengthWidget->setVisible(true);
    m_itemInfoWidget->setVisible(false);
}

void SingleInstallPage::hideInfomation()
{
    m_upDown = true;
    centralLayout->setContentsMargins(10, 0, 10, 30);
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
    m_progress->setVisible(false);
    m_uninstallButton->setVisible(false);
    m_reinstallButton->setVisible(false);
    m_backButton->setVisible(true);

    QModelIndex index = m_packagesModel->first();
    const int stat = index.data(DebListModel::PackageOperateStatusRole).toInt();

    DPalette palette;
    if (stat == DebListModel::Success) {
        m_doneButton->setVisible(true);

        if (m_operate == Install || m_operate == Reinstall) {
            m_infoControlButton->setExpandTips(tr("Display install details"));
            m_tipsLabel->setText(tr("Installed successfully"));
            QPalette pe;
            pe.setColor(QPalette::WindowText, QColor("#47790C"));
            m_tipsLabel->setPalette(pe);
        } else {
            m_infoControlButton->setExpandTips(tr("Display uninstall details"));
            m_tipsLabel->setText(tr("Uninstalled successfully"));

            palette = DApplicationHelper::instance()->palette(m_tipsLabel);
            palette.setBrush(DPalette::WindowText, palette.color(DPalette::TextWarning));
            m_tipsLabel->setPalette(palette);
        }

    } else if (stat == DebListModel::Failed) {
        m_confirmButton->setVisible(true);

        palette = DApplicationHelper::instance()->palette(m_tipsLabel);
        palette.setBrush(DPalette::WindowText, palette.color(DPalette::TextWarning));
        m_tipsLabel->setPalette(palette);

        if (m_operate == Install || m_operate == Reinstall)
            m_tipsLabel->setText(index.data(DebListModel::PackageFailReasonRole).toString());
        else
        {
            m_tipsLabel->setText(tr("Uninstall Failed"));
        }
    } else {
        Q_UNREACHABLE();
    }

    if(!m_upDown)
        m_infoControlButton->setShrinkTips(tr("Collapse"));
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

    QPixmap iconPix = icon.pixmap(m_packageIcon->size());

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
    //m_backButton->setVisible(true);

    DPalette palette;
    if (installed) {
        if (installedSameVersion) {
            m_tipsLabel->setText(tr("Same version installed"));

            palette = DApplicationHelper::instance()->palette(m_tipsLabel);
            palette.setBrush(DPalette::WindowText, palette.color(DPalette::TextWarning));
            m_tipsLabel->setPalette(palette);
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
        palette = DApplicationHelper::instance()->palette(m_tipsLabel);
        palette.setBrush(DPalette::WindowText, palette.color(DPalette::TextWarning));
        m_tipsLabel->setPalette(palette);

        m_installButton->setVisible(false);
        m_reinstallButton->setVisible(false);
        m_confirmButton->setVisible(true);
        m_backButton->setVisible(true);
    }
}
void SingleInstallPage::afterGetAutherFalse()
{
    if( m_operate == Install)
    {
        //m_backButton->setVisible(true);
        m_installButton->setVisible(true);
    }
    else if(m_operate == Uninstall)
    {
        //m_backButton->setVisible(true);
        m_reinstallButton->setVisible(true);
        m_uninstallButton->setVisible(true);
    }
    else if(m_operate == Reinstall)
    {
        //m_backButton->setVisible(true);
        m_reinstallButton->setVisible(true);
        m_uninstallButton->setVisible(true);
    }
}
