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

#ifndef ADDPACKAGETHREAD_H
#define ADDPACKAGETHREAD_H

#include <QObject>
#include <QThread>
#include <QSet>
#include <QByteArray>

class PackagesManager;

/**
 * @brief The AddPackageThread class
 * 批量添加,部分加载
 */
class AddPackageThread : public QThread
{
    Q_OBJECT
public:
    AddPackageThread(QSet<QByteArray> appendedPackagesMd5);

    void run();

    /**
     * @brief setPackages 获取需要安装的软件包的列表
     * @param packages 软件包列表
     */
    void setPackages(QStringList packages);


    /**
     * @brief setAppendPackagesMd5 获取到已经添加到应用的deb包的Md5的set
     * @param appendPackagesMd5 目前应用中的deb包的Md5的set
     */
    void setAppendPackagesMd5(QSet<QByteArray> appendedPackagesMd5);

signals:

    /**
     * @brief addedPackage 当前包可以被添加到应用中
     */
    void addedPackage(int, QString, QByteArray);

    /**
     * @brief appendFinished 批量添加结束
     * @param 是否需要下载wine依赖的标识
     */
    void appendFinished();

signals:
    /**
     * @brief invalidPackage 当前包是无效的包
     */
    void invalidPackage();

    /**
     * @brief packageAlreadyExists 当前包已经被添加到应用中
     */
    void packageAlreadyExists();

signals:
    /**
     * @brief refreshSinglePage 刷新单包界面
     */
    void refreshSinglePage();

    /**
     * @brief refreshMultiPage 刷新批量安装界面
     */
    void refreshMultiPage();

    /**
     * @brief single2MultiPage 从单包安装界面刷新为批量安装界面
     */
    void single2MultiPage();

private:
    QStringList m_packages;                     //要添加的软件包列表
    QSet<QByteArray> m_appendedPackagesMd5;     //已经添加的deb包的MD5值的集合

private:
    /**
     * @brief SymbolicLink 为路径中存在空格的包创建临时文件夹以及软链接
     * @param previousName 存在空格的路径ruanji
     * @param packageName  当前包的包名
     * @return 创建成功后软链接的全路径
     */
    QString SymbolicLink(QString previousName, QString packageName);

    /**
     * @brief link          创建软链接
     * @param linkPath      原路径
     * @param packageName   包名
     * @return  创建软链接之后的路径
     */
    QString link(QString linkPath, QString packageName);

    /**
     * @brief mkTempDir 创建存放软链接的临时路径
     * @return 是否创建成功
     */
    bool mkTempDir();

private:
    const QString m_tempLinkDir = "/tmp/LinkTemp/";             // 软链接的存放路径

private:

    /**
     * @brief checkInvalid 检查有效文件的数量
     */
    void checkInvalid();

    int m_validPackageCount = 0;        //有效文件的数量

    qint64 md5SumTotalTime = 0;         //性能测试临时变量，获取md5Sum总耗时
};

#endif // ADDPACKAGETHREAD_H
