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

#include <DLabel>
#include <DProgressBar>
#include <DPushButton>
#include <DTextEdit>
#include <DWidget>

#include <QApt/DebFile>

class DebListModel;
class SingleInstallPage : public DWidget
{
    Q_OBJECT

public:
    explicit SingleInstallPage(DebListModel *model, DWidget *parent = 0);

signals:
    void back() const;
    void requestUninstallConfirm() const;

public slots:
    void uninstallCurrentPackage();

private:
    enum Operate {
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
    DWidget *m_itemInfoWidget;
    DLabel *m_packageIcon;
    DLabel *m_packageName;
    DLabel *m_packageVersion;
    DLabel *m_packageDescription;
    DLabel *m_tipsLabel;
    DProgressBar *m_progress;
    DTextEdit *m_workerInfomation;
    DWidget *m_strengthWidget;
    InfoControlButton *m_infoControlButton;
    DPushButton *m_installButton;
    DPushButton *m_uninstallButton;
    DPushButton *m_reinstallButton;
    DPushButton *m_confirmButton;
    DPushButton *m_backButton;
    DPushButton *m_doneButton;

};

#endif  // SINGLEINSTALLPAGE_H
