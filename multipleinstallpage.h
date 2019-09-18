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

#include <DWidget>

#include <DPushButton>
#include <DProgressBar>
#include <DTextEdit>
#include <QPropertyAnimation>

class PackagesListView;
class DebListModel;
class MultipleInstallPage : public DWidget
{
    Q_OBJECT

public:
    explicit MultipleInstallPage(DebListModel *model, DWidget *parent = 0);

    void afterGetAutherFalse();
signals:
    void back() const;
    void requestRemovePackage(const int index) const;
    void hideAutoBarTitle();

private slots:
    void onWorkerFinshed();
    void onOutputAvailable(const QString &output);
    void onProgressChanged(const int progress);
    void onItemClicked(const QModelIndex &index);

    void showInfo();
    void hideInfo();

    void onAutoScrollInstallList(int opIndex);
    void hiddenCancelButton();

private:
    DebListModel *m_debListModel;
    PackagesListView *m_appsView;
    QTextEdit *m_infoArea;
    InfoControlButton *m_infoControlButton;
    DProgressBar *m_installProgress;
    QPropertyAnimation *m_progressAnimation;
    DPushButton *m_installButton;
    DPushButton *m_acceptButton;
    DPushButton *m_backButton;

};

#endif // MULTIPLEINSTALLPAGE_H
