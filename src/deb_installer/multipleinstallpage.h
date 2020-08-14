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

#ifndef MULTIPLEINSTALLPAGE_H
#define MULTIPLEINSTALLPAGE_H

#include "infocontrolbutton.h"
#include "droundbgframe.h"
#include "installprocessinfoview.h"

#include <QPropertyAnimation>

#include <QWidget>
#include <DPushButton>
#include <DProgressBar>
#include <DSpinner>
#include <DCommandLinkButton>

class PackagesListView;
class DebListModel;
class WorkerProgress;
class MultipleInstallPage : public QWidget
{
    Q_OBJECT

public:
    explicit MultipleInstallPage(DebListModel *model, QWidget *parent = nullptr);

    void setEnableButton(bool bEnable);
    void afterGetAutherFalse();

    void setScrollBottom(int index);

    void DealDependResult(int iAuthRes, QString dependName);

    void setInitSelect();

signals:
    void back() const;

    void requestRemovePackage(const int index) const;

    void hideAutoBarTitle();

private slots:
    void onScrollSlotFinshed();
    void onWorkerFinshed();

    void onOutputAvailable(const QString &output);
    void onProgressChanged(const int progress);
    void onRequestRemoveItemClicked(const QModelIndex &index);

    void showInfo();
    void hideInfo();

    void onAutoScrollInstallList(int opIndex);
    void hiddenCancelButton();

private:
    void initUI();
    void initConnections();
    void initContentLayout();

private:
    DebListModel *m_debListModel;

    DRoundBgFrame *m_appsListViewBgFrame;
    QWidget *m_contentFrame;
    QWidget *m_processFrame;
    QVBoxLayout *m_contentLayout;
    QVBoxLayout *m_centralLayout;

    PackagesListView *m_appsListView;

    InstallProcessInfoView *m_installProcessInfoView;

    WorkerProgress *m_installProgress;
    QPropertyAnimation *m_progressAnimation;

    DPushButton *m_installButton;
    DPushButton *m_acceptButton;
    DPushButton *m_backButton;
    InfoControlButton *m_infoControlButton;

    // fix bug:33999 change DebInfoLabel to DCommandLinkButton for Activity color
    DCommandLinkButton *m_tipsLabel;
    DSpinner *m_dSpinner;

    int m_index = -1;
    bool m_upDown = true;
};

#endif // MULTIPLEINSTALLPAGE_H
