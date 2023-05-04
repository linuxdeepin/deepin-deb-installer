// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "hierarchicalverify.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusError>
#include <QDebug>

// 分级管控DBus接口信息
const char DBUS_HIERARCHICAL_BUS[] = "com.deepin.daemon.ACL";
const char DBUS_HIERARCHICAL_PATH[] = "/org/deepin/security/hierarchical/Control";
const char DBUS_HIERARCHICAL_INTERFACE[] = "org.deepin.security.hierarchical.Control";
const char DBUS_HIERARCHICAL_METHOD[] = "Availabled";

// 匹配正则,%1为错误码
const char VERIFY_ERROR_REGEXP[] = "(deepin)+[^\\n]*(hook)+[^\\n]*%1";

/**
   @brief dpkg校验错误码，当前仅验签错误
 */
enum HierarchicalError {
    VerifyError = 65280,  ///< dpkg hook 签名校验错误码
};

/**
   @class HierarchicalVerify
   @brief 分级管控签名校验辅助类
   @details 配合【安全中心】分级管控，实现不同的安装包管理策略，进行签名验证
    1. 当分级管控接口不可用时，使用安装器校验，@sa `Utils::Digital_Verify`
    2. 当分级管控控件可用时，采用dpkg hook方式调用验签工具，错误码信息通过命令行输出，
       安装器接收输出信息并判断是否为验签错误。
 */

HierarchicalVerify::HierarchicalVerify() {}

HierarchicalVerify::~HierarchicalVerify() {}

/**
   @return 返回分级管控签名校验辅助类实例
 */
HierarchicalVerify *HierarchicalVerify::instance()
{
    static HierarchicalVerify ins;
    return &ins;
}

/**
   @return 返回当前分级管控签名验证是否可用，此信息通过查询DBus接口属性值取得。
 */
bool HierarchicalVerify::isValid()
{
    if (interfaceInvalid) {
        return false;
    }

    bool availabled = checkHierarchicalInterface();
    if (valid != availabled) {
        valid = availabled;

        // 分级验签不可用时，清理当前缓存数据
        if (!valid) {
            clearVerifyResult();
        }

        Q_EMIT validChanged(valid);
    }

    return valid;
}

/**
   @brief 检测软件包 \a pkgName 安装失败时的错误信息 \a errorString 中是否包含验签不通过的错误信息。

   @warning 通过正则表达式匹配输出，当前通过 hook 标志和错误码 65280 匹配，需注意命令行输出信息更新未正常匹配的情况
 */
bool HierarchicalVerify::checkTransactionError(const QString &pkgName, const QString &errorString)
{
    static QRegExp s_ErrorReg(QString(VERIFY_ERROR_REGEXP).arg(VerifyError));
    if (errorString.contains(s_ErrorReg)) {
        invalidPackages.insert(pkgName);
        qWarning() << QString("[Hierarchical] Package %1 detected hierarchical error!").arg(pkgName);
        return true;
    }

    return false;
}

/**
   @return 返回软件包 \a pkgName 是否通过校验，这个信息从校验缓存中获取，
    通过 `clearVerifyResult` 移除
 */
bool HierarchicalVerify::pkgVerifyPassed(const QString &pkgName)
{
    return !invalidPackages.contains(pkgName);
}

/**
   @brief 移除缓存中的校验结果
 */
void HierarchicalVerify::clearVerifyResult()
{
    invalidPackages.clear();
}

/**
   @brief 检测当前系统环境下是否包含分级管控接口，并取值判断接口是否可用。
 */
bool HierarchicalVerify::checkHierarchicalInterface()
{
    bool availabled = false;
    QDBusInterface interface(
        DBUS_HIERARCHICAL_BUS, DBUS_HIERARCHICAL_PATH, DBUS_HIERARCHICAL_INTERFACE, QDBusConnection::systemBus());

    if (interface.isValid()) {
        QDBusMessage message = interface.call(DBUS_HIERARCHICAL_METHOD);
        QDBusError error = interface.lastError();
        if (QDBusError::NoError != error.type()) {
            qWarning() << QString("[Hierarchical] DBus %1 read property %2 error: type(%2) [%3] %4")
                              .arg(DBUS_HIERARCHICAL_BUS)
                              .arg(DBUS_HIERARCHICAL_METHOD)
                              .arg(error.type())
                              .arg(error.name())
                              .arg(error.message());

            // QDBusInterface 在构造时不一定能判断接口是否有效，调用后二次判断
            if (!interface.isValid() || QDBusError::UnknownInterface == error.type() ||
                QDBusError::InvalidInterface == error.type()) {
                interfaceInvalid = true;
                qWarning() << QString("[Hierarchical] Interface %1 is not valid! Disable check hierarchical control interface.")
                                  .arg(DBUS_HIERARCHICAL_INTERFACE);
            }
        } else {
            QDBusReply<bool> reply(message);
            availabled = reply.value();

            qInfo() << QString("[Hierarchical] Get %1 property %2 value: %3")
                           .arg(DBUS_HIERARCHICAL_BUS)
                           .arg(DBUS_HIERARCHICAL_METHOD)
                           .arg(availabled);
        }
    } else {
        interfaceInvalid = true;
        qWarning() << QString("[Hierarchical] DBus interface %1 invalid! error: [%2] %3")
                          .arg(DBUS_HIERARCHICAL_INTERFACE)
                          .arg(interface.lastError().name())
                          .arg(interface.lastError().message());
    }

    return availabled;
}
