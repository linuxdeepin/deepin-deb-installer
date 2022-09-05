// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingdialog.h"
#include <DSettingsWidgetFactory>
#include <DStandardPaths>
#include <DCheckBox>

#include <QDir>
#include <qsettingbackend.h>
#include <QDebug>
#include <QTextStream>
#include <QSettings>

SettingDialog::SettingDialog(QWidget *parent)
    : DSettingsDialog(parent)
{
    init();
}

SettingDialog::~SettingDialog()
{
    delete m_setting;
}

void SettingDialog::init()
{
    m_setting = DSettings::fromJsonFile(":/data/deepin-deb-installer.json");
    setFocus(Qt::PopupFocusReason);
    const QString confDir = DStandardPaths::writableLocation(QStandardPaths::AppConfigLocation); // 换了枚举值，待验证

    const QString confPath = confDir + QDir::separator() + "deepin-deb-installer.conf";
    QDir dir;
    bool isexist = dir.exists(confDir);
    if (!isexist)
        dir.mkpath(confDir);
    // 创建设置项存储后端
    auto backend = new QSettingBackend(confPath, this);
    m_setting->setBackend(backend);
    updateSettings(m_setting);

    QString key = "basic.develop_digital_verify.";
    QString bValue = m_setting->option(key)->value().toString();

    QSettings setting(confPath, QSettings::IniFormat);
    setting.setValue(key + "/" + "value", bValue);
    setting.sync();

    connect(m_setting, &DSettings::valueChanged, this, [=] {
        m_isDigital = m_setting->value("basic.develop_digital_verify.").toBool();
    });
    m_isDigital = m_setting->value("basic.develop_digital_verify.").toBool();

}

bool SettingDialog::isDigitalVerified()
{
    return m_setting->value("basic.develop_digital_verify.").toBool();
}
