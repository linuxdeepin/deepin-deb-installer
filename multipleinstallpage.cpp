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

#include <QApplication>
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>
#include <QVBoxLayout>
MultipleInstallPage::MultipleInstallPage(DebListModel *model, QWidget *parent)
    : QWidget(parent)
    , m_debListModel(model)
    , m_appsView(new PackagesListView)
    , m_infoArea(new QTextEdit)
    , m_infoControlButton(new InfoControlButton(tr("Display details"), tr("Collapse")))
    , m_installProgress(new WorkerProgress)
    , m_progressAnimation(new QPropertyAnimation(m_installProgress, "value", this))
    , m_installButton(new BlueButton)
    , m_acceptButton(new GrayButton)
    , m_backButton(new GrayButton) {
    PackagesListDelegate *delegate = new PackagesListDelegate;

    m_appsView->setObjectName("AppsView");
    m_infoArea->setObjectName("InfoArea");
    m_infoControlButton->setObjectName("InfoControlButton");

    m_appsView->setModel(model);
    m_appsView->setFixedHeight(213);
    m_appsView->setItemDelegate(delegate);

    m_installButton->setText(tr("Install"));
    m_acceptButton->setText(tr("Done"));
    m_acceptButton->setVisible(false);
    m_backButton->setText(tr("Back"));
    m_backButton->setVisible(false);

    m_infoArea->setReadOnly(true);
    m_infoArea->setVisible(false);
    m_infoArea->setAcceptDrops(false);
    m_infoArea->setFixedHeight(186);
    m_infoArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_infoControlButton->setVisible(false);
    m_installProgress->setVisible(false);

    QHBoxLayout *btnsLayout = new QHBoxLayout;
    btnsLayout->addStretch();
    btnsLayout->addWidget(m_installButton);
    btnsLayout->addWidget(m_backButton);
    btnsLayout->addWidget(m_acceptButton);
    btnsLayout->setSpacing(10);
    btnsLayout->addStretch();
    btnsLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_appsView);
    centralLayout->addWidget(m_infoControlButton);
    centralLayout->setAlignment(m_infoControlButton, Qt::AlignHCenter);
    centralLayout->addWidget(m_infoArea);
    centralLayout->addStretch();
    centralLayout->addWidget(m_installProgress);
    centralLayout->setAlignment(m_installProgress, Qt::AlignHCenter);
    centralLayout->addLayout(btnsLayout);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(20, 0, 20, 30);
    setLayout(centralLayout);

    connect(m_infoControlButton, &InfoControlButton::expand, this, &MultipleInstallPage::showInfo);
    connect(m_infoControlButton, &InfoControlButton::shrink, this, &MultipleInstallPage::hideInfo);
    connect(m_installButton, &QPushButton::clicked, m_debListModel, &DebListModel::installAll);
    connect(m_backButton, &QPushButton::clicked, this, &MultipleInstallPage::back);
    connect(m_acceptButton, &QPushButton::clicked, qApp, &QApplication::quit);

    connect(m_appsView, &PackagesListView::clicked, this, &MultipleInstallPage::onItemClicked);
    connect(m_appsView, &PackagesListView::entered, m_debListModel, &DebListModel::setCurrentIndex);

    connect(model, &DebListModel::workerProgressChanged, this, &MultipleInstallPage::onProgressChanged);
    connect(model, &DebListModel::appendOutputInfo, this, &MultipleInstallPage::onOutputAvailable);
}

void MultipleInstallPage::onWorkerFinshed() {
    m_acceptButton->setVisible(true);
    //    m_backButton->setVisible(true);//注释 by psy
    m_installProgress->setVisible(false);
}

void MultipleInstallPage::onOutputAvailable(const QString &output) {
    m_infoArea->append(output.trimmed());

    // change to install
    if (m_installButton->isVisible()) {
        m_installButton->setVisible(false);

        m_installProgress->setVisible(true);
        m_infoControlButton->setVisible(true);
    }
}

void MultipleInstallPage::onProgressChanged(const int progress) {
    m_progressAnimation->setStartValue(m_installProgress->value());
    m_progressAnimation->setEndValue(progress);
    m_progressAnimation->start();

    // finished
    if (progress == 100) {
        onOutputAvailable(QString());
        QTimer::singleShot(m_progressAnimation->duration(), this, &MultipleInstallPage::onWorkerFinshed);
    }
}

void MultipleInstallPage::onItemClicked(const QModelIndex &index) {
    if (!m_debListModel->isWorkerPrepare()) return;

    const int r = index.row();

    emit requestRemovePackage(r);
}

void MultipleInstallPage::showInfo() {
    m_appsView->setVisible(false);
    m_infoArea->setVisible(true);
}

void MultipleInstallPage::hideInfo() {
    m_appsView->setVisible(true);
    m_infoArea->setVisible(false);
}
