// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "error_notify_dialog_helper.h"
#include "utils/ddlog.h"

#include <QStyle>
#include <QLabel>
#include <QPushButton>

#include <DDialog>

#include "utils/hierarchicalverify.h"

DWIDGET_USE_NAMESPACE

static const int kDefaultDialogWidth = 380;

/**
 * @class ErrorNotifyDialogHelper
 * @brief A helper class to show the error notify dialog.
 */
ErrorNotifyDialogHelper::ErrorNotifyDialogHelper(QObject *parent)
    : QObject{parent}
{
    qCDebug(appLog) << "ErrorNotifyDialogHelper constructed";
}

void ErrorNotifyDialogHelper::showHierarchicalVerifyWindow()
{
    qCDebug(appLog) << "Checking hierarchical verify status";
    if (!HierarchicalVerify::instance()->isValid()) {
        qCDebug(appLog) << "Hierarchical verify not valid, skipping dialog";
        return;
    }
    qCDebug(appLog) << "Creating error notify dialog";

    DDialog *dialog = new DDialog();
    // limit the display width
    dialog->setFixedWidth(kDefaultDialogWidth);
    dialog->setFocusPolicy(Qt::TabFocus);
    dialog->setModal(true);
    dialog->setWindowFlag(Qt::WindowStaysOnTopHint);

    // set the information displayed by the pop-up window
    dialog->setTitle(QObject::tr("Unable to install"));
    dialog->setMessage(
        QObject::tr("This package does not have a valid digital signature and has been blocked from installing/running. "
                    "Go to Security Center > Tools > App Security to change the settings."));
    dialog->setIcon(QIcon::fromTheme("di_popwarning"));
    dialog->addButton(QObject::tr("Cancel", "button"), false, DDialog::ButtonNormal);
    dialog->addButton(QObject::tr("Proceed", "button"), true, DDialog::ButtonRecommend);
    dialog->show();
    qCDebug(appLog) << "Error notify dialog shown";

    // Copy from ddialog.cpp, used to set the default dialog height.
    // Avoid incomplete display at large font size/high zoom ratio, called after show().
    auto *msgLabel = dialog->findChild<QLabel *>("MessageLabel");
    if (msgLabel) {
        qCDebug(appLog) << "Setting message label minimum height";
        auto *dialogStyle = dialog->style();
        if (dialogStyle) {
            qCDebug(appLog) << "Getting dialog style";
            const QSize sz =
                dialogStyle->itemTextRect(msgLabel->fontMetrics(), msgLabel->rect(), Qt::TextWordWrap, false, msgLabel->text())
                    .size();
            msgLabel->setMinimumHeight(qMax(sz.height(), msgLabel->sizeHint().height()));
        }
    }

    auto *btnPorceed = qobject_cast<QPushButton *>(dialog->getButton(1));
    if (btnPorceed) {
        qCDebug(appLog) << "Setting focus policy for proceed button";
        btnPorceed->setFocusPolicy(Qt::TabFocus);
        btnPorceed->setFocus();
    }

    connect(dialog, &DDialog::finished, [dialog](int result) {
        qCDebug(appLog) << "Dialog finished with result:" << result;
        if (QDialog::Accepted == result) {
            qCDebug(appLog) << "Proceeding to security center";
            HierarchicalVerify::instance()->proceedDefenderSafetyPage();
        }
        dialog->deleteLater();
    });
}
