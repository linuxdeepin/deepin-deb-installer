// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PACKAGESELECTVIEW_H
#define PACKAGESELECTVIEW_H

#include "model/packageselectmodel.h"

#include <QWidget>

class QListWidget;
class QCheckBox;
class QEvent;
class QPushButton;
class PackageSelectItem;
class DRoundBgFrame;

class PackageSelectView : public QWidget
{
    Q_OBJECT
public:
    explicit PackageSelectView(QWidget *parent = nullptr);

    void flushDebList(const QList<DebIr> &selectInfos);
    void checkSelect();
    void selectAll(bool select);
    void setHaveMustInstallDeb(bool have);

signals:
    void packageInstallConfim(const QList<int> &selectIndexes);

public slots:
    void onInstallClicked();

protected:
    void showEvent(QShowEvent *e) override;
    void changeEvent(QEvent *event) override;

private:
    void clearDebList();
    void updateItemHeights();

    DRoundBgFrame *listBgFrame;
    QListWidget *packageListWidget;
    QCheckBox *selectAllBox;
    QPushButton *installButton;

    QList<PackageSelectItem *> items;
    bool haveMustInstallDeb = false;
};

#endif // PACKAGESELECTVIEW_H
