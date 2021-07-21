/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      zhangkai <zhangkai@uniontech.com>
* Maintainer:  zhangkai <zhangkai@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <DSettingsDialog>
#include <DSettings>
#include <DSettingsOption>
#include <DWidget>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class SettingDialog : public DSettingsDialog
{
    Q_OBJECT
public:
    explicit SettingDialog(QWidget *parent = nullptr);
    ~SettingDialog();
    void init();
    bool isDigitalVerified();
    void setCheckboxEnable(bool isDevelopMode);
    //    DWidget *widget();

signals:

public slots:
private:
    DSettings *m_setting;
    bool m_isDigital = false;
};

#endif // SETTINGDIALOG_H
