/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
*
* Author:     cuizhen <cuizhen@uniontech.com>
* Maintainer:  cuizhen <cuizhen@uniontech.com>
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

#include "AddPackageThread.h"
#include "packagesmanager.h"

#include <QApt/Backend>
#include <QApt/DebFile>

#include <DRecentManager>

#include <QDir>

DCORE_USE_NAMESPACE

using namespace QApt;

AddPackageThread::AddPackageThread(QSet<QByteArray> appendedPackagesMd5)
{
    m_appendedPackagesMd5 = appendedPackagesMd5;
}

/**
 * @brief AddPackageThread::setPackages 获取需要安装的软件包的列表
 * @param packages 软件包列表
 */
void AddPackageThread::setPackages(QStringList packages)
{
    m_packages.clear();
    m_packages.append(packages);
}

/**
 * @brief AddPackageThread::setAppendPackagesMd5 获取到已经添加到应用的deb包的Md5的set
 * @param appendedPackagesMd5 目前应用中的deb包的Md5的set
 */
void AddPackageThread::setAppendPackagesMd5(QSet<QByteArray> appendedPackagesMd5)
{
    m_appendedPackagesMd5 = appendedPackagesMd5;
}

/**
 * @brief AddPackageThread::checkInvalid 检查有效文件的数量
 */
void AddPackageThread::checkInvalid()
{
    m_validPackageCount = 0; //每次添加时都清零
    for (QString package : m_packages) {
        QApt::DebFile pkgFile(package);
        if (pkgFile.isValid()) {            //只有有效文件才会计入
            m_validPackageCount ++;
        }
    }
}

/**
 * @brief AddPackageThread::run
 */
void AddPackageThread::run()
{
    QTime time;
    time.start();
    qDebug() << "[AddPackageThread]" << "[run]" << "start add packages";
    checkInvalid();     //运行之前先计算有效文件的数量
    for (QString debPackage : m_packages) {                 //通过循环添加所有的包

        //判断此次添加的包是否是绝对路径,如果是相对路径则转换为绝对路径
        if (debPackage[0] != "/") {
            QFileInfo packageAbsolutePath(debPackage);
            debPackage = packageAbsolutePath.absoluteFilePath();                           //获取绝对路径
            qInfo() << "get AbsolutePath" << packageAbsolutePath.absoluteFilePath();
        }

        //管理最近文件列表
        DRecentData data;
        data.appName = "Deepin Deb Installer";
        data.appExec = "deepin-deb-installer";
        DRecentManager::addItem(debPackage, data);

        // 判断当前文件路径中是否存在空格,如果存在则创建软链接并在之后的安装时使用软链接进行访问.
        if (debPackage.contains(" ")) {
            QApt::DebFile *p = new DebFile(debPackage);
            debPackage = SymbolicLink(debPackage, p->packageName());
            qDebug() << "PackagesManager:" << "There are spaces in the path, add a soft link" << debPackage;
            delete p;
        }

        QApt::DebFile *pkgFile = new DebFile(debPackage);

        //判断当前文件是否是无效文件
        if (pkgFile && !pkgFile->isValid()) {
            //处理无效文件
            emit invalidPackage();
            delete pkgFile;
            continue;
        }

        QTime getMD5;
        getMD5.start();
        // 获取当前文件的md5的值,防止重复添加
        const auto md5 = pkgFile->md5Sum();
        int md5Time = getMD5.elapsed();
        qInfo() << "[run]" << "获取" << pkgFile->packageName() << "的MD5 用时" << md5Time << " ms";
        md5SumTotalTime += md5Time;
        // 如果当前已经存在此md5的包,则说明此包已经添加到程序中
        if (m_appendedPackagesMd5.contains(md5)) {
            //处理重复文件
            emit packageAlreadyExists();
            delete pkgFile;
            continue;
        }
        // 可以添加,发送添加信号

        m_appendedPackagesMd5 << md5;   // 添加到set中，用来判断重复
        emit addedPackage(m_validPackageCount, debPackage, md5);
        delete pkgFile;
    }

    //所有包都添加结束.
    int totalTime = time.elapsed();
    qInfo() << "当前添加" << m_validPackageCount << "个包，获取md5Sum总用时" << md5SumTotalTime << "ms";
    qInfo() << "当前添加" << m_packages.size() << "个包， 添加总用时" << totalTime << "ms"
            << "除去获取MD5外用时" << totalTime - md5SumTotalTime << "ms";
    emit appendFinished();
}

/**
 * @brief PackagesManager::SymbolicLink 创建软连接
 * @param previousName 原始路径
 * @param packageName 软件包的包名
 * @return 软链接的路径
 */
QString AddPackageThread::SymbolicLink(QString previousName, QString packageName)
{
    if (!mkTempDir()) {//如果创建临时目录失败,则提示
        qWarning() << "PackagesManager:" << "Failed to create temporary folder";
        return previousName;
    }
    //成功则开始创建
    return link(previousName, packageName);
}

/**
 * @brief PackagesManager::mkTempDir 创建软链接存放的临时目录
 * @return 创建目录的结果
 */
bool AddPackageThread::mkTempDir()
{
    QDir tempPath(m_tempLinkDir);
    if (!tempPath.exists()) {       //如果临时目录不存在则返回创建结果
        return tempPath.mkdir(m_tempLinkDir);
    } else {
        //临时目录已经存在,直接返回创建成功
        return true;
    }
}

/**
 * @brief PackagesManager::link 创建软链接
 * @param linkPath              原文件的路径
 * @param packageName           包的packageName
 * @return                      软链接之后的路径
 */
QString AddPackageThread::link(QString linkPath, QString packageName)
{
    qDebug() << "PackagesManager: Create soft link for" << packageName;
    QFile linkDeb(linkPath);

    //创建软链接时，如果当前临时目录中存在同名文件，即同一个名字的应用，考虑到版本可能有变化，将后续添加进入的包重命名为{packageName}_i
    //删除后再次添加会在临时文件的后面添加_1,此问题不影响安装。如果有问题，后续再行修改。
    int count = 1;
    QString tempName = packageName;

    // 命名创建的软链接文件
    while (true) {
        QFile tempLinkPath(m_tempLinkDir + tempName);
        //对已经存在重名文件的处理
        if (tempLinkPath.exists()) {    //命名方式为在包名后+"_i" PS:i 为当前重复的数字,无实际意义,只是为了区别不同的包
            tempName = packageName + "_" + QString::number(count);
            qWarning() << "PackagesManager:" << "A file with the same name exists in the current temporary directory,"
                       "and the current file name is changed to"
                       << tempName;
            count++;
        } else {
            break;
        }
    }
    //创建软链接
    if (linkDeb.link(linkPath, m_tempLinkDir + tempName))
        return m_tempLinkDir + tempName;    //创建成功,返回创建的软链接的路径.
    else {
        //创建失败,直接返回路径
        qWarning() << "PackagesManager:" << "Failed to create Symbolick link error.";
        return linkPath;
    }
}
