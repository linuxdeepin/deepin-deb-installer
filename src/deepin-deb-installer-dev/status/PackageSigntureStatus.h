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

#ifndef PackageSigntureStatus_H
#define PackageSigntureStatus_H

#include <QObject>
#include <QProcess>

#define deb_verify_tool "/usr/bin/deepin-deb-verify"

enum SigntureStatus {
    SigntureUnknown = -1,     //其他错误
    SigntureVerifySuccess,       //验证成功
    SigntureInexistence,  //解压deb文件用的临时目录不存在
    SigntureExtractFail,      //提取deb包内容时出错
    SigntureVerifyFail        //deb包验证失败
};

class PackageSigntureStatus
{

public:

    PackageSigntureStatus();

    ~PackageSigntureStatus();

    /**
     * @brief checkPackageSignture 检查包的签名状态
     * @param packagePath 包的路径
     * @return
     */
    SigntureStatus checkPackageSignture(QString packagePath);

private:

    /**
     * @brief checkDigitalVerifyTools 检查当前系统中是否存在验证工具
     * @return true: 存在签名验证工具   false:签名验证工具不存在
     */
    bool checkDigitalVerifyTools();

    /**
     * @brief checkDeviceLock 检查当前是否开启开发者模式
     * @return true: 已开启开发者模式   false: 未开启开发者模式
     */
    bool checkDeviceLock();

private:
    QProcess *m_pCheckSignProc = nullptr;
};

#endif // PackageSigntureStatus_H
