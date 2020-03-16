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

#ifndef SINGLEINSTALLPAGE_H
#define SINGLEINSTALLPAGE_H

#include "infocontrolbutton.h"
#include "installprocessinfoview.h"
#include "debinfolabel.h"

#include <DLabel>
#include <DProgressBar>
#include <DPushButton>
#include <DTextEdit>
#include <QWidget>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <DDialog>
#include <QApt/DebFile>
#include <unistd.h>
#include <stdlib.h>
#include <QProcess>

class DebListModel;
class WorkerProgress;
class SingleInstallPage : public QWidget
{
    Q_OBJECT

public:
    explicit SingleInstallPage(DebListModel *model, QWidget *parent = nullptr);
    void afterGetAutherFalse();
protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event) override;
signals:
    void back() const;
    void requestUninstallConfirm() const;

public slots:
    void uninstallCurrentPackage();

private:
    enum Operate {
        Install,
        Uninstall,
        Reinstall
    };

private:
    void initUI();
    void initContentLayout();
    void initPkgInfoView(int fontinfosize);
    void initPkgInstallProcessView(int fontinfosize);
    void initConnections();
    int initLabelWidth(int fontinfo);
    void setPackageInfo();



private slots:
    void install();
    void reinstall();

    void showInfomation();
    void hideInfomation();

    void showInfo();
    void onOutputAvailable(const QString &output);
    void onWorkerFinished();
    void onWorkerProgressChanged(const int progress);

private:
    int m_operate;
    bool m_workerStarted;
    bool m_upDown;
    DebListModel *m_packagesModel;
    QWidget *m_contentFrame;
    QWidget *m_itemInfoFrame;
    DLabel *m_packageIcon;
    DebInfoLabel *m_packageName;
    DebInfoLabel *m_packageVersion;
    DLabel *m_packageDescription;
    DebInfoLabel *m_tipsLabel;
    QWidget *m_progressFrame;
    WorkerProgress *m_progress;
    InstallProcessInfoView *m_installProcessView;
    InfoControlButton *m_infoControlButton;
    DPushButton *m_installButton;
    DPushButton *m_uninstallButton;
    DPushButton *m_reinstallButton;
    DPushButton *m_confirmButton;
    DPushButton *m_backButton;
    DPushButton *m_doneButton;
    QVBoxLayout *m_contentLayout;
    QVBoxLayout *m_centralLayout;
    QString m_description;
    QString packagename_description;
    int flag = 0;

};

#endif  // SINGLEINSTALLPAGE_H
