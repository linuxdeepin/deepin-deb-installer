/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef SINGLEINSTALLPAGE_H
#define SINGLEINSTALLPAGE_H

#include "infocontrolbutton.h"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QTextEdit>

#include <QApt/DebFile>

#include <dlinkbutton.h>

class DebListModel;
class SingleInstallPage : public QWidget
{
    Q_OBJECT

public:
    explicit SingleInstallPage(DebListModel *model, QWidget *parent = 0);

signals:
    void back() const;
    void requestUninstallConfirm() const;

public slots:
    void uninstallCurrentPackage();

private:
    enum Operate
    {
        Install,
        Uninstall,
    };

private:
    void setPackageInfo();

private slots:
    void install();

    void showInfomation();
    void hideInfomation();

    void showInfo();
    void onOutputAvailable(const QString &output);
    void onWorkerFinished();
    void onWorkerProgressChanged(const int progress);

private:
    int m_operate;
    bool m_workerStarted;
    DebListModel *m_packagesModel;
    QWidget *m_itemInfoWidget;
    QLabel *m_packageIcon;
    QLabel *m_packageName;
    QLabel *m_packageVersion;
    QLabel *m_packageDescription;
    QLabel *m_tipsLabel;
    QProgressBar *m_progress;
    QTextEdit *m_workerInfomation;
    QWidget *m_strengthWidget;
    InfoControlButton *m_infoControlButton;
    QPushButton *m_installButton;
    QPushButton *m_uninstallButton;
    QPushButton *m_reinstallButton;
    QPushButton *m_confirmButton;
    QPushButton *m_backButton;
};

#endif // SINGLEINSTALLPAGE_H
