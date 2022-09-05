// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

signals:

public slots:
private:
    DSettings *m_setting;
    bool m_isDigital = false;
};

#endif // SETTINGDIALOG_H
