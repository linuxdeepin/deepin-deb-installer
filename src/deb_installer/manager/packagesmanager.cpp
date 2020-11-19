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

#include "packagesmanager.h"
#include "DealDependThread.h"
#include "PackageDependsStatus.h"
#include "AddPackageThread.h"

#include "model/deblistmodel.h"
#include "utils/utils.h"
#include "utils/DebugTimeManager.h"

#include <DRecentManager>

#include <QPair>
#include <QSet>
#include <QtConcurrent>
#include <QDir>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

using namespace QApt;

/**
 * @brief isArchMatches 判断包的架构是否符合系统要求
 * @param sysArch       系统架构
 * @param packageArch   包的架构
 * @param multiArchType 系统多架构类型
 * @return 是否符合多架构要求
 */
bool isArchMatches(QString sysArch, const QString &packageArch, const int multiArchType)
{
    Q_UNUSED(multiArchType);

    if (sysArch.startsWith(':')) sysArch.remove(0, 1);

    if (sysArch == "all" || sysArch == "any") return true;

    return sysArch == packageArch;
}

QString resolvMultiArchAnnotation(const QString &annotation, const QString &debArch,
                                  const int multiArchType = InvalidMultiArchType)
{
    if (annotation == "native" || annotation == "any") return QString();
    if (annotation == "all") return QString();
    if (multiArchType == MultiArchForeign) return QString();

    QString arch;
    if (annotation.isEmpty())
        arch = debArch;
    else
        arch = annotation;

    if (!arch.startsWith(':') && !arch.isEmpty())
        return arch.prepend(':');
    else
        return arch;
}

bool dependencyVersionMatch(const int result, const RelationType relation)
{
    switch (relation) {
    case LessOrEqual:
        return result <= 0;
    case GreaterOrEqual:
        return result >= 0;
    case LessThan:
        return result < 0;
    case GreaterThan:
        return result > 0;
    case Equals:
        return result == 0;
    case NotEqual:
        return result != 0;
    default:;
    }

    return true;
}

Backend *init_backend()
{
    Backend *b = new Backend;

    if (b->init()) return b;

    qFatal("%s", b->initErrorMessage().toStdString().c_str());
//    return nullptr;
}

PackagesManager::PackagesManager(QObject *parent)
    : QObject(parent)
    , m_backendFuture(QtConcurrent::run(init_backend))
{
    dthread = new DealDependThread();
    connect(dthread, &DealDependThread::DependResult, this, &PackagesManager::DealDependResult);
    connect(dthread, &DealDependThread::enableCloseButton, this, &PackagesManager::enableCloseButton);

    //批量打开 分批加载线程
    m_pAddPackageThread = new AddPackageThread(m_appendedPackagesMd5);

    //添加经过检查的包到软件中
    connect(m_pAddPackageThread, &AddPackageThread::addedPackage, this, &PackagesManager::addPackage, Qt::AutoConnection);

    //转发无效包的信号
    connect(m_pAddPackageThread, &AddPackageThread::invalidPackage, this, &PackagesManager::invalidPackage);

    //转发包已经添加的信号
    connect(m_pAddPackageThread, &AddPackageThread::packageAlreadyExists, this, &PackagesManager::packageAlreadyExists);

    //处理包添加结束的信号
    connect(m_pAddPackageThread, &AddPackageThread::appendFinished, this, &PackagesManager::appendPackageFinished);
}

bool PackagesManager::isBackendReady() { return m_backendFuture.isFinished(); }

bool PackagesManager::isArchError(const int idx)
{
    Backend *b = m_backendFuture.result();
    DebFile deb(m_preparedPackages[idx]);

    const QString arch = deb.architecture();

    if (arch == "all" || arch == "any") return false;

    bool architectures = !b->architectures().contains(deb.architecture());

    return architectures;
}

const ConflictResult PackagesManager::packageConflictStat(const int index)
{
    DebFile p(m_preparedPackages[index]);

    ConflictResult ConflictResult = isConflictSatisfy(p.architecture(), p.conflicts());
    return ConflictResult;
}

const ConflictResult PackagesManager::isConflictSatisfy(const QString &arch, Package *package)
{
    const QString &name = package->name();
    qDebug() << "PackagesManager:" <<  "check conflict for package" << name << arch;

    const auto ret_installed = isInstalledConflict(name, package->version(), package->architecture());
    if (!ret_installed.is_ok()) return ret_installed;

    qDebug() << "PackagesManager:" << "check conflict for local installed package is ok.";

    const auto ret_package = isConflictSatisfy(arch, package->conflicts());

    qDebug() << "PackagesManager:" << "check finished, conflict is satisfy:" << package->name() << bool(ret_package.is_ok());

    return ret_package;
}

const ConflictResult PackagesManager::isInstalledConflict(const QString &packageName, const QString &packageVersion,
                                                          const QString &packageArch)
{
    static QList<QPair<QString, DependencyInfo>> sysConflicts;

    if (sysConflicts.isEmpty()) {
        Backend *b = m_backendFuture.result();
        for (Package *p : b->availablePackages()) {
            if (!p->isInstalled()) continue;
            const auto &conflicts = p->conflicts();
            if (conflicts.isEmpty()) continue;

            for (const auto &conflict_list : conflicts)
                for (const auto &conflict : conflict_list)
                    sysConflicts << QPair<QString, DependencyInfo>(p->name(), conflict);
        }
    }

    for (const auto &info : sysConflicts) {
        const auto &conflict = info.second;
        const auto &pkgName = conflict.packageName();
        const auto &pkgVersion = conflict.packageVersion();
        const auto &pkgArch = conflict.multiArchAnnotation();

        if (pkgName != packageName) continue;

        // pass if arch not match
        if (!pkgArch.isEmpty() && pkgArch != packageArch && pkgArch != "any" && pkgArch != "native") continue;

        if (pkgVersion.isEmpty()) return ConflictResult::err(info.first);

        const int relation = Package::compareVersion(packageVersion, conflict.packageVersion());
        // match, so is bad
        if (dependencyVersionMatch(relation, conflict.relationType())) return ConflictResult::err(info.first);
    }

    return ConflictResult::ok(QString());
}

const ConflictResult PackagesManager::isConflictSatisfy(const QString &arch, const QList<DependencyItem> &conflicts)
{
    for (const auto &conflict_list : conflicts) {
        for (const auto &conflict : conflict_list) {
            const QString name = conflict.packageName();
            Package *p = packageWithArch(name, arch, conflict.multiArchAnnotation());

            if (!p || !p->isInstalled()) continue;

            // arch error, conflicts
            if (!isArchMatches(arch, p->architecture(), p->multiArchType())) {
                qDebug() << "PackagesManager:" << "conflicts package installed: " << arch << p->name() << p->architecture()
                         << p->multiArchTypeString();
                return ConflictResult::err(name);
            }

            const QString conflict_version = conflict.packageVersion();
            const QString installed_version = p->installedVersion();
            const auto type = conflict.relationType();
            const auto result = Package::compareVersion(installed_version, conflict_version);

            // not match, ok
            if (!dependencyVersionMatch(result, type)) continue;

            // test package
            const QString mirror_version = p->availableVersion();
            if (mirror_version == installed_version) continue;

            // mirror version is also break
            const auto mirror_result = Package::compareVersion(mirror_version, conflict_version);
            if (dependencyVersionMatch(mirror_result, type)) {
                qDebug() << "PackagesManager:" <<  "conflicts package installed: " << arch << p->name() << p->architecture()
                         << p->multiArchTypeString() << mirror_version << conflict_version;
                return ConflictResult::err(name);
            }
        }
    }

    return ConflictResult::ok(QString());
}

int PackagesManager::packageInstallStatus(const int index)
{
    //修改安装状态的存放方式，将安装状态与MD5绑定，而非与index绑定
    //如果此时已经刷新过安装状态，则直接返回。
    //PS: 修改原因见头文件
    if (m_packageInstallStatus.contains(m_packageMd5[index]))
        return m_packageInstallStatus[m_packageMd5[index]];

    DebFile *deb = new DebFile(m_preparedPackages[index]);

    const QString packageName = deb->packageName();
    const QString packageArch = deb->architecture();
    Backend *b = m_backendFuture.result();
    Package *p = b->package(packageName + ":" + packageArch);

    int ret = DebListModel::NotInstalled;
    do {
        if (!p) break;

        const QString installedVersion = p->installedVersion();
        if (installedVersion.isEmpty()) break;

        const QString packageVersion = deb->version();
        const int result = Package::compareVersion(packageVersion, installedVersion);

        if (result == 0)
            ret = DebListModel::InstalledSameVersion;
        else if (result < 0)
            ret = DebListModel::InstalledLaterVersion;
        else
            ret = DebListModel::InstalledEarlierVersion;
    } while (false);

    //存储包的安装状态
    //2020-11-19 修改安装状态的存储绑定方式
    m_packageInstallStatus[m_packageMd5[index]] = ret;
    delete deb;
    return ret;
}

void PackagesManager::DealDependResult(int iAuthRes, int iIndex, QString dependName)
{
    if (iAuthRes == DebListModel::AuthDependsSuccess) {
        for (int num = 0; num < m_dependInstallMark.size(); num++) {
            m_packageMd5DependsStatus[m_dependInstallMark.at(num)].status = DebListModel::DependsOk;//更换依赖的存储结构
        }
        m_errorIndex.clear();
    }
    if (iAuthRes == DebListModel::CancelAuth || iAuthRes == DebListModel::AnalysisErr) {
        for (int num = 0; num < m_dependInstallMark.size(); num++) {
            m_packageMd5DependsStatus[m_dependInstallMark.at(num)].status = DebListModel::DependsAuthCancel;//更换依赖的存储结构
        }
        emit enableCloseButton(true);
    }
    if (iAuthRes == DebListModel::AuthDependsErr) {
        for (int num = 0; num < m_dependInstallMark.size(); num++) {
            m_packageMd5DependsStatus[m_dependInstallMark.at(num)].status = DebListModel::DependsBreak;//更换依赖的存储结构
            if (!m_errorIndex.contains(m_dependInstallMark[num]))
                m_errorIndex.push_back(m_dependInstallMark[num]);
        }
        emit enableCloseButton(true);
    }
    emit DependResult(iAuthRes, iIndex, dependName);
}

PackageDependsStatus PackagesManager::getPackageDependsStatus(const int index)
{
    //更换依赖的存储方式
    if (m_packageMd5DependsStatus.contains(m_packageMd5[index])) {
        return m_packageMd5DependsStatus[m_packageMd5[index]];
    }

    DebFile *deb = new DebFile(m_preparedPackages[index]);
    const QString architecture = deb->architecture();
    PackageDependsStatus ret = PackageDependsStatus::ok();

    if (isArchError(index)) {
        ret.status = DebListModel::ArchBreak;       //添加ArchBreak错误。
        ret.package = deb->packageName();
        m_packageMd5DependsStatus.insert(m_packageMd5[index], ret);//更换依赖的存储方式
        return PackageDependsStatus::_break(deb->packageName());
    }

    // conflicts
    const ConflictResult debConflitsResult = isConflictSatisfy(architecture, deb->conflicts());

    if (!debConflitsResult.is_ok()) {
        qDebug() << "PackagesManager:" << "depends break because conflict" << deb->packageName();
        ret.package = debConflitsResult.unwrap();
        ret.status = DebListModel::DependsBreak;
    } else {
        const ConflictResult localConflictsResult =
            isInstalledConflict(deb->packageName(), deb->version(), architecture);
        if (!localConflictsResult.is_ok()) {
            qDebug() << "PackagesManager:" << "depends break because conflict with local package" << deb->packageName();
            ret.package = localConflictsResult.unwrap();
            ret.status = DebListModel::DependsBreak;
        } else {
            QSet<QString> choose_set;
            choose_set << deb->packageName();
            QStringList dependList;
            bool isWineApplication = false;             //判断是否是wine应用
            for (auto ditem : deb->depends()) {
                for (auto dinfo : ditem) {
                    Package *depend = packageWithArch(dinfo.packageName(), deb->architecture());
                    if (depend) {
                        if (depend->name() == "deepin-elf-verify") {    //deepi-elf-verify 是amd64架构非i386
                            dependList << depend->name();
                        } else {
                            dependList << depend->name() + ":" + depend->architecture();
                        }
                        if (dinfo.packageName().contains("deepin-wine")) {              // 如果依赖中出现deepin-wine字段。则是wine应用
                            qDebug() << deb->packageName() << "is a wine Application";
                            isWineApplication = true;
                        }
                    }
                }
            }
            ret = checkDependsPackageStatus(choose_set, deb->architecture(), deb->depends());
            qDebug() << "PackagesManager:" << "Check" << deb->packageName() << "depends:" << ret.status;

            // 删除无用冗余的日志
            //由于卸载p7zip会导致wine依赖被卸载，再次安装会造成应用闪退，因此判断的标准改为依赖不满足即调用pkexec
            //fix bug: https://pms.uniontech.com/zentao/bug-view-45734.html
            if (isWineApplication && ret.status != DebListModel::DependsOk) {               //增加是否是wine应用的判断
                qDebug() << "PackagesManager:" << "Unsatisfied dependency: " << ret.package;
                if (!m_dependInstallMark.contains(m_packageMd5[index])) {           //更换判断依赖错误的标记
                    if (!dthread->isRunning()) {
                        m_dependInstallMark.append(m_packageMd5[index]);            //依赖错误的软件包的标记 更改为md5取代验证下标
                        qDebug() << "PackagesManager:" << "command install depends:" << dependList;
                        dthread->setDependsList(dependList, index);
                        dthread->setBrokenDepend(ret.package);
                        dthread->run();
                    }
                }
                ret.status = DebListModel::DependsBreak;                                    //只要是下载，默认当前wine应用依赖为break
            }
        }
    }
    if (ret.isBreak()) Q_ASSERT(!ret.package.isEmpty());

    m_packageMd5DependsStatus.insert(m_packageMd5[index], ret);

    if (ret.status == DebListModel::DependsAvailable) {
        const auto list = packageAvailableDepends(index);
        qDebug() << "PackagesManager:"  << "Available depends list:" << list.size() << list;
    }
    delete deb;
    return ret;
}

const QString PackagesManager::packageInstalledVersion(const int index)
{
    //更换安装状态的存储结构
    Q_ASSERT(m_packageInstallStatus.contains(m_packageMd5[index]));

    DebFile *deb = new DebFile(m_preparedPackages[index]);

    const QString packageName = deb->packageName();
    const QString packageArch = deb->architecture();
    Backend *b = m_backendFuture.result();
    Package *p = b->package(packageName + ":" + packageArch);
    delete  deb;

    //修复可能某些包无法package的错误，如果遇到此类包，返回安装版本为空
    if (p)
        return p->installedVersion();   //能正常打包，返回包的安装版本
    else {
        return "";                      //此包无法正常package，返回空
    }
}

const QStringList PackagesManager::packageAvailableDepends(const int index)
{
    DebFile *deb = new DebFile(m_preparedPackages[index]);
    QSet<QString> choose_set;
    const QString debArch = deb->architecture();
    const auto &depends = deb->depends();
    packageCandidateChoose(choose_set, debArch, depends);

    // TODO: check upgrade from conflicts
    delete deb;
    return choose_set.toList();
}

void PackagesManager::packageCandidateChoose(QSet<QString> &choosed_set, const QString &debArch,
                                             const QList<DependencyItem> &dependsList)
{
    for (auto const &candidate_list : dependsList) packageCandidateChoose(choosed_set, debArch, candidate_list);
}

void PackagesManager::packageCandidateChoose(QSet<QString> &choosed_set, const QString &debArch,
                                             const DependencyItem &candidateList)
{
    bool choosed = false;

    for (const auto &info : candidateList) {
        Package *dep = packageWithArch(info.packageName(), debArch, info.multiArchAnnotation());
        if (!dep) continue;

        const auto choosed_name = dep->name() + resolvMultiArchAnnotation(QString(), dep->architecture());
        if (choosed_set.contains(choosed_name)) {
            choosed = true;
            break;
        }

        // TODO: upgrade?
        //        if (!dep->installedVersion().isEmpty()) return;
        //  修复升级依赖时，因为依赖包版本过低，造成安装循环。
        // 删除无用冗余的日志
        if (Package::compareVersion(dep->installedVersion(), info.packageVersion()) < 0) {
            Backend *b = m_backendFuture.result();
            Package *p = b->package(dep->name() + resolvMultiArchAnnotation(QString(), dep->architecture()));
            if (p)
                choosed_set << dep->name() + resolvMultiArchAnnotation(QString(), dep->architecture());
            else
                choosed_set << dep->name() + " not found";
        }

        if (!isConflictSatisfy(debArch, dep->conflicts()).is_ok()) {
            qDebug() << "PackagesManager:" << "conflict error in choose candidate" << dep->name();
            continue;
        }

        // pass if break
        QSet<QString> set = choosed_set;
        set << choosed_name;
        const auto stat = checkDependsPackageStatus(set, dep->architecture(), dep->depends());
        if (stat.isBreak()) {
            qDebug() << "PackagesManager:" << "depends error in choose candidate" << dep->name();
            continue;
        }

        choosed = true;
        choosed_set << choosed_name;
        packageCandidateChoose(choosed_set, debArch, dep->depends());
        break;
    }

//    Q_ASSERT(choosed);
}

QMap<QString, QString> PackagesManager::specialPackage()
{
    QMap<QString, QString> sp;
    sp.insert("deepin-wine-plugin-virtual", "deepin-wine-helper");
    sp.insert("deepin-wine32", "deepin-wine");

    return sp;
}

const QStringList PackagesManager::packageReverseDependsList(const QString &packageName, const QString &sysArch)
{
    Package *package = packageWithArch(packageName, sysArch);
    Q_ASSERT(package);

    QSet<QString> ret{packageName};
    QQueue<QString> testQueue;

    for (const auto &item : package->requiredByList().toSet())
        testQueue.append(item);
    while (!testQueue.isEmpty()) {
        const auto item = testQueue.first();
        testQueue.pop_front();

        if (ret.contains(item)) continue;

        Package *p = packageWithArch(item, sysArch);
        if (!p || !p->isInstalled()) continue;

        if (p->recommendsList().contains(packageName)) continue;
        if (p->suggestsList().contains(packageName)) continue;
        // fix bug: https://pms.uniontech.com/zentao/bug-view-37220.html dde相关组件特殊处理.
        //修复dde会被动卸载但是不会提示的问题
        //if (item.contains("dde")) continue;
        ret << item;

        // fix bug:https://pms.uniontech.com/zentao/bug-view-37220.html
        if (specialPackage().contains(item)) {
            testQueue.append(specialPackage()[item]);
        }
        // append new reqiure list
        for (const auto &r : p->requiredByList()) {
            if (ret.contains(r) || testQueue.contains(r)) continue;
            Package *subPackage = packageWithArch(r, sysArch);
            if (!subPackage || !subPackage->isInstalled())
                continue;
            if (subPackage->recommendsList().contains(item))
                continue;
            if (subPackage->suggestsList().contains(item))
                continue;
            testQueue.append(r);
        }
    }
    // remove self
    ret.remove(packageName);

    return ret.toList();
}

void PackagesManager::reset()
{
    m_errorIndex.clear();
    m_dependInstallMark.clear();
    m_preparedPackages.clear();
    m_packageInstallStatus.clear();
    m_packageMd5DependsStatus.clear();  //修改依赖状态的存储结构，此处清空存储的依赖状态数据
    m_appendedPackagesMd5.clear();

    //reloadCache必须要加
    m_backendFuture.result()->reloadCache();
}

void PackagesManager::resetInstallStatus()
{
    m_packageInstallStatus.clear();
    m_packageMd5DependsStatus.clear();          //修改依赖状态的存储结构，此处清空存储的依赖状态数据
    //reloadCache必须要加
    m_backendFuture.result()->reloadCache();
}

void PackagesManager::resetPackageDependsStatus(const int index)
{
    // 查看此包是否已经存储依赖状态。
    if (!m_packageMd5DependsStatus.contains(m_packageMd5[index])) return;   //更改依赖状态的存储结构
    else {
        // 针对wine依赖做一个特殊处理，如果wine依赖break,则直接返回。
        if ((m_packageMd5DependsStatus[m_packageMd5[index]].package == "deepin-wine") && m_packageMd5DependsStatus[m_packageMd5[index]].status != DebListModel::DependsOk) {
            return;
        }
    }
    // reload backend cache
    //reloadCache必须要加
    m_backendFuture.result()->reloadCache();;
    m_packageMd5DependsStatus.remove(m_packageMd5[index]);  //删除当前包的依赖状态（之后会重新获取此包的依赖状态）
}

/**
 * @brief PackagesManager::removePackage 删除指定下标的包
 * @param index 指定的下标
 */
void PackagesManager::removePackage(const int index)
{
    DebFile *deb = new DebFile(m_preparedPackages[index]);
    const auto md5 = deb->md5Sum();
    delete deb;
    m_appendedPackagesMd5.remove(md5);
    m_preparedPackages.removeAt(index);

    m_appendedPackagesMd5.remove(m_packageMd5[index]);          //在判断是否重复的md5的集合中删除掉当前包的md5
    m_packageMd5DependsStatus.remove(m_packageMd5[index]);      //删除指定包的依赖状态
    m_packageMd5.removeAt(index);                               //在索引map中删除指定的项

    if (m_dependInstallMark.contains(m_packageMd5[index]))      //如果这个包是wine包，则在wine标记list中删除
        m_dependInstallMark.removeOne(m_packageMd5[index]);

    m_packageInstallStatus.clear();
}

/**
 * @brief PackagesManager::appendPackage 将前端给的包，传输到添加线程中。并开始添加
 * @param packages 要添加的包的列表
 * 此处可以优化，如果只有一两个包直接添加
 * 大于等于三个包，先添加两个再开始线程
 */
void PackagesManager::appendPackage(QStringList packages)
{
    checkInvalid(packages);     //运行之前先计算有效文件的数量
    qDebug() << "PackagesManager:" << "append Package" << packages;
    if (packages.size() == 1) {
        appendNoThread(packages, packages.size());
    } else {
        QStringList subPackages;
        subPackages << packages[0];
        appendNoThread(subPackages, packages.size());
        packages.removeAt(0);

        if (packages.isEmpty())
            return;
        m_pAddPackageThread->setPackages(packages);                     //传递要添加的包到添加线程中
        m_pAddPackageThread->setAppendPackagesMd5(m_appendedPackagesMd5);       //传递当前已经添加的包的MD5 判重时使用

        m_pAddPackageThread->start();   //开始添加线程
    }
}

/**
 * @brief AddPackageThread::checkInvalid 检查有效文件的数量
 */
void PackagesManager::checkInvalid(QStringList packages)
{
    m_validPackageCount = 0; //每次添加时都清零
    for (QString package : packages) {
        QApt::DebFile *pkgFile = new DebFile(package);
        if (pkgFile && pkgFile->isValid()) {            //只有有效文件才会计入
            m_validPackageCount ++;
        }
        delete pkgFile;
    }
}

void PackagesManager::appendNoThread(QStringList packages, int allPackageSize)
{
    qDebug() << "[PackagesManager]" << "[appendNoThread]" << "start add packages";
    for (QString debPackage : packages) {                 //通过循环添加所有的包
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
            qDebug() << "[PackagesManager]" << "[appendNoThread]" << "package is invalid";
            emit invalidPackage();
            delete pkgFile;
            continue;
        }

        PERF_PRINT_BEGIN("POINT-03", "pkgsize=" + QString::number(pkgFile->installedSize()) + "b");
        // 获取当前文件的md5的值,防止重复添加
        qInfo() << "[Performance Testing]: Before get the value of MD5 Sum";
        const auto md5 = pkgFile->md5Sum();
        qInfo() << "[Performance Testing]: After get the value of MD5 Sum";

        // 如果当前已经存在此md5的包,则说明此包已经添加到程序中
        if (m_appendedPackagesMd5.contains(md5)) {
            //处理重复文件
            emit packageAlreadyExists();
            delete pkgFile;
            continue;
        }
        // 可以添加,发送添加信号

        addPackage(m_validPackageCount, debPackage, md5);
        delete pkgFile;
    }

    //所有包都添加结束.
    if (allPackageSize == 1)
        PERF_PRINT_END("POINT-03");
}

/**
 * @brief PackagesManager::refreshPage 根据添加包的情况 刷新页面
 * @param validPkgCount 此次添加的包的数量（一次拖入或者打开【可能是多个包】【此处只是预计能够添加到程序中的包的数量】）
 */
void PackagesManager::refreshPage(int validPkgCount)
{
    // 获取当前已经添加到程序中的包的数量
    int packageCount = m_preparedPackages.size();
    if (packageCount == 1) {            //当前程序中只添加了一个包
        if (validPkgCount == 1) {       //此次只有一个包将会被添加的程序中
            emit refreshSinglePage();   //刷新单包安装界面

        } else if (validPkgCount > 1) {  //当前程序中值添加了一个包，但是这次有不止一个包将会被添加到程序中
            emit single2MultiPage();     //刷新批量安装界面
            emit appendStart();          //开始批量添加
        }
    } else if (packageCount == 2) {
        //当前程序中已经添加了两个包
        //1.第一次是添加了一个包，第二次又添加了多于一个包

        emit single2MultiPage();        //刷新批量安装界面
        emit appendStart();             //发送批量添加信号
    } else {
        //此时批量安装界面已经刷新过。如果再添加，就只刷新model
        emit refreshMultiPage();
        emit appendStart();
    }
}

/**
 * @brief PackagesManager::appendPackageFinished 此次添加已经结束
 */
void PackagesManager::appendPackageFinished()
{
    //刷新所有添加的包的依赖状态
    for (int i = 0; i < m_preparedPackages.size(); i++) {
        getPackageDependsStatus(i);
    }
    //告诉前端，此次添加已经结束
    emit appendFinished();
}

void PackagesManager::addPackage(int validPkgCount, QString packagePath, QByteArray packageMd5Sum)
{
    qInfo() << "[PackagesManager]:" << "[addPackage]" << packagePath;
    //一定要保持 m_preparedPacjages 和 packageMd5的下标保持一致
    //二者一定是一一对应的。
    //此后的依赖状态和安装状态都是与md5绑定的 md5是与index绑定
    m_preparedPackages.insert(0, packagePath);      //每次添加的包都放到最前面
    m_packageMd5.insert(0, packageMd5Sum);          //添加MD5Sum
    m_appendedPackagesMd5 << packageMd5Sum;         //将MD5添加到集合中，这里是为了判断包不再重复
    refreshPage(validPkgCount);                     //添加后，根据添加的状态刷新界面
}

const PackageDependsStatus PackagesManager::checkDependsPackageStatus(QSet<QString> &choosed_set,
                                                                      const QString &architecture,
                                                                      const QList<DependencyItem> &depends)
{
    PackageDependsStatus ret = PackageDependsStatus::ok();

    for (const auto &candicate_list : depends) {
        const auto r = checkDependsPackageStatus(choosed_set, architecture, candicate_list);
        ret.maxEq(r);

        if (ret.isBreak()) break;
    }

    return ret;
}

const PackageDependsStatus PackagesManager::checkDependsPackageStatus(QSet<QString> &choosed_set,
                                                                      const QString &architecture,
                                                                      const DependencyItem &candicate)
{
    PackageDependsStatus ret = PackageDependsStatus::_break(QString());

    for (const auto &info : candicate) {
        const auto r = checkDependsPackageStatus(choosed_set, architecture, info);
        ret.minEq(r);

        if (!ret.isBreak()) break;
    }

    return ret;
}

const PackageDependsStatus PackagesManager::checkDependsPackageStatus(QSet<QString> &choosed_set,
                                                                      const QString &architecture,
                                                                      const DependencyInfo &dependencyInfo)
{
    const QString package_name = dependencyInfo.packageName();

    Package *p = packageWithArch(package_name, architecture, dependencyInfo.multiArchAnnotation());

    if (!p) {
        qDebug() << "PackagesManager:" << "depends break because package" << package_name << "not available";
        return PackageDependsStatus::_break(package_name);
    }

    const RelationType relation = dependencyInfo.relationType();
    const QString &installedVersion = p->installedVersion();

    if (!installedVersion.isEmpty()) {
        const int result = Package::compareVersion(installedVersion, dependencyInfo.packageVersion());
        if (dependencyVersionMatch(result, relation))
            return PackageDependsStatus::ok();
        else {
            const QString &mirror_version = p->availableVersion();
            if (mirror_version != installedVersion) {
                const auto mirror_result = Package::compareVersion(mirror_version, dependencyInfo.packageVersion());

                if (dependencyVersionMatch(mirror_result, relation)) {
                    qDebug() << "PackagesManager:" << "availble by upgrade package" << p->name() + ":" + p->architecture() << "from"
                             << installedVersion << "to" << mirror_version;
                    // 修复卸载p7zip导致deepin-wine-helper被卸载的问题，Available 添加packageName
                    return PackageDependsStatus::available(p->name());
                }
            }

            qDebug() << "PackagesManager:" << "depends break by" << p->name() << p->architecture() << dependencyInfo.packageVersion();
            qDebug() << "PackagesManager:" << "installed version not match" << installedVersion;
            return PackageDependsStatus::_break(p->name());
        }
    } else {
        const int result = Package::compareVersion(p->version(), dependencyInfo.packageVersion());
        if (!dependencyVersionMatch(result, relation)) {
            qDebug() << "PackagesManager:" << "depends break by" << p->name() << p->architecture() << dependencyInfo.packageVersion();
            qDebug() << "PackagesManager:" << "available version not match" << p->version();
            return PackageDependsStatus::_break(p->name());
        }

        // is that already choosed?
        if (choosed_set.contains(p->name())) return PackageDependsStatus::ok();

        // check arch conflicts
        if (p->multiArchType() == MultiArchSame) {
            Backend *b = backend();
            for (const auto &arch : b->architectures()) {
                if (arch == p->architecture()) continue;

                Package *tp = b->package(p->name() + ":" + arch);
                if (tp && tp->isInstalled()) {
                    qDebug() << "PackagesManager:" << "multiple architecture installed: " << p->name() << p->version() << p->architecture() << "but now need"
                             << tp->name() << tp->version() << tp->architecture();
                    return PackageDependsStatus::_break(p->name() + ":" + p->architecture());
                }
            }
        }
        // let's check conflicts
        if (!isConflictSatisfy(architecture, p).is_ok()) {
            qDebug() << "PackagesManager:" << "depends break because conflict, ready to find providers" << p->name();

            Backend *b = m_backendFuture.result();
            for (auto *ap : b->availablePackages()) {
                if (!ap->providesList().contains(p->name())) continue;

                // is that already provide by another package?
                if (ap->isInstalled()) {
                    qDebug() << "PackagesManager:" << "find a exist provider: " << ap->name();
                    return PackageDependsStatus::ok();
                }

                // provider is ok, switch to provider.
                if (isConflictSatisfy(architecture, ap).is_ok()) {
                    qDebug() << "PackagesManager:" << "switch to depends a new provider: " << ap->name();
                    choosed_set << ap->name();
                    return PackageDependsStatus::ok();
                }
            }

            qDebug() << "PackagesManager:" << "providers not found, still break: " << p->name();
            return PackageDependsStatus::_break(p->name());
        }

        // now, package dependencies status is available or break,
        // time to check depends' dependencies, but first, we need
        // to add this package to choose list
        choosed_set << p->name();

        qDebug() << "PackagesManager:" << "Check indirect dependencies for package" << p->name();

        const auto r = checkDependsPackageStatus(choosed_set, p->architecture(), p->depends());
        if (r.isBreak()) {
            choosed_set.remove(p->name());
            qDebug() << "PackagesManager:" << "depends break by direct depends" << p->name() << p->architecture() << r.package;
            return PackageDependsStatus::_break(p->name());
        }

        qDebug() << "PackagesManager:" << "Check finshed for package" << p->name();

        // 修复卸载p7zip导致deepin-wine-helper被卸载的问题，Available 添加packageName
        return PackageDependsStatus::available(p->name());
    }
}

Package *PackagesManager::packageWithArch(const QString &packageName, const QString &sysArch,
                                          const QString &annotation)
{
    Backend *b = m_backendFuture.result();
    Package *p = b->package(packageName + resolvMultiArchAnnotation(annotation, sysArch));
    do {
        // change: 按照当前支持的CPU架构进行打包。取消对deepin-wine的特殊处理
        if (!p) p = b->package(packageName);
        if (p) break;
        for (QString arch : b->architectures()) {
            if (!p) p = b->package(packageName + ":" + arch);
            if (p) break;
        }

    } while (false);

    if (p) return p;

    qDebug() << "PackagesManager:" << "check virtual package providers for" << packageName << sysArch << annotation;

    // check virtual package providers
    for (auto *ap : b->availablePackages())
        if (ap->name() != packageName && ap->providesList().contains(packageName))
            return packageWithArch(ap->name(), sysArch, annotation);
    return nullptr;
}

/**
 * @brief PackagesManager::SymbolicLink 创建软连接
 * @param previousName 原始路径
 * @param packageName 软件包的包名
 * @return 软链接的路径
 */
QString PackagesManager::SymbolicLink(QString previousName, QString packageName)
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
bool PackagesManager::mkTempDir()
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
QString PackagesManager::link(QString linkPath, QString packageName)
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

/**
 * @brief PackagesManager::rmTempDir 删除存放软链接的临时目录
 * @return 删除临时目录的结果
 * PS: 移动创建临时目录 创建软链接的函数到 AddPackageThread中
 *
 */
bool PackagesManager::rmTempDir()
{
    QDir tempPath(m_tempLinkDir);
    if (tempPath.exists()) {            //如果临时目录存在，则删除临时目录
        return tempPath.removeRecursively();
    } else {
        return true;                    //临时目录不存在，返回删除成功
    }
}

PackagesManager::~PackagesManager()
{
    // 删除 临时目录，会尝试四次，四次失败后退出。
    int rmTempDirCount = 0;
    while (true) {
        if (rmTempDir())        //删除成功
            break;
        qWarning() << "PackagesManager:" << "Failed to delete temporary folder， Current attempts:" << rmTempDirCount << "/3";
        if (rmTempDirCount > 3) {       //删除三次仍然失败则警告 不过每次重启都会删除临时目录
            qWarning() << "PackagesManager:" << "Failed to delete temporary folder, Exit application";
            break;
        }
        rmTempDirCount++;
    }
    delete dthread;

    //先取消当前异步计算的后端。
    m_backendFuture.cancel();

    Backend *b = m_backendFuture.result();

    // 删除与库的连接
    b->deleteLater();
    delete b;
    PERF_PRINT_END("POINT-02");         //关闭应用
}

