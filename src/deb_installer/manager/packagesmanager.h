/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
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

#ifndef PACKAGESMANAGER_H
#define PACKAGESMANAGER_H

#include "utils/result.h"

#include <QFuture>
#include <QObject>

#include <QApt/Backend>
#include <QApt/DebFile>
#include <QThread>
#include <QProcess>

typedef Result<QString> ConflictResult;

class PackageDependsStatus;
class DebListModel;
class DealDependThread;
class AddPackageThread;

class PackagesManager : public QObject
{
    Q_OBJECT

    friend class DebListModel;

public:
    explicit PackagesManager(QObject *parent = nullptr);
    ~PackagesManager();

public slots:
    /**
     * @brief DealDependResult 处理wine依赖下载结果的槽函数
     * @param iAuthRes  下载结果
     * @param iIndex    wine应用的下标
     * @param dependName    wine依赖错误的依赖名称
     */
    void DealDependResult(int iAuthRes, int iIndex, QString dependName);

//// 依赖下载相关信号
signals:
    /**
     * @brief DependResult 处理wine依赖下载结果
     */
    void DependResult(int, int, QString);

    /**
     * @brief enableCloseButton 设置关闭按钮是否可用的信号
     */
    void enableCloseButton(bool);

////添加包相关信号
signals:
    /**
     * @brief invalidPackage 无效包的信号
     */
    void invalidPackage();

    /**
     * @brief packageAlreadyExists 包重复添加的信号
     */
    void packageAlreadyExists();

    /**
     * @brief appendStart 批量安装开始添加包的信号
     */
    void appendStart();

    /**
     * @brief appendFinished 批量安装添加包结束的信号
     */
    void appendFinished();

//// 界面刷新相关信号
signals:
    /**
     * @brief single2MultiPage 单包安装刷新为批量安装的信号
     */
    void single2MultiPage();

    /**
     * @brief refreshSinglePage 刷新单包安装界面的信号
     */
    void refreshSinglePage();

    /**
     * @brief refreshMultiPage 刷新批量安装界面的信号
     */
    void refreshMultiPage();

//// 后端状态相关函数
public:

    /**
     * @brief isBackendReady 判断安装程序后端是否加载完成
     * @return 安装程序后端加载的结果
     *
     * true: 加载完成
     * false: 未加载完成
     */
    bool isBackendReady();

    /**
     * @brief backend 获取后端指针
     * @return  后端的指针
     */
    QApt::Backend *backend() const { return m_backendFuture.result(); }

//// 包状态相关函数
public:

    /**
     * @brief package   获取指定下标的包的路径
     * @param index  下标
     * @return  包的路径
     */
    QString package(const int index) const { return m_preparedPackages[index]; }

    /**
     * @brief isArchError 判断指定下标的包是否符合架构要求
     * @param idx   指定的下标
     * @return  符合架构要求的结果
     *
     * false: 符合架构要求
     * true: 不符合当前系统架构的要求
     */
    bool isArchError(const int idx);


    /**
     * @brief packageInstallStatus 获取指定index的包的安装状态
     * @param index 指定的index
     * @return 包的安装状态
     */
    int packageInstallStatus(const int index);


    /**
     * @brief packageInstalledVersion 获取指定下标的包的安装版本
     * @param index 下标
     * @return 包的版本
     */
    const QString packageInstalledVersion(const int index);


    /**
     * @brief packageConflictStat 获取指定包的冲突状态
     * @param index 下标
     * @return 包的冲突状态
     */
    const ConflictResult packageConflictStat(const int index);


    /**
     * @brief packageAvailableDepends 获取指定包的可用的依赖
     * @param index 下标
     * @return  指定包所有的需要下载的依赖
     */
    const QStringList packageAvailableDepends(const int index);

    /**
     * @brief getPackageDependsStatus 获取指定包的依赖的状态
     * @param index 下标
     * @return 包的依赖状态
     */
    PackageDependsStatus getPackageDependsStatus(const int index);

    /**
     * @brief packageReverseDependsList 获取依赖于此包的所有应用名称
     * @param packageName 需要检查的包的包名
     * @param sysArch 包的架构
     * @return  依赖于此包的应用列表
     */
    const QStringList packageReverseDependsList(const QString &packageName, const QString &sysArch);

//// 添加删除相关函数
public:
    /**
     * @brief appendPackage 添加包到程序中
     * @param debPackage    要添加的包的列表
     */
    void appendPackage(QStringList debPackage);

    /**
     * @brief removePackage 删除指定的包
     * @param index 要删除的包的下标
     * @param listDependInstallMark 因wine依赖被标记的下标
     */
    void removePackage(const int index);

//// 重置状态相关函数
public:
    /**
     * @brief reset 重置 PackageManager的状态
     */
    void reset();

    /**
     * @brief resetPackageDependsStatus 重置指定安装包的状态
     * @param index 指定包的下标
     */
    void resetPackageDependsStatus(const int index);

public:

    QList<QByteArray> m_errorIndex;        //wine依赖错误的包的下标

//// 依赖查找 获取等相关函数
private:

    /**
     * @brief checkDependsPackageStatus 检查依赖包的状态
     * @param choosed_set   被选择安装或卸载的包的集合
     * @param architecture  包的架构
     * @param depends       包的依赖列表
     * @return
     */
    const PackageDependsStatus checkDependsPackageStatus(QSet<QString> &choosed_set, const QString &architecture,
                                                         const QList<QApt::DependencyItem> &depends);
    const PackageDependsStatus checkDependsPackageStatus(QSet<QString> &choosed_set, const QString &architecture,
                                                         const QApt::DependencyItem &candicate);
    const PackageDependsStatus checkDependsPackageStatus(QSet<QString> &choosed_set, const QString &architecture,
                                                         const QApt::DependencyInfo &dependencyInfo);
    /**
     * @brief packageCandidateChoose   查找包的依赖候选
     * @param choosed_set   包的依赖候选的集合
     * @param debArch       包的架构
     * @param dependsList   依赖列表
     */
    void packageCandidateChoose(QSet<QString> &choosed_set, const QString &debArch,
                                const QList<QApt::DependencyItem> &dependsList);
    void packageCandidateChoose(QSet<QString> &choosed_set, const QString &debArch,
                                const QApt::DependencyItem &candidateItem);

    /**
     * @brief isInstalledConflict 是否存在下载冲突
     * @param packageName         包名
     * @param packageVersion      包的版本
     * @param packageArch         包的架构
     * @return
     */
    const ConflictResult isInstalledConflict(const QString &packageName, const QString &packageVersion,
                                             const QString &packageArch);
    /**
     * @brief isConflictSatisfy 是否冲突满足
     * @param arch              架构
     * @param package           包名
     * @return     冲突的结果
     */
    const ConflictResult isConflictSatisfy(const QString &arch, QApt::Package *package);
    const ConflictResult isConflictSatisfy(const QString &arch, const QList<QApt::DependencyItem> &conflicts);

private:
    /**
     * @brief packageWithArch 从指定的架构上打包
     * @param packageName   包名
     * @param sysArch       系统架构
     * @param annotation    注解
     * @return  package指针
     */
    QApt::Package *packageWithArch(const QString &packageName, const QString &sysArch,
                                   const QString &annotation = QString());

    /**
     * @brief resetInstallStatus 重置所有包的安装状态
     */
    void resetInstallStatus();

private:

    // fix bug:https://pms.uniontech.com/zentao/bug-view-37220.html
    // 卸载deepin-wine-plugin-virture 时无法卸载deepin-wine-helper. Temporary solution：Special treatment for these package
    QMap<QString, QString> specialPackage();


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

    //使用软连接方式解决文件路径中存在空格的问题。
    /**
     * @brief rmTempDir 删除存放软链接的临时目录
     * @return 删除临时目录的结果
     */
    bool rmTempDir();

private:

    /**
     * @brief addPackage   添加包的处理槽函数
     * @param validPkgCount 有效包的数量
     * @param packagePath   此次添加包的路径
     * @param packageMd5Sum 此次添加的包的md5值
     */
    void addPackage(int validPkgCount, QString packagePath, QByteArray packageMd5Sum);


    /**
     * @brief refreshPage 刷新当前的页面
     * @param pkgCount  需要添加的包的数量
     */
    void refreshPage(int pkgCount);

    /**
     * @brief appendFinished 添加包结束后，如果此时需要下载wine依赖，则直接开始下载
     * @param 是否需要下载wine依赖的标识
     */
    void appendPackageFinished();

    void appendNoThread(QStringList packages, int allPackageSize);

    /**
     * @brief checkInvalid 检查有效文件的数量
     */
    void checkInvalid(QStringList packages);

private:
    QFuture<QApt::Backend *> m_backendFuture;           //安装程序后端指针(异步加载)

    QList<QString> m_preparedPackages;          //存放包路径的列表
    QSet<QByteArray> m_appendedPackagesMd5;     //存放包MD5的集合


    QList<QByteArray> m_packageMd5;

    /**
     * @brief m_packageMd5DependsStatus 包的依赖状态的Map
     * QByteArray 包的下标
     * PackageDependsStatus 依赖的状态
     *
     * 此前将下标与安装状态绑定
     * 此时使用MD5与依赖状态绑定
     * 修改原因：
     * 1.修改添加方式从添加到插入到最前方
     * 2.使用之前的方式会导致所有包的依赖状态错乱
     */
    QMap<QByteArray, PackageDependsStatus> m_packageMd5DependsStatus;

    /**
     * @brief m_packageInstallStatus 包安装状态的Map
     * int: 包的下标
     * int: 包的安装状态
     * 此前的安装状态将下标与安装状态绑定
     * 此时使用MD5与安装状态绑定
     * 修改原因：
     * 1.修改添加方式从添加到插入到最前方
     * 2.使用之前的方式会导致所有包的安装状态都是第一个包的安装状态
     */
    QMap<QByteArray, int> m_packageInstallStatus;

    int m_DealDependIndex = -1;             // wine应用处理的下标
    DealDependThread *dthread = nullptr;    //下载依赖的线程

    /**
     * @brief m_dependInstallMark
     * 将依赖下载的标记修改为md5sum  与包绑定 而非下标
     */
    QList<QByteArray> m_dependInstallMark;         //wine依赖下标的标记

private:
    const QString m_tempLinkDir = "/tmp/LinkTemp/";     //软链接临时路径

private:
    AddPackageThread *m_pAddPackageThread = nullptr;    //添加包的线程

    bool installWineDepends = false;

    int m_validPackageCount = 0;

    qint64 dependsStatusTotalTime = 0;

};

#endif  // PACKAGESMANAGER_H
