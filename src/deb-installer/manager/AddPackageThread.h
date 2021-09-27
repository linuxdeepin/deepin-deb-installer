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


#ifndef ADDPACKAGETHREAD_H
#define ADDPACKAGETHREAD_H

#include <QObject>
#include <QThread>
#include <QSet>
#include <QMap>
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
     * @brief sigAddPackageToInstaller 把包添加到安装器中
     */
    void signalAddPackageToInstaller(int, QString, QByteArray);

    /**
     * @brief appendFinished 批量添加结束
     * @param 是否需要下载wine依赖的标识
     */
    void signalAppendFinished();

signals:
    /**
     * @brief invalidPackage 当前包是无效的包
     */
    void signalInvalidPackage();

    /**
     * @brief notLocalPackage 包不在本地的信号
     *
     * ps: 包不在本地无法安装
     */
    void signalNotLocalPackage();

    /**
     * @brief packageAlreadyExists 当前包已经被添加到应用中
     */
    void signalPackageAlreadyExists();

signals:
    /**
     * @brief refreshSinglePage 刷新单包界面
     */
    void signalRefreshSinglePage();

    /**
     * @brief refreshMultiPage 刷新批量安装界面
     */
    void signalRefreshMultiPage();

    /**
     * @brief single2MultiPage 从单包安装界面刷新为批量安装界面
     */
    void signalSingleToMultiPage();

private:
    //要添加的软件包列表
    QStringList m_packages                  = {};

    //已经添加的deb包的MD5值的集合
    QSet<QByteArray> m_appendedPackagesMd5  = {};

    QMap<QString, QByteArray> m_allPackages = {};

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

    /**
     * @brief dealInvalidPackage 查看包是否有效
     * @param packagePath 包的路径
     * @return 包的有效性
     *   true   : 文件能打开
     *   fasle  : 文件不在本地或无权限
     */
    bool dealInvalidPackage(QString packagePath);

    /**
     * @brief dealPackagePath 处理包的路径
     * @param packagePath 包的路径
     * @return 经过处理后的包的路径
     * 处理两种情况
     *      1： 相对路径             --------> 转化为绝对路径
     *      2： 包的路径中存在空格     --------> 使用软链接，链接到/tmp下
     */
    QString dealPackagePath(QString packagePath);

private:
    // 软链接的存放路径
    const QString m_tempLinkDir = "/tmp/LinkTemp/";

private:

    /**
     * @brief checkInvalid 检查有效文件的数量
     */
    void checkInvalid();

    //有效文件的数量
    int m_validPackageCount = 0;
};

#endif // ADDPACKAGETHREAD_H
