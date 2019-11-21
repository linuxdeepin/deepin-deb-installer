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
#include <DTextEdit>

class PackagesListView;
class DebListModel;
class MultipleInstallPage : public QWidget
{
    Q_OBJECT

public:
    explicit MultipleInstallPage(DebListModel *model, QWidget *parent = nullptr);

    void afterGetAutherFalse();
signals:
    void back() const;
    void requestRemovePackage(const int index) const;
    void hideAutoBarTitle();

private slots:
    void onWorkerFinshed();
    void onOutputAvailable(const QString &output);
    void onProgressChanged(const int progress);
    void onRequestRemoveItemClicked(const QModelIndex &index);

    void showInfo();
    void hideInfo();

    void onAutoScrollInstallList(int opIndex);
    void hiddenCancelButton();

private:
    void initContentLayout();
    void initUI();
    void initConnections();

    DebListModel *m_debListModel;
    QWidget *m_contentFrame;
    PackagesListView *m_appsListView;
    DRoundBgFrame *m_appsListViewBgFrame;
    InstallProcessInfoView *m_installProcessInfoView;
    InfoControlButton *m_infoControlButton;
    QWidget *m_processFrame;
    DProgressBar *m_installProgress;
    QPropertyAnimation *m_progressAnimation;
    DPushButton *m_installButton;
    DPushButton *m_acceptButton;
    DPushButton *m_backButton;

    QVBoxLayout *m_contentLayout;
    QVBoxLayout *m_centralLayout;
};

#endif // MULTIPLEINSTALLPAGE_H
