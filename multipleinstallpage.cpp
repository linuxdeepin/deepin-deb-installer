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

#include "multipleinstallpage.h"
#include "deblistmodel.h"
#include "packagelistview.h"
#include "packageslistdelegate.h"
#include "widgets/bluebutton.h"
#include "widgets/graybutton.h"
#include "workerprogress.h"

#include <DTitlebar>

#include <QApplication>
#include <DLabel>
#include <QPropertyAnimation>
#include <QTimer>
#include <QVBoxLayout>
MultipleInstallPage::MultipleInstallPage(DebListModel *model, DWidget *parent)
    : DWidget(parent)
    , m_debListModel(model)
    , m_appsView(new PackagesListView)
    , m_infoArea(new DTextEdit)
    , m_infoControlButton(new InfoControlButton(tr("Display install details"), tr("Collapse")))
    , m_installProgress(new WorkerProgress)
    , m_progressAnimation(new QPropertyAnimation(m_installProgress, "value", this))
    , m_installButton(new DPushButton)
    , m_acceptButton(new DPushButton)
    , m_backButton(new DPushButton)
{
    PackagesListDelegate *delegate = new PackagesListDelegate(m_appsView);

    const QFont font_const = this->font();
    QFont font_use = font_const;

    m_appsView->setObjectName("AppsView");
    m_infoArea->setObjectName("InfoArea");
    m_infoControlButton->setObjectName("InfoControlButton");

    m_appsView->setModel(model);
    m_appsView->setFixedSize(460, 186);
    m_appsView->setItemDelegate(delegate);

    m_installButton->setFixedSize(120, 36);
    m_acceptButton->setFixedSize(120, 36);
    m_backButton->setFixedSize(120, 36);

    m_installButton->setText(tr("Install"));
    m_acceptButton->setText(tr("Done"));
    m_acceptButton->setVisible(false);
    m_backButton->setText(tr("Back"));
    m_backButton->setVisible(false);

    font_use.setPixelSize(14);
    m_installButton->setFont(font_use);
    m_acceptButton->setFont(font_use);
    m_backButton->setFont(font_use);

    font_use.setPixelSize(11);
    m_infoArea->setFont(font_use);
    m_infoArea->setTextColor(QColor("#609DC8"));
    m_infoArea->setReadOnly(true);
    m_infoArea->setVisible(false);
    m_infoArea->setAcceptDrops(false);
    m_infoArea->setFixedHeight(200);
    m_infoArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_infoControlButton->setVisible(false);
    m_installProgress->setVisible(false);

    QHBoxLayout *btnsLayout = new QHBoxLayout;
    btnsLayout->addStretch();
    btnsLayout->addWidget(m_installButton);
    btnsLayout->addWidget(m_backButton);
    btnsLayout->addWidget(m_acceptButton);
    btnsLayout->setSpacing(20);
    btnsLayout->addStretch();
    btnsLayout->setContentsMargins(0, 0, 0, 0);

    centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_appsView, Qt::AlignHCenter);
    centralLayout->addWidget(m_infoControlButton);
    centralLayout->setAlignment(m_infoControlButton, Qt::AlignHCenter);
    centralLayout->addWidget(m_infoArea, Qt::AlignHCenter);
    centralLayout->addStretch();
    centralLayout->addWidget(m_installProgress);
    centralLayout->setAlignment(m_installProgress, Qt::AlignHCenter);
    centralLayout->addLayout(btnsLayout);

    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(10, 11, 10, 30);
    setLayout(centralLayout);

    connect(m_infoControlButton, &InfoControlButton::expand, this, &MultipleInstallPage::showInfo);
    connect(m_infoControlButton, &InfoControlButton::shrink, this, &MultipleInstallPage::hideInfo);
    connect(m_installButton, &DPushButton::clicked, m_debListModel, &DebListModel::installAll);
    connect(m_installButton, &DPushButton::clicked, this, &MultipleInstallPage::hiddenCancelButton);
    connect(m_backButton, &DPushButton::clicked, this, &MultipleInstallPage::back);
    connect(m_acceptButton, &DPushButton::clicked, qApp, &QApplication::quit);

    connect(m_appsView, &PackagesListView::clicked, this, &MultipleInstallPage::onItemClicked);
    connect(m_appsView, &PackagesListView::entered, m_debListModel, &DebListModel::setCurrentIndex);

    connect(model, &DebListModel::workerProgressChanged, this, &MultipleInstallPage::onProgressChanged);
    connect(model, &DebListModel::appendOutputInfo, this, &MultipleInstallPage::onOutputAvailable);

    connect(model, &DebListModel::onChangeOperateIndex, this, &MultipleInstallPage::onAutoScrollInstallList);
}

void MultipleInstallPage::onWorkerFinshed()
{
    m_acceptButton->setVisible(true);
    //m_backButton->setVisible(true);
    m_installProgress->setVisible(false);
}

void MultipleInstallPage::onOutputAvailable(const QString &output)
{
    m_infoArea->append(output.trimmed());

    // change to install
    if (!m_installButton->isVisible()) {
        //m_installButton->setVisible(false);

        m_installProgress->setVisible(true);
        m_infoControlButton->setVisible(true);
    }
}

void MultipleInstallPage::onProgressChanged(const int progress)
{
    m_progressAnimation->setStartValue(m_installProgress->value());
    m_progressAnimation->setEndValue(progress);
    m_progressAnimation->start();

    // finished
    if (progress == 100) {
        onOutputAvailable(QString());
        QTimer::singleShot(m_progressAnimation->duration(), this, &MultipleInstallPage::onWorkerFinshed);
    }
}

void MultipleInstallPage::onAutoScrollInstallList(int opIndex)
{
    if (opIndex > 1 && opIndex < m_debListModel->getInstallFileSize()) {
        QModelIndex currIndex = m_debListModel->index(opIndex - 1);
        m_appsView->scrollTo(currIndex, QAbstractItemView::PositionAtTop);
    }
    else if(opIndex == -1)//to top
    {
        QModelIndex currIndex = m_debListModel->index(0);
        m_appsView->scrollTo(currIndex);
    }
}

void MultipleInstallPage::onItemClicked(const QModelIndex &index)
{
    if (!m_debListModel->isWorkerPrepare()) return;

    const int r = index.row();

    emit requestRemovePackage(r);
}

void MultipleInstallPage::showInfo()
{
    centralLayout->setContentsMargins(20, 0, 20, 30);
    m_appsView->setVisible(false);
    m_infoArea->setVisible(true);
    emit hideAutoBarTitle();
}

void MultipleInstallPage::hideInfo()
{
    centralLayout->setContentsMargins(10, 0, 10, 30);
    m_appsView->setVisible(true);
    m_infoArea->setVisible(false);
    emit hideAutoBarTitle();
}
void MultipleInstallPage::hiddenCancelButton()
{
     //m_backButton->setVisible(false);
     m_installButton->setVisible(false);
}
void MultipleInstallPage::afterGetAutherFalse()
{
    //m_backButton->setVisible(true);
    m_installButton->setVisible(true);
}
