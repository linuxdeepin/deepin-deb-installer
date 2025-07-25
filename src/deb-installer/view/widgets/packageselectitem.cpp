// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "view/widgets/packageselectitem.h"
#include "model/packageanalyzer.h"
#include "utils/ddlog.h"

#include <DFontSizeManager>
#include <DPalette>

#include <QCheckBox>
#include <QIcon>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

PackageSelectItem::PackageSelectItem(QWidget *parent)
    : QWidget(parent)
    , nameLabel(new Dtk::Widget::DLabel)
    , versionLabel(new Dtk::Widget::DLabel)
    , descriptionLabel(new Dtk::Widget::DLabel)
    , checkBox(new QCheckBox)
{
    qCDebug(appLog) << "Initializing PackageSelectItem...";
    checkBox->setIcon(QIcon::fromTheme("application-x-deb"));
    checkBox->setIconSize(QSize(32, 32));

    auto line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setLineWidth(0);
    line->setMidLineWidth(1);

    auto nameVersionLayout = new QHBoxLayout;
    nameVersionLayout->addWidget(nameLabel);
    nameVersionLayout->addWidget(versionLabel);

    auto infoLayout = new QVBoxLayout;
    infoLayout->addLayout(nameVersionLayout);
    infoLayout->addWidget(descriptionLabel);
    infoLayout->addWidget(line);

    auto allLayout = new QHBoxLayout;
    allLayout->setContentsMargins(0, 0, 0, 0);
    allLayout->addWidget(checkBox);
    allLayout->addLayout(infoLayout);

    setLayout(allLayout);

    descriptionLabel->setFixedWidth(373);

    Dtk::Widget::DFontSizeManager::instance()->bind(nameLabel, Dtk::Widget::DFontSizeManager::T6, QFont::Medium);
    nameLabel->setForegroundRole(Dtk::Gui::DPalette::TextTitle);

    Dtk::Widget::DFontSizeManager::instance()->bind(versionLabel, Dtk::Widget::DFontSizeManager::T6, QFont::Medium);
    versionLabel->setForegroundRole(Dtk::Gui::DPalette::TextTitle);

    Dtk::Widget::DFontSizeManager::instance()->bind(descriptionLabel, Dtk::Widget::DFontSizeManager::T8, QFont::Medium);

    connect(checkBox, &QCheckBox::stateChanged, this, &PackageSelectItem::checkStatusChanged);
    qCDebug(appLog) << "PackageSelectItem initialized";
}
void PackageSelectItem::setDebIR(const DebIr &ir)
{
    qCDebug(appLog) << "Setting package info - name:" << ir.appName << "version:" << ir.version << "archMatched:" << ir.archMatched;
    nameLabel->setText(ir.appName);
    versionLabel->setText(ir.version);
    checkBox->setEnabled(ir.archMatched);

    if (ir.archMatched) {
        descriptionLabel->setForegroundRole(Dtk::Gui::DPalette::TextTips);

        // item显示策略
        QString displayText;
        auto installInfo = PackageAnalyzer::instance().packageInstallStatus(ir);
        auto installStatus = installInfo.first;
        auto installedVersion = installInfo.second;
        switch (installStatus) {
        case Pkg::NotInstalled:  // 未安装
            qCDebug(appLog) << "Package not installed";
            displayText = ir.shortDescription;
            checkBox->setChecked(true);
            break;
        case Pkg::InstalledSameVersion:  // 已安装相同版本
            qCDebug(appLog) << "Package installed same version";
            displayText = tr("Same version installed");
            break;
        case Pkg::InstalledEarlierVersion:  // 已安装较早版本
            qCDebug(appLog) << "Package installed earlier version";
            displayText = tr("Earlier version installed: %1").arg(installedVersion);
            checkBox->setChecked(true);
            break;
        case Pkg::InstalledLaterVersion:  // 已安装较新版本
            qCDebug(appLog) << "Package installed later version";
            displayText = tr("Later version installed: %1").arg(installedVersion);
            break;
        }
        descriptionLabel->setText(displayText);
    } else {
        qCDebug(appLog) << "Package architecture not matched";
        descriptionLabel->setText(tr("Unmatched package architecture"));
        descriptionLabel->setForegroundRole(Dtk::Gui::DPalette::TextWarning);
    }
    qCDebug(appLog) << "Package info set completed";
}

bool PackageSelectItem::isChecked()
{
    qCDebug(appLog) << "Getting checked state";
    return checkBox->isChecked();
}

bool PackageSelectItem::isEnabled()
{
    qCDebug(appLog) << "Getting enabled state";
    return checkBox->isEnabled();
}

void PackageSelectItem::setChecked(bool checked)
{
    qCDebug(appLog) << "Setting checked state:" << checked;
    checkBox->setChecked(checked);
    qCDebug(appLog) << "Checked state changed";
}
