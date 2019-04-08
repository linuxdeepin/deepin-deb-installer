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

#include <QVBoxLayout>
#include <QDebug>

UninstallConfirmPage::UninstallConfirmPage(QWidget *parent)
    : QWidget(parent),

      m_icon(new QLabel),
      m_tips(new QLabel),
      m_infoWrapperWidget(new QWidget),
      m_infoControl(new InfoControlButton(tr("Display related packages"), tr("Collapse"))),
      m_dependsInfomation(new QTextEdit),
      m_cancelBtn(new QPushButton),
      m_confirmBtn(new QPushButton)
{
    const QIcon icon = QIcon::fromTheme("application-vnd.debian.binary-package", QIcon::fromTheme("debian-swirl"));

    m_icon->setFixedSize(64, 64);
    m_icon->setPixmap(icon.pixmap(64, 64));
    m_tips->setAlignment(Qt::AlignCenter);
    m_tips->setStyleSheet("QLabel {"
                          "padding: 20px 0 0 0;"
                          "}");
    m_cancelBtn->setText(tr("Cancel"));
    m_cancelBtn->setFixedSize(120, 36);
    m_confirmBtn->setText(tr("Confirm"));
    m_confirmBtn->setFixedSize(120, 36);

    m_dependsInfomation->setReadOnly(true);
    m_dependsInfomation->setVisible(false);
    m_dependsInfomation->setAcceptDrops(false);
    m_dependsInfomation->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_dependsInfomation->setStyleSheet("QTextEdit {"
                                       "color: #609dc9;"
                                       "border: 1px solid #eee;"
                                       "margin: 0 0 20px 0;"
                                       "}");

    QHBoxLayout *btnsLayout = new QHBoxLayout;
    btnsLayout->addStretch();
    btnsLayout->addWidget(m_cancelBtn);
    btnsLayout->addWidget(m_confirmBtn);
    btnsLayout->addStretch();
    btnsLayout->setSpacing(30);
    btnsLayout->setMargin(0);

    QVBoxLayout *centerWrapperLayout = new QVBoxLayout;
    centerWrapperLayout->addStretch();
    centerWrapperLayout->addWidget(m_icon);
    centerWrapperLayout->addSpacing(15);
    centerWrapperLayout->setAlignment(m_icon, Qt::AlignHCenter);
    centerWrapperLayout->addWidget(m_tips);
    centerWrapperLayout->addStretch();
    centerWrapperLayout->setSpacing(0);
    centerWrapperLayout->setMargin(0);

    m_infoWrapperWidget->setLayout(centerWrapperLayout);

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

    connect(m_cancelBtn, &QPushButton::clicked, this, &UninstallConfirmPage::canceled);
    connect(m_confirmBtn, &QPushButton::clicked, this, &UninstallConfirmPage::accepted);
    connect(m_infoControl, &InfoControlButton::expand, this, &UninstallConfirmPage::showDetail);
    connect(m_infoControl, &InfoControlButton::shrink, this, &UninstallConfirmPage::hideDetail);
}

void UninstallConfirmPage::setPackage(const QString &name)
{
    QString tips(tr("Are you sure you want to uninstall %1?\nAll dependencies will also be removed"));

    m_tips->setText(tips.arg(name));
}

void UninstallConfirmPage::setRequiredList(const QStringList &requiredList)
{
    m_infoControl->setVisible(!requiredList.isEmpty());
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
