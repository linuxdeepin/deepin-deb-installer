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

#ifndef FILECHOOSEWIDGET_H
#define FILECHOOSEWIDGET_H

#include <DLabel>
#include <DPushButton>

#include <QSettings>
#include <QWidget>

class ChooseFileButton;

DWIDGET_USE_NAMESPACE

class FileChooseWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileChooseWidget(QWidget *parent = nullptr);
    bool eventFilter(QObject *watched, QEvent *event) override;
    void setChooseBtnFocus();

signals:
    void packagesSelected(const QStringList files) const;
    void OutOfFocus(bool) const;

private slots:
    void chooseFiles();
    void themeChanged();
private:
    ChooseFileButton *m_chooseFileBtn {nullptr};
    QSettings m_settings;
    DLabel *split_line {nullptr};
    DLabel *m_dndTips {nullptr};
    DLabel *m_iconImage {nullptr};
    int m_MouseBtnRelease = 0;
};

#endif  // FILECHOOSEWIDGET_H
