// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingdialog.h"
#include "utils/hierarchicalverify.h"

#include <DSettingsWidgetFactory>
#include <DStandardPaths>
#include <DCheckBox>

#include <QApplication>
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
    // 设置默认的校验勾选框
    if (widgetFactory()) {
        widgetFactory()->registerWidget("verifycheckbox", &SettingDialog::createVerifyCheckBox);
    }

    m_setting = DSettings::fromJsonFile(":/data/deepin-deb-installer.json");
    setFocus(Qt::PopupFocusReason);
    const QString confDir = DStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);  // 换了枚举值，待验证

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

/**
   @return 根据传入的设置参数 \a obj 创建用于配置校验信息的勾选框，
    存在分级管控接口时，静止配置验签功能，选项置灰。
 */
QWidget *SettingDialog::createVerifyCheckBox(QObject *obj)
{
    auto translateContext = obj->property("_d_DSettingsWidgetFactory_translateContext").toByteArray();
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    auto value = option->data("text").toString();
    QString trName;
    if (translateContext.isEmpty()) {
        trName = QObject::tr(value.toStdString().c_str());
    } else {
        trName = qApp->translate(translateContext, value.toStdString().c_str());
    }
    DCheckBox *checkBox = new DCheckBox(trName);

    checkBox->setObjectName("OptionVerifyCheckbox");
    checkBox->setAccessibleName("OptionVerifyCheckbox");

    // 分级管控模式下，不允许设置安装器验签
    checkBox->setEnabled(!HierarchicalVerify::instance()->isValid());
    QObject::connect(HierarchicalVerify::instance(), &HierarchicalVerify::validChanged, checkBox, [=](bool valid) {
        checkBox->setEnabled(!valid);
    });

    checkBox->setChecked(option->value().toBool());
    option->connect(checkBox, &QCheckBox::stateChanged, option, [=](int status) { option->setValue(status == Qt::Checked); });
    option->connect(option, &DTK_CORE_NAMESPACE::DSettingsOption::valueChanged, checkBox, [=](const QVariant &value) {
        checkBox->setChecked(value.toBool());
        checkBox->update();
    });

    return checkBox;
}

/**
   @brief 处理显示事件，此处用于展示前刷新分级管控接口状态
 */
void SettingDialog::showEvent(QShowEvent *e)
{
    // 刷新分级管控接口状态
    (void)HierarchicalVerify::instance()->isValid();
    DSettingsDialog::showEvent(e);
}
