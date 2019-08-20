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

#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>

class UninstallConfirmPage : public QWidget {
    Q_OBJECT

public:
    explicit UninstallConfirmPage(QWidget *parent = 0);

    void setPackage(const QString &name);
    void setRequiredList(const QStringList &requiredList);

signals:
    void accepted() const;
    void canceled() const;

private slots:
    void showDetail();
    void hideDetail();

private:
    QLabel *m_icon;
    QLabel *m_tips;
    QWidget *m_infoWrapperWidget;
    InfoControlButton *m_infoControl;
    QTextEdit *m_dependsInfomation;
    QPushButton *m_cancelBtn;
    QPushButton *m_confirmBtn;
};
#endif  // UNINSTALLCONFIRMPAGE_H
