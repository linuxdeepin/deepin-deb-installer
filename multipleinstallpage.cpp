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
#include "workerprogress.h"
#include "utils.h"

#include <QApplication>
#include <QPropertyAnimation>
#include <QTimer>
#include <QVBoxLayout>

#include <DLabel>
#include <DTitlebar>

MultipleInstallPage::MultipleInstallPage(DebListModel *model, DWidget *parent)
    : DWidget(parent)
    , m_debListModel(model)
    , m_contentFrame(new DFrame(this))
    , m_appsListView(new PackagesListView)
    , m_appsListViewBgFrame(new DRoundBgFrame(this))
    , m_installProcessInfoView(new InstallProcessInfoView)
    , m_infoControlButton(new InfoControlButton(tr("Display install details"), tr("Collapse")))
    , m_installProgress(new WorkerProgress)
    , m_progressAnimation(new QPropertyAnimation(m_installProgress, "value", this))
    , m_installButton(new DPushButton)
    , m_acceptButton(new DPushButton)
    , m_backButton(new DPushButton)
    , m_contentLayout(new QVBoxLayout)
    , m_centralLayout(new QVBoxLayout)
{
    initContentLayout();
    initUI();
    initConnections();
}

void MultipleInstallPage::initContentLayout()
{
    m_contentLayout->setSpacing(0);
    m_contentLayout->setContentsMargins(10, 13, 10, 30);
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

void MultipleInstallPage::initUI()
{
    PackagesListDelegate *delegate = new PackagesListDelegate(m_appsListView);

    QString mediumFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansMedium);
    QFont btnFont = Utils::loadFontBySizeAndWeight(mediumFontFamily, 14, QFont::Medium);

    m_appsListViewBgFrame->setFixedSize(460, 186);
    QHBoxLayout *appsViewLayout = new QHBoxLayout;
    appsViewLayout->setSpacing(0);
    appsViewLayout->setContentsMargins(0, 0, 0, 0);
    m_appsListViewBgFrame->setLayout(appsViewLayout);

    m_appsListView->setModel(m_debListModel);
    m_appsListView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);;
    m_appsListView->setItemDelegate(delegate);
    appsViewLayout->addWidget(m_appsListView);

    m_installButton->setFixedSize(120, 36);
    m_acceptButton->setFixedSize(120, 36);
    m_backButton->setFixedSize(120, 36);

    m_installButton->setText(tr("Install"));
    m_acceptButton->setText(tr("Done"));
    m_acceptButton->setVisible(false);
    m_backButton->setText(tr("Back"));
    m_backButton->setVisible(false);

    m_installButton->setFont(btnFont);
    m_acceptButton->setFont(btnFont);
    m_backButton->setFont(btnFont);

    m_installButton->setFocusPolicy(Qt::NoFocus);
    m_acceptButton->setFocusPolicy(Qt::NoFocus);
    m_backButton->setFocusPolicy(Qt::NoFocus);

    m_installProcessInfoView->setVisible(false);
    m_installProcessInfoView->setAcceptDrops(false);
    m_installProcessInfoView->setFixedHeight(200);
    m_installProcessInfoView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

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

    m_contentLayout->addWidget(m_appsListViewBgFrame, Qt::AlignHCenter);
    m_contentLayout->addWidget(m_infoControlButton);
    m_contentLayout->setAlignment(m_infoControlButton, Qt::AlignHCenter);
    m_contentLayout->addWidget(m_installProcessInfoView);
    m_contentLayout->addSpacing(38);
    m_contentLayout->addWidget(m_installProgress);
    m_contentLayout->addStretch();
    m_contentLayout->setAlignment(m_installProgress, Qt::AlignHCenter);
    m_contentLayout->addLayout(btnsLayout);
}

void MultipleInstallPage::initConnections()
{
    connect(m_infoControlButton, &InfoControlButton::expand, this, &MultipleInstallPage::showInfo);
    connect(m_infoControlButton, &InfoControlButton::shrink, this, &MultipleInstallPage::hideInfo);
    connect(m_installButton, &DPushButton::clicked, m_debListModel, &DebListModel::installAll);
    connect(m_installButton, &DPushButton::clicked, this, &MultipleInstallPage::hiddenCancelButton);
    connect(m_backButton, &DPushButton::clicked, this, &MultipleInstallPage::back);
    connect(m_acceptButton, &DPushButton::clicked, qApp, &QApplication::quit);

    connect(m_appsListView, &PackagesListView::clicked, this, &MultipleInstallPage::onItemClicked);
    connect(m_appsListView, &PackagesListView::entered, m_debListModel, &DebListModel::setCurrentIndex);

    connect(m_debListModel, &DebListModel::workerProgressChanged, this, &MultipleInstallPage::onProgressChanged);
    connect(m_debListModel, &DebListModel::appendOutputInfo, this, &MultipleInstallPage::onOutputAvailable);

    connect(m_debListModel, &DebListModel::onChangeOperateIndex, this, &MultipleInstallPage::onAutoScrollInstallList);
}

void MultipleInstallPage::onWorkerFinshed()
{
    m_acceptButton->setVisible(true);
    m_backButton->setVisible(true);
    m_installProgress->setVisible(false);
}

void MultipleInstallPage::onOutputAvailable(const QString &output)
{
    m_installProcessInfoView->appendText(output.trimmed());

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
        m_appsListView->scrollTo(currIndex, QAbstractItemView::PositionAtTop);
    }
    else if(opIndex == -1)//to top
    {
        QModelIndex currIndex = m_debListModel->index(0);
        m_appsListView->scrollTo(currIndex);
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
    m_contentLayout->setContentsMargins(10, 0, 10, 30);
    m_appsListViewBgFrame->setVisible(false);
    m_appsListView->setVisible(false);
    m_installProcessInfoView->setVisible(true);
    emit hideAutoBarTitle();
}

void MultipleInstallPage::hideInfo()
{
    m_contentLayout->setContentsMargins(10, 13, 10, 30);
    m_appsListViewBgFrame->setVisible(true);
    m_appsListView->setVisible(true);
    m_installProcessInfoView->setVisible(false);
    emit hideAutoBarTitle();
}
void MultipleInstallPage::hiddenCancelButton()
{
     m_backButton->setVisible(false);
     m_installButton->setVisible(false);
}
void MultipleInstallPage::afterGetAutherFalse()
{
    m_backButton->setVisible(true);
    m_installButton->setVisible(true);
}
