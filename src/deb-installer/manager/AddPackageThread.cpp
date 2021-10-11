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


#include "AddPackageThread.h"
#include "packagesmanager.h"

#include <QApt/Backend>
#include <QApt/DebFile>

#include <DRecentManager>

#include <QDir>
#include <QStorageInfo>

DCORE_USE_NAMESPACE

using namespace QApt;

AddPackageThread::AddPackageThread(QSet<QByteArray> appendedPackagesMd5)
    : m_appendedPackagesMd5(appendedPackagesMd5)
{
}

void AddPackageThread::setPackages(QStringList packages, int validPkgCount)
{
    m_packages.clear();
    m_packages.append(packages);
    m_validPackageCount = validPkgCount;
}

void AddPackageThread::setAppendPackagesMd5(QSet<QByteArray> appendedPackagesMd5)
{
    m_appendedPackagesMd5 = appendedPackagesMd5;
}

void AddPackageThread::setSamePackageMd5(QMap<QString, QByteArray> packagesMd5)
{
    m_allPackages = packagesMd5;
}

bool AddPackageThread::dealInvalidPackage(QString packagePath)
{
    //获取路径信息
    QStorageInfo info(packagePath);

    //判断路径信息是不是本地路径
    if (!info.device().startsWith("/dev/")) {
        emit signalNotLocalPackage();
        return false;
    }
    return true;
}

QString AddPackageThread::dealPackagePath(QString packagePath)
{
    //判断当前文件路径是否是绝对路径，不是的话转换为绝对路径
    if (!packagePath.startsWith("/")) {
        QFileInfo packageAbsolutePath(packagePath);
        //获取绝对路径
        packagePath = packageAbsolutePath.absoluteFilePath();
    }

    // 判断当前文件路径中是否存在空格,如果存在则创建软链接并在之后的安装时使用软链接进行访问.
    if (packagePath.contains(" ")) {
        QApt::DebFile p(packagePath);
        if (p.isValid()) {
            packagePath = SymbolicLink(packagePath, p.packageName());
            qWarning() << "PackagesManager:"
                       << "There are spaces in the path, add a soft link" << packagePath;
        }
    }
    return packagePath;
}

void AddPackageThread::run()
{
    for (QString debPackage : m_packages) {

        // 处理包不在本地的情况。
        if (!dealInvalidPackage(debPackage)) {
            continue;
        }
        QString debPkg = debPackage;
        debPackage = dealPackagePath(debPackage);

        QApt::DebFile pkgFile(debPackage);
        //判断当前文件是否是无效文件
        if (!pkgFile.isValid()) {
            // 根据文件无效的类型提示不同的文案
            emit signalInvalidPackage();
            continue;
        }
        // 获取当前文件的md5的值,防止重复添加
        QByteArray md5;
        //先查看之前检测包有效性时是否获取过md5
        md5 = m_allPackages.value(debPkg);
        if(md5.isEmpty())
             md5 = pkgFile.md5Sum();

        // 如果当前已经存在此md5的包,则说明此包已经添加到程序中
        if (m_appendedPackagesMd5.contains(md5)) {
            //处理重复文件
            emit signalPackageAlreadyExists();
            continue;
        }
        //管理最近文件列表
        DRecentData data;
        data.appName = "Deepin Deb Installer";
        data.appExec = "deepin-deb-installer";
        DRecentManager::addItem(debPackage, data);

        // 添加到set中，用来判断重复
        m_appendedPackagesMd5 << md5;

        // 可以添加,发送添加信号
        emit signalAddPackageToInstaller(m_validPackageCount, debPackage, md5);
    }

    emit signalAppendFinished();
}

QString AddPackageThread::SymbolicLink(QString previousName, QString packageName)
{
    //如果创建临时目录失败,则提示
    if (!mkTempDir()) {
        qWarning() << "AddPackageThread:" << "Failed to create temporary folder";
        return previousName;
    }
    //成功则开始创建
    return link(previousName, packageName);
}

bool AddPackageThread::mkTempDir()
{
    QDir tempPath(m_tempLinkDir);

    if (!tempPath.exists()) {
        //如果临时目录不存在则返回创建结果
        return tempPath.mkdir(m_tempLinkDir);
    } else {
        //临时目录已经存在,直接返回创建成功
        return true;
    }
}

QString AddPackageThread::link(QString linkPath, QString packageName)
{
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
            qWarning() << "AddPackageThread:" << "A file with the same name exists in the current temporary directory,"
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
        qWarning() << "AddPackageThread:" << "Failed to create Symbolick link error.";
        return linkPath;
    }
}
