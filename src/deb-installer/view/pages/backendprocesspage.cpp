// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "backendprocesspage.h"
#include "view/widgets/processwidget.h"
#include "view/widgets/noprocesswidget.h"
#include "utils/ddlog.h"

#include <QStackedLayout>

BackendProcessPage::BackendProcessPage(QWidget *parent)
    : QWidget(parent)
{
    qCDebug(appLog) << "Initializing BackendProcessPage...";
    processWidget = new ProcessWidget;
    processWidget->setIcon(QIcon::fromTheme("application-x-deb"));
    processWidget->setMainText(tr("Loading packages..."));
    processWidget->setProcessText(tr("%1/%2 loaded"));

    noProcessWidget = new NoProcessWidget;
    noProcessWidget->setActionText(tr("Initializing..."));

    allLayout = new QStackedLayout;
    allLayout->addWidget(processWidget);
    allLayout->addWidget(noProcessWidget);
    setLayout(allLayout);
    qCDebug(appLog) << "BackendProcessPage initialized";
}

void BackendProcessPage::setDisplayPage(DisplayMode mode)
{
    qCDebug(appLog) << "Setting display mode:" << mode;
    if (mode == APT_INIT) {
        qCDebug(appLog) << "Showing APT initialization view";
        allLayout->setCurrentWidget(noProcessWidget);
        noProcessWidget->start();
    } else if (mode == APT_UPDATE_CACHE) {
        noProcessWidget->setActionText(tr("Updating package cache..."));
    } else if (mode == READ_PKG) {
        qCDebug(appLog) << "Showing package loading view";
        allLayout->setCurrentWidget(processWidget);
        noProcessWidget->stop();
    } else {  // PROCESS_FIN
        qCDebug(appLog) << "Process finished, hiding widgets";
        noProcessWidget->stop();
    }
}

void BackendProcessPage::setPkgProcessRate(int currentRate, int pkgCount)
{
    qCDebug(appLog) << "Updating progress:" << currentRate << "/" << pkgCount;
    processWidget->setProgress(currentRate, pkgCount);
}
