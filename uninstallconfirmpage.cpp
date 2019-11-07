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

#include "uninstallconfirmpage.h"
#include "utils.h"

#include <QDebug>
#include <QVBoxLayout>

UninstallConfirmPage::UninstallConfirmPage(DWidget *parent)
    : DWidget(parent)
    , m_icon(new DLabel)
    , m_tips(new DLabel)
    , m_infoWrapperWidget(new DWidget)
    , m_infoControl(new InfoControlButton(tr("Display related packages"), tr("Collapse")))
    , m_dependsInfomation(new DTextEdit)
    , m_cancelBtn(new DPushButton)
    , m_confirmBtn(new DPushButton)
{
    const QIcon icon = QIcon::fromTheme("application-x-deb");

    m_icon->setFixedSize(64, 64);
    m_icon->setPixmap(icon.pixmap(64, 64));

    QString normalFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    QString mediumFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansMedium);
    m_tips->setFont(Utils::loadFontBySizeAndWeight(normalFontFamily, 14, QFont::Normal));
    m_tips->setAlignment(Qt::AlignCenter);

    QFont font = this->font();
    font.setPixelSize(14);
    m_cancelBtn->setFont(font);
    m_confirmBtn->setFont(font);
    m_dependsInfomation->setFont(font);

    QFont btnFont = Utils::loadFontBySizeAndWeight(mediumFontFamily, 14, QFont::Medium);
    m_cancelBtn->setText(tr("Cancel"));
    m_cancelBtn->setFixedSize(120, 36);
    m_confirmBtn->setText(tr("Confirm"));
    m_confirmBtn->setFixedSize(120, 36);
    m_confirmBtn->setFocusPolicy(Qt::NoFocus);
    m_cancelBtn->setFocusPolicy(Qt::NoFocus);
    m_cancelBtn->setFont(btnFont);
    m_confirmBtn->setFont(btnFont);

    m_dependsInfomation->setReadOnly(true);
    m_dependsInfomation->setVisible(false);
    m_dependsInfomation->setAcceptDrops(false);
    m_dependsInfomation->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QHBoxLayout *btnsLayout = new QHBoxLayout;
    btnsLayout->setSpacing(0);
    btnsLayout->setContentsMargins(0, 0, 0, 0);
    btnsLayout->addStretch();
    btnsLayout->addWidget(m_cancelBtn);
    btnsLayout->addSpacing(20);
    btnsLayout->addWidget(m_confirmBtn);
    btnsLayout->addStretch();

    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->addStretch();
    contentLayout->addWidget(m_icon);
    contentLayout->addSpacing(15);
    contentLayout->setAlignment(m_icon, Qt::AlignHCenter);
    contentLayout->addWidget(m_tips);
    contentLayout->addStretch();

    m_infoWrapperWidget->setLayout(contentLayout);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_infoWrapperWidget);
    centralLayout->addWidget(m_infoControl);
    centralLayout->setAlignment(m_infoControl, Qt::AlignHCenter);
    centralLayout->addSpacing(15);
    centralLayout->addWidget(m_dependsInfomation);
    centralLayout->addLayout(btnsLayout);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(20, 0, 20, 30);

    setLayout(centralLayout);

    connect(m_cancelBtn, &DPushButton::clicked, this, &UninstallConfirmPage::canceled);
    connect(m_confirmBtn, &DPushButton::clicked, this, &UninstallConfirmPage::accepted);
    connect(m_infoControl, &InfoControlButton::expand, this, &UninstallConfirmPage::showDetail);
    connect(m_infoControl, &InfoControlButton::shrink, this, &UninstallConfirmPage::hideDetail);
}

void UninstallConfirmPage::setPackage(const QString &name)
{
    QString tips = tr("Are you sure you want to uninstall %1?\nAll dependencies will also be removed");
    if(!m_requiredList.isEmpty()) {
        tips = tr("Are you sure you want to uninstall %1?\nUninstalling this software may cause other dependent software\nunusable");
    }

    m_tips->setText(tips.arg(name));
}

void UninstallConfirmPage::setRequiredList(const QStringList &requiredList)
{
    m_requiredList = requiredList;
    if (!requiredList.isEmpty()) {
        m_infoControl->setVisible(true);
    }
    else {
        m_infoControl->setVisible(false);
    }
    m_dependsInfomation->setText(requiredList.join(", "));
}

void UninstallConfirmPage::showDetail()
{
    m_infoWrapperWidget->setVisible(false);
    m_dependsInfomation->setVisible(true);
}

void UninstallConfirmPage::hideDetail()
{
    m_infoWrapperWidget->setVisible(true);
    m_dependsInfomation->setVisible(false);
}
