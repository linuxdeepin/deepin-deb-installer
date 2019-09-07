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

#ifndef UNINSTALLCONFIRMPAGE_H
#define UNINSTALLCONFIRMPAGE_H

#include "infocontrolbutton.h"

#include <DLabel>
#include <DPushButton>
#include <DTextEdit>
#include <DWidget>

class UninstallConfirmPage : public DWidget {
    Q_OBJECT

public:
    explicit UninstallConfirmPage(DWidget *parent = 0);

    void setPackage(const QString &name);
    void setRequiredList(const QStringList &requiredList);

signals:
    void accepted() const;
    void canceled() const;

private slots:
    void showDetail();
    void hideDetail();

private:
    DLabel *m_icon;
    DLabel *m_tips;
    DWidget *m_infoWrapperWidget;
    InfoControlButton *m_infoControl;
    DTextEdit *m_dependsInfomation;
    DPushButton *m_cancelBtn;
    DPushButton *m_confirmBtn;
};
#endif  // UNINSTALLCONFIRMPAGE_H
