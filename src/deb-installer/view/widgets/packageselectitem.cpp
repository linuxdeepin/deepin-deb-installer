// SPDX-FileCopyrightText: 2023-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "view/widgets/packageselectitem.h"
#include "model/packageanalyzer.h"

#include <DFontSizeManager>
#include <DPalette>

#include <QCheckBox>
#include <QIcon>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QTimer>

#include <DGuiApplicationHelper>

PackageSelectItem::PackageSelectItem(QWidget *parent)
    : QWidget (parent)
    , nameLabel(new Dtk::Widget::DLabel)
    , versionLabel(new Dtk::Widget::DLabel)
    , descriptionLabel(new Dtk::Widget::DLabel)
    , checkBox(new QCheckBox)
{
    checkBox->setIcon(QIcon::fromTheme("application-x-deb"));
    checkBox->setIconSize(QSize(32, 32));

    auto nameVersionLayout = new QHBoxLayout;
    nameVersionLayout->addWidget(nameLabel, 1);
    nameVersionLayout->addWidget(versionLabel);

    auto infoLayout = new QVBoxLayout;
    infoLayout->addLayout(nameVersionLayout);
    infoLayout->addWidget(descriptionLabel);

    auto allLayout = new QHBoxLayout;
    allLayout->setMargin(0);
    allLayout->addWidget(checkBox);
    allLayout->addLayout(infoLayout);

    setLayout(allLayout);

    nameLabel->installEventFilter(this);
    versionLabel->installEventFilter(this);
    descriptionLabel->installEventFilter(this);

    descriptionLabel->setFixedWidth(373);

    Dtk::Widget::DFontSizeManager::instance()->bind(nameLabel, Dtk::Widget::DFontSizeManager::T6, QFont::Medium);
    nameLabel->setForegroundRole(Dtk::Gui::DPalette::TextTitle);
    nameLabel->setElideMode(Qt::ElideRight);
    nameLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

    Dtk::Widget::DFontSizeManager::instance()->bind(versionLabel, Dtk::Widget::DFontSizeManager::T6, QFont::Medium);
    versionLabel->setForegroundRole(Dtk::Gui::DPalette::TextTitle);

    Dtk::Widget::DFontSizeManager::instance()->bind(descriptionLabel, Dtk::Widget::DFontSizeManager::T8, QFont::Medium);
    descriptionLabel->setElideMode(Qt::ElideRight);

    connect(checkBox, &QCheckBox::stateChanged, this, &PackageSelectItem::checkStatusChanged);
}

void PackageSelectItem::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Draw divider line aligned with text content, 10px right margin.
    // Mirrors old-path PackagesListDelegate divider (packageslistdelegate.cpp).
    const int content_x = descriptionLabel->geometry().x();
    QRect lineRect;
    lineRect.setX(content_x);
    lineRect.setY(rect().height() - 1);
    lineRect.setWidth(rect().width() - content_x - 10);
    lineRect.setHeight(1);

    auto themeType = DGuiApplicationHelper::instance()->themeType();
    auto fillColor = themeType == DGuiApplicationHelper::LightType
                         ? QColor(0, 0, 0, 12)
                         : QColor(255, 255, 255, 12);
    painter.fillRect(lineRect, fillColor);
}

void PackageSelectItem::setDebIR(const DebIr &ir)
{
    nameLabel->setText(ir.appName);
    versionLabel->setText(ir.version);
    checkBox->setEnabled(ir.archMatched);

    if (ir.archMatched) {
        descriptionLabel->setForegroundRole(Dtk::Gui::DPalette::TextTips);

        //item显示策略
        QString displayText;
        auto installInfo = PackageAnalyzer::instance().packageInstallStatus(ir);
        auto installStatus = installInfo.first;
        auto installedVersion = installInfo.second;
        switch (installStatus) {
        case PackageAnalyzer::NotInstalled: //未安装
            displayText = ir.shortDescription;
            checkBox->setChecked(true);
            break;
        case PackageAnalyzer::InstalledSameVersion: //已安装相同版本
            displayText = tr("Same version installed");
            break;
        case PackageAnalyzer::InstalledEarlierVersion: //已安装较早版本
            displayText = tr("Earlier version installed: %1").arg(installedVersion);
            checkBox->setChecked(true);
            break;
        case PackageAnalyzer::InstalledLaterVersion: //已安装较新版本
            displayText = tr("Later version installed: %1").arg(installedVersion);
            break;
        }
        descriptionLabel->setText(displayText);
    } else {
        descriptionLabel->setText(tr("Unmatched package architecture"));
        descriptionLabel->setForegroundRole(Dtk::Gui::DPalette::TextWarning);
    }
}

bool PackageSelectItem::isChecked()
{
    return checkBox->isChecked();
}

bool PackageSelectItem::isEnabled()
{
    return checkBox->isEnabled();
}

void PackageSelectItem::setChecked(bool checked)
{
    checkBox->setChecked(checked);
}

QSize PackageSelectItem::sizeHint() const
{
    auto *lay = layout();
    lay->invalidate();
    lay->activate();
    return QSize(460, lay->sizeHint().height());
}

bool PackageSelectItem::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        QTimer::singleShot(0, this, [this]() { emit sizeChanged(); });
    }
    return QWidget::eventFilter(watched, event);
}
