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

SingleInstallPage::SingleInstallPage(DebListModel *model, QWidget *parent)
    : QWidget(parent)
    , m_operate(Install)
    , m_workerStarted(false)
    , m_packagesModel(model)
    , m_contentFrame(new QWidget(this))
    , m_itemInfoFrame(new QWidget(this))
    , m_packageIcon(new DLabel(this))
    , m_packageName(new DebInfoLabel(this))
    , m_packageVersion(new DebInfoLabel(this))
    , m_packageDescription(new DLabel(this))
    , m_tipsLabel(new DebInfoLabel(this))
    , m_progressFrame(new QWidget(this))
    , m_progress(new WorkerProgress(this))
    , m_installProcessView(new InstallProcessInfoView(440, 190, this))
    , m_infoControlButton(new InfoControlButton(QApplication::translate("SingleInstallPage_Install", "Show details"), tr("Collapse")))
    , m_installButton(new DPushButton(this))
    , m_uninstallButton(new DPushButton(this))
    , m_reinstallButton(new DPushButton(this))
    , m_confirmButton(new DPushButton(this))
    , m_backButton(new DPushButton(this))
    , m_doneButton(new DPushButton(this))
    , m_contentLayout(new QVBoxLayout(m_contentFrame))
    , m_centralLayout(new QVBoxLayout(this))
    , m_pDSpinner(new DSpinner(this))
    , m_pLoadingLabel(new DCommandLinkButton("", this))
{
    initUI();
    initControlAccessibleName();// 自动化测试
}

void SingleInstallPage::initUI()
{
    QApplication::restoreOverrideCursor();
    QFontInfo fontinfo = this->fontInfo();
    int fontsize = fontinfo.pixelSize();
    initContentLayout();
    initPkgInfoView(fontsize);
    initPkgInstallProcessView(fontsize);
    initConnections();

    if (m_packagesModel->isReady())
        setPackageInfo();
    else
        QTimer::singleShot(120, this, &SingleInstallPage::setPackageInfo);

    m_upDown = true;

    QString File_transfer_Action;
    QString Targetfilepath = "/tmp/.UOS_Installer_build";
    QFileInfo fi(Targetfilepath);
    bool exist = fi.exists();
    if (!exist) {
        File_transfer_Action = "rm -rf " + Targetfilepath;
        system(File_transfer_Action.toStdString().c_str());
        qDebug() << "删除目标文件夹：" << File_transfer_Action;
    }
}

/**
 * @brief SingleInstallPage::initControlAccessibleName 初始化控件的AccessibleName
 * 兼容自动化测试
 */
void SingleInstallPage::initControlAccessibleName()
{
    m_packageName->setObjectName("SinglePagePackageName");
    m_packageName->setAccessibleName("SinglePagePackageName");

    m_packageIcon->setObjectName("SinglePagePackageIcon");
    m_packageIcon->setAccessibleName("SinglePagePackageIcon");

    m_packageVersion->setObjectName("SinglePagePackageVersion");
    m_packageVersion->setAccessibleName("SinglePagePackageVersion");

    m_packageDescription->setObjectName("SinglePagePackageDescription");
    m_packageDescription->setAccessibleName("SinglePagePackageDescription");

    m_tipsLabel->setObjectName("SinglePagePackageStatusTips");
    m_tipsLabel->setAccessibleName("SinglePagePackageStatusTips");

    m_pLoadingLabel->setObjectName("SinglePagePackageLoadingTips");
    m_pLoadingLabel->setAccessibleName("SinglePagePackageLoadingTips");
}

void SingleInstallPage::initContentLayout()
{
    m_contentLayout->addSpacing(10);
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

void SingleInstallPage::initInstallWineLoadingLayout()
{
    QVBoxLayout *m_pLoadingLayout = new QVBoxLayout(this);

    m_pDSpinner->setMinimumSize(24, 24);
    m_pDSpinner->setVisible(false);
    m_pDSpinner->start();
    m_pLoadingLayout->addWidget(m_pDSpinner);
    m_pLoadingLayout->setAlignment(m_pDSpinner, Qt::AlignHCenter);


    m_pLoadingLayout->addSpacing(4); //fix bug:33999 The spinner and The Label are too close together add a distence of 4px
    m_pLoadingLabel->setVisible(false);
    m_pLoadingLayout->setEnabled(false);//fix bug:33999 Make the DCommandLinkbutton looks like a Lable O_o
    m_pLoadingLayout->addWidget(m_pLoadingLabel);
    m_pLoadingLayout->setAlignment(m_pLoadingLabel, Qt::AlignHCenter);//fix bug:33999 keep the label in the middle
    m_pLoadingLabel->setFixedHeight(24);
    QString fontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    Utils::bindFontBySizeAndWeight(m_pLoadingLabel, fontFamily, 12, QFont::ExtraLight);

    m_contentLayout->addLayout(m_pLoadingLayout);

}
void SingleInstallPage::initPkgInfoView(int fontinfosize)
{
    int fontinfosizetemp = 0;
    int fontinfosizetemp_version = 0;
    if (fontinfosize > 18) {
        fontinfosizetemp = 23;
        fontinfosizetemp_version = 25;
    } else {
        fontinfosizetemp = 20;
        fontinfosizetemp_version = 20;
    }

    m_packageIcon->setText("icon");
    m_packageIcon->setFixedSize(64, 64);

    DebInfoLabel *packageName = new DebInfoLabel(this);
    packageName->setCustomQPalette(QPalette::WindowText);
    packageName->setFixedHeight(fontinfosizetemp);
    packageName->setText(tr("Name: "));
    packageName->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    packageName->setObjectName("PackageNameTitle");

    DebInfoLabel *packageVersion = new DebInfoLabel(this);
    packageVersion->setCustomQPalette(QPalette::WindowText);
    packageVersion->setFixedHeight(fontinfosizetemp_version);
    packageVersion->setText(tr("Version: "));
    packageVersion->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    packageVersion->setObjectName("PackageVersionTitle");

    m_packageName->setCustomQPalette(QPalette::WindowText);
    m_packageName->setFixedHeight(fontinfosizetemp);
    m_packageName->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_packageVersion->setCustomQPalette(QPalette::WindowText);
    m_packageVersion->setFixedHeight(fontinfosizetemp_version);
    m_packageVersion->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    QVBoxLayout *packageNameVLayout = new QVBoxLayout(this);
    packageNameVLayout->setSpacing(0);
    packageNameVLayout->setContentsMargins(0, 0, 0, 0);
    packageNameVLayout->addSpacing(4);
    packageNameVLayout->addWidget(packageName);

    QVBoxLayout *pkgNameValueLayout = new QVBoxLayout(this);
    pkgNameValueLayout->setSpacing(0);
    pkgNameValueLayout->setContentsMargins(0, 0, 0, 0);
    pkgNameValueLayout->addSpacing(4 + 4);
    pkgNameValueLayout->addWidget(m_packageName);

    QHBoxLayout *pkgNameLayout = new QHBoxLayout(this);
    pkgNameLayout->setSpacing(0);
    pkgNameLayout->setContentsMargins(0, 0, 0, 0);
    pkgNameLayout->addSpacing(2);
    pkgNameLayout->addLayout(packageNameVLayout);
    pkgNameLayout->addLayout(pkgNameValueLayout);
    pkgNameLayout->addStretch();

    QHBoxLayout *pkgVersionLayout = new QHBoxLayout(this);
    pkgVersionLayout->setSpacing(0);
    pkgVersionLayout->setContentsMargins(0, 0, 0, 0);
    pkgVersionLayout->addSpacing(2);
    pkgVersionLayout->addWidget(packageVersion);
    pkgVersionLayout->addWidget(m_packageVersion);
    pkgVersionLayout->addStretch();

    QVBoxLayout *itemInfoLayout = new QVBoxLayout(this);
    itemInfoLayout->setSpacing(0);
    itemInfoLayout->setContentsMargins(0, 0, 0, 0);
    itemInfoLayout->addLayout(pkgNameLayout);
    itemInfoLayout->addLayout(pkgVersionLayout);

    QHBoxLayout *itemBlockLayout = new QHBoxLayout(this);
    itemBlockLayout->setSpacing(0);
    itemBlockLayout->setContentsMargins(0, 0, 0, 0);
    itemBlockLayout->addSpacing(112 - 20 - 10);
    itemBlockLayout->addWidget(m_packageIcon);
    itemBlockLayout->addLayout(itemInfoLayout);

    QWidget *itemInfoWidget = new QWidget(this);
    itemInfoWidget->setLayout(itemBlockLayout);

    QHBoxLayout *packageDescLayout = new QHBoxLayout(this);
    packageDescLayout->addStretch();
    packageDescLayout->addWidget(m_packageDescription);
    packageDescLayout->addStretch();
    packageDescLayout->setSpacing(0);
    packageDescLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *itemLayout = new QVBoxLayout(this);
    itemLayout->addSpacing(45);
    itemLayout->addWidget(itemInfoWidget);
    itemLayout->addSpacing(20);
    itemLayout->addLayout(packageDescLayout);
    itemLayout->addStretch();
    itemLayout->setMargin(0);
    itemLayout->setSpacing(0);

    QString normalFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    QString mediumFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansMedium);

    Utils::bindFontBySizeAndWeight(packageVersion, mediumFontFamily, 14, QFont::Medium);
    Utils::bindFontBySizeAndWeight(packageName, mediumFontFamily, 14, QFont::Medium);
    Utils::bindFontBySizeAndWeight(m_packageName, normalFontFamily, 14, QFont::ExtraLight);
    Utils::bindFontBySizeAndWeight(m_packageVersion, normalFontFamily, 14, QFont::ExtraLight);

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

/**
 * @brief SingleInstallPage::initTabOrder 设置tab切换焦点的顺序。
 */
void SingleInstallPage::initTabOrder()
{
    // 调整tab切换焦点的顺序，第一个焦点是infoControlButton中的DCommandLinkButton
    // fix bug: https://pms.uniontech.com/zentao/bug-view-46968.html
    QWidget::setTabOrder(m_infoControlButton->controlButton(), m_installButton);
    QWidget::setTabOrder(m_infoControlButton->controlButton(), m_backButton);
    QWidget::setTabOrder(m_infoControlButton->controlButton(), m_uninstallButton);

    QWidget::setTabOrder(m_uninstallButton, m_reinstallButton);

    QWidget::setTabOrder(m_backButton, m_doneButton);
    QWidget::setTabOrder(m_backButton, m_confirmButton);

}

/**
 * @brief SingleInstallPage::initButtonFocusPolicy 添加各个控件的焦点获取策略
 */
void SingleInstallPage::initButtonFocusPolicy()
{
    this->setFocusPolicy(Qt::NoFocus);
    auto focus = Qt::TabFocus;
    m_installButton->setFocusPolicy(focus);
    m_uninstallButton->setFocusPolicy(focus);
    m_reinstallButton->setFocusPolicy(focus);
    m_confirmButton->setFocusPolicy(focus);
    m_backButton->setFocusPolicy(focus);
    m_doneButton->setFocusPolicy(focus);
    m_infoControlButton->controlButton()->setFocusPolicy(focus);
}

/**
 * @brief SingleInstallPage::initButtonAutoDefault 给各个按钮添加回车触发。
 * 没有infoControlButton ,因为属于自定义控件，已经在控件内部实现。
 */
void SingleInstallPage::initButtonAutoDefault()
{
    m_installButton->setAutoDefault(true);
    m_uninstallButton->setAutoDefault(true);
    m_reinstallButton->setAutoDefault(true);
    m_confirmButton->setAutoDefault(true);
    m_backButton->setAutoDefault(true);
    m_doneButton->setAutoDefault(true);

}

void SingleInstallPage::initPkgInstallProcessView(int fontinfosize)
{
    int fontinfosizetemp = 0;
    if (fontinfosize > 16) {
        fontinfosizetemp = 21;
    } else {
        fontinfosizetemp = 18;
    }
    m_infoControlButton->setObjectName("InfoControlButton");
    m_infoControlButton->setAccessibleName("InfoControlButton");
    m_installProcessView->setObjectName("WorkerInformation");
    m_installProcessView->setAccessibleName("WorkerInformation");
    m_packageDescription->setObjectName("PackageDescription");
    m_packageDescription->setAccessibleName("PackageDescription");

    m_tipsLabel->setFixedHeight(fontinfosizetemp);
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

    m_installButton->setFixedSize(120, 36);
    m_uninstallButton->setFixedSize(120, 36);
    m_reinstallButton->setFixedSize(120, 36);
    m_confirmButton->setFixedSize(120, 36);
    m_backButton->setFixedSize(120, 36);
    m_doneButton->setFixedSize(120, 36);

    m_contentLayout->addWidget(m_infoControlButton);

    //启用焦点切换。
    initButtonFocusPolicy();
    // 设置按钮回车触发
    initButtonAutoDefault();
    // 初始化按钮焦点切换策略。
    initTabOrder();

    m_packageDescription->setFixedHeight(65);
    m_packageDescription->setFixedWidth(270);
    m_packageDescription->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    QVBoxLayout *btnsFrameLayout = new QVBoxLayout(this);
    btnsFrameLayout->setSpacing(0);
    btnsFrameLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *btnsLayout = new QHBoxLayout(this);
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

    QVBoxLayout *progressLayout = new QVBoxLayout(this);
    progressLayout->setSpacing(0);
    progressLayout->setContentsMargins(0, 8, 0, 0);
    progressLayout->addWidget(m_progress);
    progressLayout->setAlignment(m_progress, Qt::AlignHCenter);
    m_progressFrame->setLayout(progressLayout);
    m_progressFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QWidget *btnsFrame = new QWidget(this);
    btnsFrame->setFixedHeight(m_installButton->maximumHeight());
    btnsFrameLayout->addWidget(m_progressFrame);
    btnsFrameLayout->addStretch();
    btnsFrameLayout->addLayout(btnsLayout);
    btnsFrame->setLayout(btnsFrameLayout);

    QString normalFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    QString mediumFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansMedium);
    Utils::bindFontBySizeAndWeight(m_tipsLabel, normalFontFamily, 12, QFont::Normal);
    Utils::bindFontBySizeAndWeight(m_installButton, mediumFontFamily, 14, QFont::Medium);
    Utils::bindFontBySizeAndWeight(m_uninstallButton, mediumFontFamily, 14, QFont::Medium);
    Utils::bindFontBySizeAndWeight(m_reinstallButton, mediumFontFamily, 14, QFont::Medium);
    Utils::bindFontBySizeAndWeight(m_confirmButton, mediumFontFamily, 14, QFont::Medium);
    Utils::bindFontBySizeAndWeight(m_backButton, mediumFontFamily, 14, QFont::Medium);
    Utils::bindFontBySizeAndWeight(m_doneButton, mediumFontFamily, 14, QFont::Medium);
    Utils::bindFontBySizeAndWeight(m_packageDescription, normalFontFamily, 12, QFont::ExtraLight);

    m_contentLayout->addWidget(m_installProcessView);
    m_contentLayout->addStretch();
    m_contentLayout->addWidget(m_tipsLabel);
    m_contentLayout->addStretch();
    m_contentLayout->addWidget(btnsFrame);

    initInstallWineLoadingLayout();

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
//    connect(m_doneButton, &DPushButton::clicked, qApp, &QApplication::quit);
    connect(m_doneButton, &DPushButton::clicked, qApp, [ = ] {
        QString Targetfilepath = "/tmp/.UOS_Installer_build";
        QString delete_action = "rm -rf " + Targetfilepath;
        system(delete_action.toStdString().c_str());
        QApplication::quit();
    });

    connect(m_packagesModel, &DebListModel::appendOutputInfo, this, &SingleInstallPage::onOutputAvailable);
    connect(m_packagesModel, &DebListModel::onStartInstall, this, [ = ] {
        m_progressFrame->setVisible(true);
    });
    connect(m_packagesModel, &DebListModel::transactionProgressChanged, this, &SingleInstallPage::onWorkerProgressChanged);
    connect(m_packagesModel, &DebListModel::DependResult, this, &SingleInstallPage::DealDependResult);
    // 抛弃 CommitErrorFinished 与OnCommitErrorFinished 在listModel中修改为信号workerFinished。
    connect(m_packagesModel, &DebListModel::workerFinished, this, &SingleInstallPage::onWorkerFinished);
}

int SingleInstallPage::initLabelWidth(int fontinfo)
{
    int fontlabelwidth = 0;
    switch (fontinfo) {
    case 11:
        fontlabelwidth = 260;
        break;
    case 12:
        fontlabelwidth = 255;
        break;
    case 13:
        fontlabelwidth = 250;
        break;
    case 14:
        fontlabelwidth = 250;
        break;
    case 15:
        fontlabelwidth = 240;
        break;
    case 16:
        fontlabelwidth = 240;
        break;
    case 18:
        fontlabelwidth = 230;
        break;
    case 20:
        fontlabelwidth = 220;
        break;
    default:
        fontlabelwidth = 220;
        break;
    }
    return fontlabelwidth;
}

void SingleInstallPage::reinstall()
{
    // 重装按钮点击后清除焦点
    // fix bug: https://pms.uniontech.com/zentao/bug-view-46813.html
    m_reinstallButton->clearFocus();
    m_backButton->setVisible(false);
    m_installButton->setVisible(false);
    m_reinstallButton->setVisible(false);
    m_uninstallButton->setVisible(false);
    m_tipsLabel->setVisible(false);

    m_operate = Reinstall;
    m_packagesModel->installPackages();
}
void SingleInstallPage::install()
{
    // 安装按钮点击后清除焦点
    // fix bug: https://pms.uniontech.com/zentao/bug-view-46813.html
    m_installButton->clearFocus();
    m_backButton->setVisible(false);
    m_installButton->setVisible(false);
    m_tipsLabel->setVisible(false);
    m_infoControlButton->setExpandTips(QApplication::translate("SingleInstallPage_Install", "Show details"));
    m_infoControlButton->setVisible(true);

    m_operate = Install;
    m_packagesModel->installPackages();
}

void SingleInstallPage::uninstallCurrentPackage()
{
    // 卸载按钮点击后清除焦点
    // fix bug: https://pms.uniontech.com/zentao/bug-view-46813.html
    m_uninstallButton->clearFocus();
    m_tipsLabel->setVisible(false);
    m_backButton->setVisible(false);
    m_reinstallButton->setVisible(false);
    m_uninstallButton->setVisible(false);
    m_progressFrame->setVisible(true);
    m_infoControlButton->setExpandTips(QApplication::translate("SingleInstallPage_Uninstall", "Show details"));
    m_infoControlButton->setVisible(true);

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
//    m_progressFrame->setVisible(true);
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
    if (!m_infoControlButton->isVisible())
        m_infoControlButton->setVisible(true);
    // 如果当前要输出的信息是dpkg running,waitting... 进度不增加。
    if (m_progress->value() < 90 && !output.contains("dpkg running, waitting...")) m_progress->setValue(m_progress->value() + 10);

    if (!m_workerStarted) {
        m_workerStarted = true;
        showInfo();
    }
}

/**
 * @brief SingleInstallPage::OnCommitErrorFinished
 * transaction 返回CommitError时的槽函数，目前不再使用
 * 暂时留用，待下个版本测试后，如果正常，删除。
 */
void SingleInstallPage::OnCommitErrorFinished()
{
    m_tipsLabel->setVisible(true);
    m_progressFrame->setVisible(false);
    m_uninstallButton->setVisible(false);
    m_reinstallButton->setVisible(false);
    m_backButton->setVisible(true);

    m_confirmButton->setVisible(true);
    m_tipsLabel->setCustomDPalette(DPalette::TextWarning);

    if (m_operate == Uninstall)
        m_tipsLabel->setText(tr("Uninstall Failed"));
}

void SingleInstallPage::onWorkerFinished()
{
    m_infoControlButton->setVisible(true);
    m_tipsLabel->setVisible(true);
    m_progressFrame->setVisible(false);
    m_uninstallButton->setVisible(false);
    m_reinstallButton->setVisible(false);
    m_backButton->setVisible(true);

    QModelIndex index = m_packagesModel->first();
    const int stat = index.data(DebListModel::PackageOperateStatusRole).toInt();

    if (stat == DebListModel::Success) {
        m_doneButton->setVisible(true);

        if (m_operate == Install || m_operate == Reinstall) {
            qDebug() << "Installed successfully";
            m_infoControlButton->setExpandTips(QApplication::translate("SingleInstallPage_Install", "Show details"));
            m_tipsLabel->setText(tr("Installed successfully"));
            m_tipsLabel->setCustomDPalette(DPalette::DarkLively);

        } else {
            qDebug() << "Uninstalled successfully";
            m_infoControlButton->setExpandTips(QApplication::translate("SingleInstallPage_Uninstall", "Show details"));
            m_tipsLabel->setText(tr("Uninstalled successfully"));
            m_tipsLabel->setCustomDPalette(DPalette::TextWarning);
        }

    } else if (stat == DebListModel::Failed) {
        m_confirmButton->setVisible(true);
        m_tipsLabel->setCustomDPalette(DPalette::TextWarning);

        if (m_operate == Install || m_operate == Reinstall)
            m_tipsLabel->setText(index.data(DebListModel::PackageFailReasonRole).toString());
        else {
            m_tipsLabel->setText(tr("Uninstall Failed"));
        }
    } else {
        //正常情况不会进入此分支，如果进入此分支表明状态错误。
        m_confirmButton->setVisible(true);
        qDebug() << "Operate Status Error. current"
                 << "index=" << index.row() << "stat=" << stat;
    }

    if (!m_upDown)
        m_infoControlButton->setShrinkTips(tr("Collapse"));
}

void SingleInstallPage::onWorkerProgressChanged(const int progress)
{
    qDebug() << progress << endl;
    if (progress < m_progress->value()) {
        return;
    }

    m_progress->setValue(progress);
    if (progress == m_progress->maximum()) {
        qDebug() << "onWorkerProgressChanged" << progress;
        QTimer::singleShot(100, this, &SingleInstallPage::onWorkerFinished);
    }
}

void SingleInstallPage::setPackageInfo()
{
    qApp->processEvents();
    QFontInfo fontinfosize = this->fontInfo();
    int fontlabelsize = fontinfosize.pixelSize();
    DebFile *package = new DebFile(m_packagesModel->preparedPackages().first());

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
    m_description = description;
    const QSize boundingSize = QSize(m_packageDescription->width(), 54);
    m_packageDescription->setText(Utils::holdTextInRect(m_packageDescription->font(), description, boundingSize));

    //set package name
    packagename_description = Utils::fromSpecialEncoding(package->packageName());
    packageversion_description = Utils::fromSpecialEncoding(package->version());
    delete package;
    if (fontlabelsize > 18) {
        const QSize package_boundingSize = QSize(initLabelWidth(fontlabelsize), 23);
        m_packageName->setText(Utils::holdTextInRect(m_packageName->font(), packagename_description, package_boundingSize));
        const QSize packageversion_boundingSize = QSize(initLabelWidth(fontlabelsize) - 10, 23);
        m_packageVersion->setText(Utils::holdTextInRect(m_packageVersion->font(), packageversion_description, packageversion_boundingSize));
    } else {
        const QSize package_boundingSize = QSize(initLabelWidth(fontlabelsize), 20);
        m_packageName->setText(Utils::holdTextInRect(m_packageName->font(), packagename_description, package_boundingSize));
        const QSize packageversion_boundingSize = QSize(initLabelWidth(fontlabelsize) - 10, 20);
        m_packageVersion->setText(Utils::holdTextInRect(m_packageVersion->font(), packageversion_description, packageversion_boundingSize));
    }

    // package install status
    const QModelIndex index = m_packagesModel->index(0);
    //fix bug:42285 调整状态优先级， 依赖状态 > 安装状态
    //否则会导致安装不同版本的包（依赖不同）时安装依赖出现问题（包括界面混乱、无法下载依赖等）
    const int dependsStat = index.data(DebListModel::PackageDependsStatusRole).toInt();
    qDebug() << "set package info"
             << "depend status" << dependsStat;
    if (dependsStat == DebListModel::DependsBreak || dependsStat == DebListModel::DependsAuthCancel) {
        m_tipsLabel->setText(index.data(DebListModel::PackageFailReasonRole).toString());
        m_tipsLabel->setCustomDPalette(DPalette::TextWarning);

        m_installButton->setVisible(false);
        m_reinstallButton->setVisible(false);
        m_confirmButton->setVisible(true);
        m_backButton->setVisible(true);
        return;
    }
    const int installStat = index.data(DebListModel::PackageVersionStatusRole).toInt();

    const bool installed = installStat != DebListModel::NotInstalled;
    m_installButton->setVisible(!installed);
    m_uninstallButton->setVisible(installed);
    m_reinstallButton->setVisible(installed);
    m_confirmButton->setVisible(false);
    m_doneButton->setVisible(false);

    if (installed) {
        if (installStat == DebListModel::InstalledSameVersion) {
            m_tipsLabel->setCustomDPalette(DPalette::TextWarning);
            m_tipsLabel->setText(tr("Same version installed"));
        } else if (installStat == DebListModel::InstalledLaterVersion) {
            m_tipsLabel->setCustomDPalette(DPalette::TextWarning);
            m_tipsLabel->setText(tr("Later version installed: %1")
                                 .arg(index.data(DebListModel::PackageInstalledVersionRole).toString()));
        } else {
            m_tipsLabel->setCustomDPalette(DPalette::TextWarning);
            m_tipsLabel->setText(tr("Earlier version installed: %1")
                                 .arg(index.data(DebListModel::PackageInstalledVersionRole).toString()));
        }
        return;
    }
}

void SingleInstallPage::setEnableButton(bool bEnable)
{
    // fix bug: 36120 After the uninstall authorization is canceled, hide the uninstall details and display the version status
    m_tipsLabel->setVisible(true);
    m_tipsLabel->setVisible(true);

    m_installButton->setEnabled(bEnable);
    m_reinstallButton->setEnabled(bEnable);
    m_uninstallButton->setEnabled(bEnable);
}

void SingleInstallPage::afterGetAutherFalse()
{
    //等待dpkg启动但是授权取消后，如果详细信息是expend状态，则shrink
    m_infoControlButton->shrink();
    m_infoControlButton->setVisible(false);
    m_progressFrame->setVisible(false);
    if (m_operate == Install) {
        m_installButton->setVisible(true);
    } else if (m_operate == Uninstall) {
        m_reinstallButton->setVisible(true);
        m_uninstallButton->setVisible(true);
    } else if (m_operate == Reinstall) {
        m_reinstallButton->setVisible(true);
        m_uninstallButton->setVisible(true);
    }
}

void SingleInstallPage::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    const QSize boundingSize = QSize(m_packageDescription->width(), 54);
    m_packageDescription->setText(Utils::holdTextInRect(m_packageDescription->font(), m_description, boundingSize));

    DPalette palette = DApplicationHelper::instance()->palette(m_packageDescription);
    palette.setBrush(DPalette::WindowText, palette.color(DPalette::TextTips));
    m_packageDescription->setPalette(palette);
}

void SingleInstallPage::setAuthConfirm(QString dependName)
{
    m_installButton->setVisible(false);
    m_reinstallButton->setVisible(false);
    m_uninstallButton->setVisible(false);
    m_confirmButton->setVisible(false);
    m_backButton->setVisible(false);
    m_pDSpinner->setVisible(true);
    m_pDSpinner->start();
    m_pLoadingLabel->setText(tr("Installing dependencies: %1").arg(dependName));
    m_pLoadingLabel->setVisible(true);
    m_tipsLabel->setVisible(false);
}

void SingleInstallPage::setAuthBefore()
{
    m_tipsLabel->setVisible(true);
    m_progressFrame->setVisible(false);
    QModelIndex index = m_packagesModel->first();
    const int dependsStat = index.data(DebListModel::PackageDependsStatusRole).toInt();
    if (dependsStat == DebListModel::DependsBreak || dependsStat == DebListModel::DependsAuthCancel) {
        m_confirmButton->setVisible(true);
        m_backButton->setVisible(true);
        m_confirmButton->setEnabled(false);
        m_backButton->setEnabled(false);
    } else {
        if (m_operate == Install) {
            m_installButton->setVisible(true);
        } else if (m_operate == Uninstall) {
            m_reinstallButton->setVisible(true);
            m_uninstallButton->setVisible(true);
        } else if (m_operate == Reinstall) {
            m_reinstallButton->setVisible(true);
            m_uninstallButton->setVisible(true);
        }
        m_installButton->setEnabled(false);
        m_reinstallButton->setEnabled(false);
        m_uninstallButton->setEnabled(false);
    }

    m_pLoadingLabel->setVisible(false);
    m_pDSpinner->stop();
    m_pDSpinner->setVisible(false);
}

void SingleInstallPage::setCancelAuthOrAuthDependsErr()
{
    qDebug() << "set Cancel Auth or Auth Depends Error";
    m_tipsLabel->setVisible(true);
    m_progressFrame->setVisible(false);
    QModelIndex index = m_packagesModel->first();
    const int dependsStat = index.data(DebListModel::PackageDependsStatusRole).toInt();
    qDebug() << "cancel Auth" << dependsStat;
    if (dependsStat == DebListModel::DependsBreak || dependsStat == DebListModel::DependsAuthCancel) {
        qDebug() << "confirm button";
        m_confirmButton->setVisible(true);
        m_backButton->setVisible(true);
        m_confirmButton->setEnabled(true);
        m_backButton->setEnabled(true);
        m_installButton->setVisible(false);
        m_reinstallButton->setVisible(false);
        m_uninstallButton->setVisible(false);
    } else {
        m_confirmButton->setVisible(false);
        m_backButton->setVisible(false);
        //fix bug 42285: 在升级安装wine应用（wine->wine5）,依赖安装后，界面显示错乱。
        const int installStat = index.data(DebListModel::PackageVersionStatusRole).toInt();
        if (installStat == DebListModel::NotInstalled) {
            m_installButton->setVisible(true);
            m_installButton->setEnabled(true);
        } else {
            m_reinstallButton->setVisible(true);
            m_uninstallButton->setVisible(true);
            m_reinstallButton->setEnabled(true);
            m_uninstallButton->setEnabled(true);
        }
    }
    m_pLoadingLabel->setVisible(false);
    m_pDSpinner->stop();
    m_pDSpinner->setVisible(false);
}
void SingleInstallPage::DealDependResult(int iAuthRes, QString dependName)
{
    qDebug() << "Deal DependResult" << iAuthRes;
    switch (iAuthRes) {
    case DebListModel::AuthConfirm:
        setAuthConfirm(dependName);
        break;

    case DebListModel::AuthBefore:
        setAuthBefore();
        break;
    case DebListModel::CancelAuth:
    case DebListModel::AnalysisErr:
        setCancelAuthOrAuthDependsErr();
        break;
    case DebListModel::AuthDependsSuccess:
        setCancelAuthOrAuthDependsErr();
        break;
    case DebListModel::AuthDependsErr:
        setCancelAuthOrAuthDependsErr();
        m_tipsLabel->setText(tr("Failed to install %1").arg(dependName));
        m_tipsLabel->setCustomDPalette(DPalette::TextWarning);
        break;
    default:
        break;
    }
}
