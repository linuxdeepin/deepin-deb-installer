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
