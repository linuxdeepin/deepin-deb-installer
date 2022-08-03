/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
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

#include "PackageSigntureStatus.h"

#include <QFile>
#include <QtDebug>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

PackageSigntureStatus::PackageSigntureStatus()
    : m_pCheckSignProc(new QProcess)
{


}

bool PackageSigntureStatus::checkDigitalVerifyTools()
{
    QFile file(deb_verify_tool);
    return file.exists();
}

bool PackageSigntureStatus::checkDeviceLock()
{
    QDBusInterface Installer("com.deepin.deepinid", "/com/deepin/deepinid", "com.deepin.deepinid");
    return Installer.property("DeviceUnlocked").toBool();                            // 判断当前是否处于开发者模式
}

SigntureStatus PackageSigntureStatus::checkPackageSignture(QString packagePath)
{

    if (checkDeviceLock()) {
        qInfo() << "已打开开发者模式，默认签名验证通过";
        return SigntureVerifySuccess;
    }
    if (checkDigitalVerifyTools()) {
        QString program = "/usr/bin/deepin-deb-verify ";
        packagePath = "\"" + packagePath + "\"";
        program = program + packagePath;
        m_pCheckSignProc->start(program);
        m_pCheckSignProc->waitForFinished(-1);
        const QString output = m_pCheckSignProc->readAllStandardOutput();
        const QString output1 = m_pCheckSignProc->readAllStandardError();
        qInfo() << "签名校验结果：" << output1;
        for (const auto &item : output1.split('\n')) {
            if (item.toLatin1() == "[INFO] signature verified!") {
                return SigntureVerifySuccess;
            }
            if (item.toLatin1() == "cannot find signinfo in deb file") {
                return SigntureInexistence;
            }
            if (item.toLatin1() == "extract deb_file failed!") {
                return SigntureExtractFail;
            }
            if (item.toLatin1() == "verify deb file failed!") {
                return SigntureVerifyFail;
            }
        }
    }
    return SigntureUnknown;
}

PackageSigntureStatus::~PackageSigntureStatus()
{
    delete m_pCheckSignProc;
}
