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

const QString holdTextInRect(const QFont &font, QString srcText, const QSize &size)
{
    qDebug() << srcText;

    bool bContainsChinese = srcText.contains(QRegExp("[\\x4e00-\\x9fa5]+"));

    QString text;
    QString tempText;
    int totalHeight = size.height();
    int lineWidth = size.width() - 12;

    int offset = bContainsChinese ? font.pixelSize() : 0;

    QFontMetrics fm(font);

    int calcHeight = 0;
    int lineHeight = fm.height();
    int lineSpace = 0;
    int lineCount = (totalHeight - lineSpace) / lineHeight;
    int prevLineCharIndex = 0;
    for (int charIndex = 0; charIndex < srcText.size() && lineCount >= 0; ++charIndex) {
        int fmWidth = fm.horizontalAdvance(tempText);
        if (fmWidth > lineWidth - offset) {
            calcHeight += lineHeight + 3;
            if (calcHeight + lineHeight > totalHeight) {
                QString endString = srcText.mid(prevLineCharIndex);
                const QString &endText = fm.elidedText(endString, Qt::ElideRight, size.width());
                text += endText;

                lineCount = 0;
                break;
            }

            if (!bContainsChinese) {
                QChar currChar = tempText.at(tempText.length() - 1);
                QChar nextChar = srcText.at(srcText.indexOf(tempText) + tempText.length());
                if (currChar.isLetter() && nextChar.isLetter()) {
                    tempText += '-';
                }
                fmWidth = fm.horizontalAdvance(tempText);
                if (fmWidth > size.width()) {
                    --charIndex;
                    --prevLineCharIndex;
                    tempText = tempText.remove(tempText.length() - 2, 1);
                }
            }
            text += tempText;

            --lineCount;
            if (lineCount > 0) {
                text += "\n";
            }
            tempText = srcText.at(charIndex);

            prevLineCharIndex = charIndex;
        } else {
            tempText += srcText.at(charIndex);
        }
    }

    if (lineCount > 0) {
        text += tempText;
    }

    return text;
}

SingleInstallPage::SingleInstallPage(DebListModel *model, DWidget *parent)
    : DWidget(parent)
    , m_operate(Install)
    , m_workerStarted(false)
    , m_packagesModel(model)
    , m_contentFrame(new DWidget)
    , m_itemInfoFrame(new DWidget)
    , m_packageIcon(new DLabel)
    , m_packageName(new DebInfoLabel)
    , m_packageVersion(new DebInfoLabel)
    , m_packageDescription(new DLabel)
    , m_tipsLabel(new DLabel)
    , m_progressFrame(new DWidget)
    , m_progress(new WorkerProgress)
    , m_installProcessView(new InstallProcessInfoView)
    , m_infoControlButton(new InfoControlButton(tr("Display install details"), tr("Collapse")))
    , m_installButton(new DPushButton)
    , m_uninstallButton(new DPushButton)
    , m_reinstallButton(new DPushButton)
    , m_confirmButton(new DPushButton)
    , m_backButton(new DPushButton)
    , m_doneButton(new DPushButton)
    , m_contentLayout(new QVBoxLayout(m_contentFrame))
    , m_centralLayout(new QVBoxLayout(this))
{
    initUI();
}

void SingleInstallPage::initUI()
{
    initContentLayout();
    initPkgInfoView();
    initPkgInstallProcessView();
    initConnections();

    if (m_packagesModel->isReady())
        setPackageInfo();
    else
        QTimer::singleShot(120, this, &SingleInstallPage::setPackageInfo);

    m_upDown = true;
}

void SingleInstallPage::initContentLayout()
{
    m_contentLayout->setSpacing(0);
    m_contentLayout->setContentsMargins(20, 0, 20, 30);
    m_contentFrame->setLayout(m_contentLayout);
    m_centralLayout->addWidget(m_contentFrame);

    m_centralLayout->setSpacing(0);
    m_centralLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(m_centralLayout);

//#define SHOWBGCOLOR
#ifdef SHOWBGCOLOR
    m_contentFrame->setStyleSheet("QFrame{background: cyan}");
#endif
}

void SingleInstallPage::initPkgInfoView()
{
    QString normalFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    QString mediumFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansMedium);
    QString pkgFontFamily = Utils::loadFontFamilyByType(Utils::DefautFont);

    m_packageName->setObjectName("PackageName");
    m_packageVersion->setObjectName("PackageVersion");

    m_packageIcon->setText("icon");
    m_packageIcon->setFixedSize(64, 64);

    QFont lblFont = Utils::loadFontBySizeAndWeight(mediumFontFamily, 14, QFont::Medium);
    DLabel *packageName = new DLabel;
    DPalette pkgPalette = DApplicationHelper::instance()->palette(packageName);
    pkgPalette.setBrush(DPalette::ToolTipText, pkgPalette.color(DPalette::ToolTipText));
    packageName->setPalette(pkgPalette);
    packageName->setFixedHeight(20);
    packageName->setText(tr("Name: "));
    packageName->setFont(lblFont);
    packageName->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    packageName->setObjectName("PackageNameTitle");

    DLabel *packageVersion = new DLabel;
    pkgPalette = DApplicationHelper::instance()->palette(packageVersion);
    pkgPalette.setBrush(DPalette::ToolTipText, pkgPalette.color(DPalette::ToolTipText));
    packageVersion->setPalette(pkgPalette);
    packageVersion->setFixedHeight(20);
    packageVersion->setText(tr("Version: "));
    packageVersion->setFont(lblFont);
    packageVersion->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    packageVersion->setObjectName("PackageVersionTitle");

    QFont pkgFont = Utils::loadFontBySizeAndWeight(normalFontFamily, 14, QFont::ExtraLight);
    m_packageName->setCustomPalette(DPalette::Dark);
    m_packageName->setFixedHeight(20);
    m_packageName->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_packageName->setFont(pkgFont);
    m_packageVersion->setCustomPalette(DPalette::Dark);
    m_packageVersion->setFixedHeight(20);
    m_packageVersion->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_packageVersion->setFont(pkgFont);

    QVBoxLayout *packageNameVLayout = new QVBoxLayout;
    packageNameVLayout->setSpacing(0);
    packageNameVLayout->setContentsMargins(0, 0, 0, 0);
    packageNameVLayout->addSpacing(4);
    packageNameVLayout->addWidget(packageName);

    QVBoxLayout *pkgNameValueLayout = new QVBoxLayout;
    pkgNameValueLayout->setSpacing(0);
    pkgNameValueLayout->setContentsMargins(0, 0, 0, 0);
    pkgNameValueLayout->addSpacing(4+4);
    pkgNameValueLayout->addWidget(m_packageName);

    QHBoxLayout *pkgNameLayout = new QHBoxLayout;
    pkgNameLayout->setSpacing(0);
    pkgNameLayout->setContentsMargins(0, 0, 0, 0);
    pkgNameLayout->addSpacing(2);
    pkgNameLayout->addLayout(packageNameVLayout);
    pkgNameLayout->addLayout(pkgNameValueLayout);
    pkgNameLayout->addStretch();

    QHBoxLayout *pkgVersionLayout = new QHBoxLayout;
    pkgVersionLayout->setSpacing(0);
    pkgVersionLayout->setContentsMargins(0, 0, 0, 0);
    pkgVersionLayout->addSpacing(2);
    pkgVersionLayout->addWidget(packageVersion);
    pkgVersionLayout->addWidget(m_packageVersion);
    pkgVersionLayout->addStretch();

    QVBoxLayout *itemInfoLayout = new QVBoxLayout;
    itemInfoLayout->setSpacing(0);
    itemInfoLayout->setContentsMargins(0, 0, 0, 0);
    itemInfoLayout->addLayout(pkgNameLayout);
    itemInfoLayout->addLayout(pkgVersionLayout);

    QHBoxLayout *itemBlockLayout = new QHBoxLayout;
    itemBlockLayout->setSpacing(0);
    itemBlockLayout->setContentsMargins(0, 0, 0, 0);
    itemBlockLayout->addSpacing(112-20-10);
    itemBlockLayout->addWidget(m_packageIcon);
    itemBlockLayout->addLayout(itemInfoLayout);

    DWidget *itemInfoWidget = new DWidget(this);
    itemInfoWidget->setLayout(itemBlockLayout);

    QHBoxLayout *packageDescLayout = new QHBoxLayout;
    packageDescLayout->addStretch();
    packageDescLayout->addWidget(m_packageDescription);
    packageDescLayout->addStretch();
    packageDescLayout->setSpacing(0);
    packageDescLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *itemLayout = new QVBoxLayout;
    itemLayout->addSpacing(45);
    itemLayout->addWidget(itemInfoWidget);
    itemLayout->addSpacing(28);
    itemLayout->addLayout(packageDescLayout);
    itemLayout->addStretch();
    itemLayout->setMargin(0);
    itemLayout->setSpacing(0);

    m_itemInfoFrame->setLayout(itemLayout);
    m_itemInfoFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_itemInfoFrame->setVisible(false);

    m_contentLayout->addWidget(m_itemInfoFrame);

#ifdef SHOWBGCOLOR
    packageName->setStyleSheet("{background: blue;}");
    packageVersion->setStyleSheet("{background: red;}");
    m_itemInfoFrame->setStyleSheet("QFrame{background: green;}");
    m_packageName->setStyleSheet("QLabel{background: blue;}");
    m_packageVersion->setStyleSheet("QLabel{background: yellow;}");
    m_packageDescription->setStyleSheet("QLabel{background: orange;}");
    m_packageIcon->setStyleSheet("QLabel{background: brown;}");
#endif
}

void SingleInstallPage::initPkgInstallProcessView()
{
    QString normalFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    QString mediumFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansMedium);

    m_infoControlButton->setObjectName("InfoControlButton");
    m_installProcessView->setObjectName("WorkerInformation");
    m_packageDescription->setObjectName("PackageDescription");

    QFont tipFont = Utils::loadFontBySizeAndWeight(normalFontFamily, 12, QFont::Normal);
    m_tipsLabel->setFont(tipFont);
    m_tipsLabel->setFixedHeight(18);
    m_tipsLabel->setAlignment(Qt::AlignCenter);

    m_progressFrame->setVisible(false);
    m_infoControlButton->setVisible(false);

    m_installProcessView->setVisible(false);
    m_installProcessView->setAcceptDrops(false);
    m_installProcessView->setFixedHeight(200);
    m_installProcessView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

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
    m_installButton->setFocusPolicy(Qt::NoFocus);
    m_uninstallButton->setFocusPolicy(Qt::NoFocus);
    m_reinstallButton->setFocusPolicy(Qt::NoFocus);
    m_confirmButton->setFocusPolicy(Qt::NoFocus);
    m_backButton->setFocusPolicy(Qt::NoFocus);
    m_doneButton->setFocusPolicy(Qt::NoFocus);

    QFont descFont = Utils::loadFontBySizeAndWeight(normalFontFamily, 12, QFont::ExtraLight);
    m_packageDescription->setFixedHeight(73);
    m_packageDescription->setFixedWidth(270);
    m_packageDescription->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_packageDescription->setFont(descFont);
    DPalette palette = DApplicationHelper::instance()->palette(m_packageDescription);
    palette.setBrush(DPalette::ToolTipText, palette.color(DPalette::ItemBackground));
    m_packageDescription->setPalette(palette);

    QVBoxLayout *btnsFrameLayout = new QVBoxLayout;
    btnsFrameLayout->setSpacing(0);
    btnsFrameLayout->setContentsMargins(0, 0, 0, 0);

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

    QVBoxLayout *progressLayout = new QVBoxLayout;
    progressLayout->setSpacing(0);
    progressLayout->setContentsMargins(0, 8, 0, 0);
    progressLayout->addWidget(m_progress);
    progressLayout->setAlignment(m_progress, Qt::AlignHCenter);
    m_progressFrame->setLayout(progressLayout);
    m_progressFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    DWidget *btnsFrame = new DWidget;
    btnsFrame->setFixedHeight(m_installButton->maximumHeight());
    btnsFrameLayout->addWidget(m_progressFrame);
    btnsFrameLayout->addStretch();
    btnsFrameLayout->addLayout(btnsLayout);
    btnsFrame->setLayout(btnsFrameLayout);

    m_contentLayout->addWidget(m_infoControlButton);
    m_contentLayout->addWidget(m_installProcessView);
    m_contentLayout->addStretch();
    m_contentLayout->addWidget(m_tipsLabel);
    m_contentLayout->addSpacing(8);
    m_contentLayout->addWidget(btnsFrame);

#ifdef SHOWBGCOLOR
    m_progressFrame->setStyleSheet("QFrame{background:blue}");
    m_tipsLabel->setStyleSheet("QLabel{background: gray}");
    btnsFrame->setStyleSheet("QFrame{background:red}");
    m_infoControlButton->setStyleSheet("QFrame{background: purple}");
    m_installButton->setStyleSheet("QPushButton{background: blue}");
    m_uninstallButton->setStyleSheet("QPushButton{background: yellow}");
    m_reinstallButton->setStyleSheet("QPushButton{background: purple}");
    m_backButton->setStyleSheet("QPushButton{background: brown}");
    m_confirmButton->setStyleSheet("QPushButton{background: pink}");
    m_doneButton->setStyleSheet("QPushButton{background: cyan}");
#endif
}

void SingleInstallPage::initConnections()
{
    connect(m_infoControlButton, &InfoControlButton::expand, this, &SingleInstallPage::showInfomation);
    connect(m_infoControlButton, &InfoControlButton::shrink, this, &SingleInstallPage::hideInfomation);
    connect(m_installButton, &DPushButton::clicked, this, &SingleInstallPage::install);
    connect(m_reinstallButton, &DPushButton::clicked, this, &SingleInstallPage::reinstall);
    connect(m_uninstallButton, &DPushButton::clicked, this, &SingleInstallPage::requestUninstallConfirm);
    connect(m_backButton, &DPushButton::clicked, this, &SingleInstallPage::back);
    connect(m_confirmButton, &DPushButton::clicked, qApp, &QApplication::quit);
    connect(m_doneButton, &DPushButton::clicked, qApp, &QApplication::quit);

    connect(m_packagesModel, &DebListModel::appendOutputInfo, this, &SingleInstallPage::onOutputAvailable);
    connect(m_packagesModel, &DebListModel::transactionProgressChanged, this, &SingleInstallPage::onWorkerProgressChanged);
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
    m_installProcessView->setVisible(true);
    m_itemInfoFrame->setVisible(false);
}

void SingleInstallPage::hideInfomation()
{
    m_upDown = true;
    m_installProcessView->setVisible(false);
    m_itemInfoFrame->setVisible(true);
}

void SingleInstallPage::showInfo()
{
    m_infoControlButton->setVisible(true);
    m_progressFrame->setVisible(true);
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
    m_installProcessView->appendText(output.trimmed());

    // pump progress
    if (m_progress->value() < 90) m_progress->setValue(m_progress->value() + 10);

    if (!m_workerStarted) {
        m_workerStarted = true;
        showInfo();
    }
}

void SingleInstallPage::onWorkerFinished()
{
    m_progressFrame->setVisible(false);
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
            palette = DebApplicationHelper::instance()->palette(m_tipsLabel);
            palette.setColor(QPalette::WindowText, palette.color(DPalette::DarkLively));
            m_tipsLabel->setPalette(palette);
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

    const QIcon icon = QIcon::fromTheme("application-x-deb");

    QPixmap iconPix = icon.pixmap(m_packageIcon->size());

    m_itemInfoFrame->setVisible(true);
    m_packageIcon->setPixmap(iconPix);
    m_packageName->setText(package->packageName());
    m_packageVersion->setText(package->version());

    // set package description
    //    const QRegularExpression multiLine("\n+", QRegularExpression::MultilineOption);
    //    const QString description = package->longDescription().replace(multiLine, "\n");
    const QString description = Utils::fromSpecialEncoding(package->longDescription());
    const QSize boundingSize = QSize(m_packageDescription->width(), 72);
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
            palette = DApplicationHelper::instance()->palette(m_tipsLabel);
            palette.setBrush(DPalette::WindowText, palette.color(DPalette::TextWarning));
            m_tipsLabel->setPalette(palette);
            m_tipsLabel->setText(tr("Same version installed"));
        }
        else {
            palette = DApplicationHelper::instance()->palette(m_tipsLabel);
            palette.setBrush(DPalette::WindowText, palette.color(DPalette::TextWarning));
            m_tipsLabel->setPalette(palette);
            m_tipsLabel->setText(tr("Other version installed: %1")
                                 .arg(index.data(DebListModel::PackageInstalledVersionRole).toString()));
        }
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
