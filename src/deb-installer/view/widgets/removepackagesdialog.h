// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REMOVEPACKAGESDIALOG_H
#define REMOVEPACKAGESDIALOG_H

#include <QString>
#include <QStringList>

class QWidget;

/**
 * Shared confirmation dialog used by single and batch install pages when an
 * install plan would remove other installed packages.
 */
namespace RemovePackagesDialog {

enum Choice {
    Cancel,
    Keep,
    Uninstall
};

Choice exec(QWidget *parent, const QString &targetText, const QStringList &removePackages);

}  // namespace RemovePackagesDialog

#endif  // REMOVEPACKAGESDIALOG_H
