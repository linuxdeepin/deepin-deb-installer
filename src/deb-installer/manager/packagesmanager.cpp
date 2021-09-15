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
#include "utils/utils.h"
#include "model/deblistmodel.h"

#include <DRecentManager>

#include <QPair>
#include <QSet>
#include <QDir>
#include <QtConcurrent>

#include <fstream>

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
    if ("all" ==sysArch|| "any" == sysArch) 
        return true;

    return sysArch == packageArch;
}

QString resolvMultiArchAnnotation(const QString &annotation, const QString &debArch, const int multiArchType)
{
    if ("native" == annotation|| "any" == annotation) 
        return QString();
    if ("all" == annotation ) 
        return QString();
    if (multiArchType == MultiArchForeign) 
        return QString();

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
    default:
        ;
    }

    return true;
}

Backend *init_backend()
{
    Backend *backend = new Backend;

    if (backend->init()) 
        return backend;

    qFatal("%s", backend->initErrorMessage().toStdString().c_str());
}

PackagesManager::PackagesManager(QObject *parent)
    : QObject(parent)
    , m_backendFuture(QtConcurrent::run(init_backend))
{
    m_installWineThread = new DealDependThread();
    connect(m_installWineThread, &DealDependThread::signalDependResult, this, &PackagesManager::slotDealDependResult);
    connect(m_installWineThread, &DealDependThread::signalEnableCloseButton, this, &PackagesManager::signalEnableCloseButton);

    //批量打开 分批加载线程
    m_pAddPackageThread = new AddPackageThread(m_appendedPackagesMd5);

    //添加经过检查的包到软件中
    connect(m_pAddPackageThread, &AddPackageThread::signalAddPackageToInstaller, this, &PackagesManager::addPackage, Qt::AutoConnection);

    //转发无效包的信号
    connect(m_pAddPackageThread, &AddPackageThread::signalInvalidPackage, this, &PackagesManager::signalInvalidPackage);

    //转发不是本地包的信号
    connect(m_pAddPackageThread, &AddPackageThread::signalNotLocalPackage, this, &PackagesManager::signalNotLocalPackage);

    //转发包已经添加的信号
    connect(m_pAddPackageThread, &AddPackageThread::signalPackageAlreadyExists, this, &PackagesManager::signalPackageAlreadyExists);

    //处理包添加结束的信号
    connect(m_pAddPackageThread, &AddPackageThread::signalAppendFinished, this, &PackagesManager::slotAppendPackageFinished);
}

bool PackagesManager::isBackendReady()
{
    return m_backendFuture.isFinished();
}

bool PackagesManager::isArchError(const int idx)
{
    Backend *backend = m_backendFuture.result();
    DebFile deb(m_preparedPackages[idx]);
    if(!deb.isValid())
        return false;
    const QString arch = deb.architecture();

    if ("all" == arch ||"any" == arch ) 
        return false;

    bool architectures = !backend->architectures().contains(deb.architecture());

    return architectures;
}

const ConflictResult PackagesManager::packageConflictStat(const int index)
{
    DebFile debfile(m_preparedPackages[index]);
    if(!debfile.isValid())
        return ConflictResult::err("");

    ConflictResult ConflictResult = isConflictSatisfy(debfile.architecture(), debfile.conflicts());
    return ConflictResult;
}

const ConflictResult PackagesManager::isConflictSatisfy(const QString &arch, Package *package)
{
    const QString &packageName = package->name();
    const auto ret_installed = isInstalledConflict(packageName, package->version(), package->architecture());
    if (!ret_installed.is_ok()){
        qWarning()<<packageName <<"check installed conflict not satisfied";
        return ret_installed;
    } 

    const auto conflictStatus = isConflictSatisfy(arch, package->conflicts());

    return conflictStatus;
}

const ConflictResult PackagesManager::isInstalledConflict(const QString &packageName, const QString &packageVersion,
                                                          const QString &packageArch)
{
    static QList<QPair<QString, DependencyInfo>> sysConflicts;

    if (sysConflicts.isEmpty()) {
        Backend *b = m_backendFuture.result();
        for (Package *p : b->availablePackages()) {
            if (!p->isInstalled()) 
                continue;
            const auto &conflicts = p->conflicts();
            if (conflicts.isEmpty()) 
                continue;

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

        if (pkgName != packageName) 
            continue;

        // pass if arch not match
        if (!pkgArch.isEmpty() && pkgArch != packageArch && pkgArch != "any" && pkgArch != "native") 
            continue;

        if (pkgVersion.isEmpty()) 
            return ConflictResult::err(info.first);

        const int relation = Package::compareVersion(packageVersion, conflict.packageVersion());
        // match, so is bad
        if (dependencyVersionMatch(relation, conflict.relationType())) 
            return ConflictResult::err(info.first);
    }

    return ConflictResult::ok(QString());
}

const ConflictResult PackagesManager::isConflictSatisfy(const QString &arch, const QList<DependencyItem> &conflicts)
{
    for (const auto &conflict_list : conflicts) {
        for (const auto &conflict : conflict_list) {
            const QString name = conflict.packageName();

            //修复依赖中 conflict与provides 存在相同 virtual package
            //此前使用packageWithArch, 在package打包失败时，会寻找virtual package的提供的其他包
            //在dde-daemon中 lastore-daemon-migration与dde-daemon为conflict,
            //lastore-daemon-migration 又提供了dde-daemon,导致最后打成的包不是virtual package而是provides的包
            Backend *backend = m_backendFuture.result();
            if(!backend)
                return ConflictResult::err(QString());
            Package *package = backend->package(name);

            if(!package)
                continue;

            if (!package->isInstalled()){
                package = nullptr;
                continue;
            }
            // arch error, conflicts
            if (!isArchMatches(arch, package->architecture(), package->multiArchType())) {
                qWarning() << "PackagesManager:" << "conflicts package installed: "
                         << arch << package->name() << package->architecture()
                         << package->multiArchTypeString();
                package = nullptr;
                return ConflictResult::err(name);
            }

            const QString conflict_version = conflict.packageVersion();
            const QString installed_version = package->installedVersion();
            const auto type = conflict.relationType();
            const auto result = Package::compareVersion(installed_version, conflict_version);

            // not match, ok
            if (!dependencyVersionMatch(result, type)) {
                package = nullptr;
                continue;
            }
            // test package
            const QString mirror_version = package->availableVersion();

            //删除版本相同比较，如果安装且版本符合则判断冲突，此前逻辑存在问题
            // mirror version is also break
            const auto mirror_result = Package::compareVersion(mirror_version, conflict_version);
            if (dependencyVersionMatch(mirror_result, type)) {
                qWarning() << "PackagesManager:" <<  "conflicts package installed: "
                         << arch << package->name() << package->architecture()
                         << package->multiArchTypeString() << mirror_version << conflict_version;
                package = nullptr;
                return ConflictResult::err(name);
            }

        }
    }
    return ConflictResult::ok(QString());
}

int PackagesManager::packageInstallStatus(const int index)
{
    if(index < 0 || index >= m_preparedPackages.size())
        return -1;
    //修改安装状态的存放方式，将安装状态与MD5绑定，而非与index绑定
    //如果此时已经刷新过安装状态，则直接返回。
    //PS: 修改原因见头文件

    //提前获取当前的md5
    auto currentPackageMd5 = m_packageMd5[index];
    if (m_packageInstallStatus.contains(currentPackageMd5))
        return m_packageInstallStatus[currentPackageMd5];

    DebFile debFile(m_preparedPackages[index]);
    if (!debFile.isValid())
        return DebListModel::NotInstalled;
    const QString packageName = debFile.packageName();
    const QString packageArch = debFile.architecture();
    Backend *backend = m_backendFuture.result();
    if(!backend){
        qWarning()<< "Failed to load libqapt backend";
        return DebListModel::NotInstalled;
    }
    Package *package = packageWithArch(packageName, packageArch);

    int ret = DebListModel::NotInstalled;

    if (!package)
        return ret;

    const QString installedVersion = package->installedVersion();
    package = nullptr;
    if (installedVersion.isEmpty())
        return ret;

    const QString packageVersion = debFile.version();
    const int result = Package::compareVersion(packageVersion, installedVersion);

    if (result == 0)
        ret = DebListModel::InstalledSameVersion;
    else if (result < 0)
        ret = DebListModel::InstalledLaterVersion;
    else
        ret = DebListModel::InstalledEarlierVersion;


    //存储包的安装状态
    //2020-11-19 修改安装状态的存储绑定方式
    m_packageInstallStatus[currentPackageMd5] = ret;
    return ret;
}

void PackagesManager::slotDealDependResult(int iAuthRes, int iIndex, QString dependName)
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
        emit signalEnableCloseButton(true);
    }
    if (iAuthRes == DebListModel::AuthDependsErr) {
        for (int num = 0; num < m_dependInstallMark.size(); num++) {
            m_packageMd5DependsStatus[m_dependInstallMark.at(num)].status = DebListModel::DependsBreak;//更换依赖的存储结构
            if (!m_errorIndex.contains(m_dependInstallMark[num]))
                m_errorIndex.push_back(m_dependInstallMark[num]);
        }
        emit signalEnableCloseButton(true);
    }
    emit signalDependResult(iAuthRes, iIndex, dependName);
}

/**
 * @brief PackagesManager::getPackageMd5 获取某个包的md5 值
 * @param index 包的下表
 * @return  包的md5
 * 现在包的状态与md5绑定，下标再与md5绑定，而非直接与下标绑定
 * 这种做法的优点在于，不需要在前端再去调整所有包的顺序，只需要获取对应下标的md5即可,调整大多数状态不需要担心状态与下标对应错乱
 * 缺点是：每次获取状态都需要读md5.频繁读取会造成性能影响
 */
QByteArray PackagesManager::getPackageMd5(const int index)
{
    if (index < m_packageMd5.size()) {
        return m_packageMd5[index];
    }
    return nullptr;
}

/**
 * @brief PackagesManager::getPackageDependsStatus 获取某个包的依赖状态
 * @param index 包的下标
 * @return 包的依赖状态
 */
PackageDependsStatus PackagesManager::getPackageDependsStatus(const int index)
{
    //提前获取需要的md5
    auto currentPackageMd5 = m_packageMd5[index];

    if (m_packageMd5DependsStatus.contains(currentPackageMd5)) {
        return m_packageMd5DependsStatus[currentPackageMd5];
    }

    DebFile debFile(m_preparedPackages[index]);
    if(!debFile.isValid())
        return PackageDependsStatus::_break("");
    const QString architecture = debFile.architecture();
    PackageDependsStatus dependsStatus = PackageDependsStatus::ok();

    if (isArchError(index)) {
        dependsStatus.status = DebListModel::ArchBreak;       //添加ArchBreak错误。
        dependsStatus.package = debFile.packageName();
        m_packageMd5DependsStatus.insert(currentPackageMd5, dependsStatus);//更换依赖的存储方式
        
        QString packageName = debFile.packageName();
        return PackageDependsStatus::_break(packageName);
    }

    // conflicts
    const ConflictResult debConflitsResult = isConflictSatisfy(architecture, debFile.conflicts());

    if (!debConflitsResult.is_ok()) {
        qWarning() << "PackagesManager:" << "depends break because conflict" << debFile.packageName();
        dependsStatus.package = debConflitsResult.unwrap();
        dependsStatus.status = DebListModel::DependsBreak;
    } else {
        const ConflictResult localConflictsResult =
            isInstalledConflict(debFile.packageName(), debFile.version(), architecture);
        if (!localConflictsResult.is_ok()) {
            qWarning() << "PackagesManager:" << "depends break because conflict with local package" << debFile.packageName();
            dependsStatus.package = localConflictsResult.unwrap();
            dependsStatus.status = DebListModel::DependsBreak;
        } else {
            QSet<QString> choose_set;
            choose_set << debFile.packageName();
            QStringList dependList;
            bool isWineApplication = false;             //判断是否是wine应用
            for (auto ditem : debFile.depends()) {
                for (auto dinfo : ditem) {
                    Package *depend = packageWithArch(dinfo.packageName(), debFile.architecture());
                    if (depend) {
                        if (depend->name() == "deepin-elf-verify") {    //deepi-elf-verify 是amd64架构非i386
                            dependList << depend->name();
                        } else {
                            dependList << depend->name() + ":" + depend->architecture();
                        }

                        depend = nullptr;

                        if (dinfo.packageName().contains("deepin-wine")) {              // 如果依赖中出现deepin-wine字段。则是wine应用
                            isWineApplication = true;
                        }
                    }
                }
            }
            dependsStatus = checkDependsPackageStatus(choose_set, debFile.architecture(), debFile.depends());
            // 删除无用冗余的日志
            //由于卸载p7zip会导致wine依赖被卸载，再次安装会造成应用闪退，因此判断的标准改为依赖不满足即调用pkexec
            if (isWineApplication && dependsStatus.status != DebListModel::DependsOk) {               //增加是否是wine应用的判断
                
                if (!m_dependInstallMark.contains(currentPackageMd5)) {           //更换判断依赖错误的标记
                    if (!m_installWineThread->isRunning()) {
                        m_dependInstallMark.append(currentPackageMd5);            //依赖错误的软件包的标记 更改为md5取代验证下标
                        qInfo() << "PackagesManager:" << "command install depends:" << dependList;
                        m_installWineThread->setDependsList(dependList, index);
                        m_installWineThread->setBrokenDepend(dependsStatus.package);
                        m_installWineThread->run();
                    }
                }
                dependsStatus.status = DebListModel::DependsBreak;                                    //只要是下载，默认当前wine应用依赖为break
            }
        }
    }
    if (dependsStatus.isBreak()) 
        Q_ASSERT(!dependsStatus.package.isEmpty());

    m_packageMd5DependsStatus.insert(currentPackageMd5, dependsStatus);

    return dependsStatus;
}

const QString PackagesManager::packageInstalledVersion(const int index)
{
    //更换安装状态的存储结构
    DebFile debFile(m_preparedPackages[index]);
    if(!debFile.isValid())
        return "";
    const QString packageName = debFile.packageName();
    const QString packageArch = debFile.architecture();
    Backend *backend = m_backendFuture.result();
    Package *package = backend->package(packageName + ":" + packageArch);

    //修复可能某些包无法package的错误，如果遇到此类包，返回安装版本为空
    if (package)
        return package->installedVersion();   //能正常打包，返回包的安装版本
    else {
        return "";                      //此包无法正常package，返回空
    }
}

const QStringList PackagesManager::packageAvailableDepends(const int index)
{
    DebFile debFile(m_preparedPackages[index]);
    if(!debFile.isValid())
        return QStringList();
    QSet<QString> choose_set;
    const QString debArch = debFile.architecture();
    const auto &depends = debFile.depends();
    packageCandidateChoose(choose_set, debArch, depends);

    // TODO: check upgrade from conflicts
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
    for (const auto &info : candidateList) {
        Package *package = packageWithArch(info.packageName(), debArch, info.multiArchAnnotation());
        if (!package)
            continue;

        const auto choosed_name = package->name() + resolvMultiArchAnnotation(QString(), package->architecture());
        if (choosed_set.contains(choosed_name)){
            package = nullptr;
            break;
        }
        //当前依赖未安装，则安装当前依赖。
        if (package->installedVersion().isEmpty()) {
            choosed_set << choosed_name;
        } else {
            // 当前依赖已安装，判断是否需要升级
            //  修复升级依赖时，因为依赖包版本过低，造成安装循环。
            // 删除无用冗余的日志
            if (Package::compareVersion(package->installedVersion(), info.packageVersion()) < 0) {
                Backend *backend = m_backendFuture.result();
                if(!backend){
                    qWarning()<<"libqapt backend loading error";
                    package = nullptr;
                    return;
                }
                Package *updatePackage = backend->package(package->name()
                                                          + resolvMultiArchAnnotation(QString(), package->architecture()));
                if (updatePackage){
                    choosed_set << updatePackage->name() + resolvMultiArchAnnotation(QString(), package->architecture());
                    updatePackage = nullptr;
                }
                else{
                    choosed_set << info.packageName() + " not found";
                }
            } else { //若依赖包符合版本要求,则不进行升级
                package = nullptr;
                continue;
            }
        }

        if (!isConflictSatisfy(debArch, package->conflicts()).is_ok()){
            package = nullptr;
            continue;
        }

        QSet<QString> upgradeDependsSet = choosed_set;
        upgradeDependsSet << choosed_name;
        const auto stat = checkDependsPackageStatus(upgradeDependsSet, package->architecture(), package->depends());
        if (stat.isBreak()){
            package = nullptr;
            continue;
        }

        choosed_set << choosed_name;
        packageCandidateChoose(choosed_set, debArch, package->depends());

        package = nullptr;
        break;
    }

}

QMap<QString, QString> PackagesManager::specialPackage()
{
    QMap<QString, QString> sp;
    sp.insert("deepin-wine-plugin-virtual", "deepin-wine-helper");
    sp.insert("deepin-wine32", "deepin-wine");
    sp.insert("deepin-wine-helper", "deepin-wine-plugin");

    return sp;
}

const QStringList PackagesManager::packageReverseDependsList(const QString &packageName, const QString &sysArch)
{
    Package *package = packageWithArch(packageName, sysArch);
    if(!package){
        qWarning()<<"Failed to package from"<<packageName<<"with"<< sysArch;
        return {};
    }
    QStringList requiredList = package->requiredByList();

    package = nullptr;

    //确定和当前包存在直接或间接反向依赖的包的集合
    QSet<QString> reverseDependSet{packageName};

    // 存放当前需要验证反向依赖的包
    QQueue<QString> reverseQueue;

    for (const auto &requiredPackage : requiredList.toSet())
        reverseQueue.append(requiredPackage);
    while (!reverseQueue.isEmpty()) {
        const auto item = reverseQueue.first();
        reverseQueue.pop_front();

        if (reverseDependSet.contains(item))
            continue;

        Package *currentPackage = packageWithArch(item, sysArch);
        if (!currentPackage || !currentPackage->isInstalled()){
            currentPackage = nullptr;
            continue;
        }
        if (currentPackage->recommendsList().contains(packageName)){
            currentPackage = nullptr;
            continue;
        }
        if (currentPackage->suggestsList().contains(packageName)) {
            currentPackage = nullptr;
            continue;
        }

        reverseDependSet << item;

        if (specialPackage().contains(item))
            reverseQueue.append(specialPackage()[item]);

        // 判断当前反向依赖是否有反向依赖
        for (const auto &dependRequiredPackage : currentPackage->requiredByList()) {
            if (reverseDependSet.contains(dependRequiredPackage) || reverseQueue.contains(dependRequiredPackage)) continue;
            Package *subPackage = packageWithArch(dependRequiredPackage, sysArch);
            if (dependRequiredPackage.startsWith("deepin.")) {  // 此类wine应用在系统中的存在都是以deepin.开头
                // 部分wine应用在系统中有一个替换的名字，使用requiredByList 可以获取到这些名字
                if (subPackage && !subPackage->requiredByList().isEmpty()) {    //增加对package指针的检查
                    for (QString rdepends : subPackage->requiredByList()) {
                        reverseQueue.append(rdepends);
                    }
                }
            }
            if (!subPackage || !subPackage->isInstalled()){      //增加对package指针的检查
                subPackage = nullptr;
                continue;
            }
            if (subPackage->recommendsList().contains(item)){
                subPackage = nullptr;
                continue;
            }
            if (subPackage->suggestsList().contains(item)) {
                subPackage = nullptr;
                continue;
            }
            reverseQueue.append(dependRequiredPackage);
        }

        currentPackage = nullptr;
    }
    // remove self
    reverseDependSet.remove(packageName);

    return reverseDependSet.toList();
}

void PackagesManager::reset()
{
    m_errorIndex.clear();
    m_dependInstallMark.clear();
    m_preparedPackages.clear();
    m_packageInstallStatus.clear();
    m_packageMd5DependsStatus.clear();  //修改依赖状态的存储结构，此处清空存储的依赖状态数据
    m_appendedPackagesMd5.clear();
    m_packageMd5.clear();

    //reloadCache必须要加
    m_backendFuture.result()->reloadCache();
}

void PackagesManager::resetPackageDependsStatus(const int index)
{
    // 查看此包是否已经存储依赖状态。
    //提前获取package 的md5
    auto currentPackageMd5 = m_packageMd5[index];
    if (!m_packageMd5DependsStatus.contains(currentPackageMd5)) 
        return; 
    else {
        // 针对wine依赖做一个特殊处理，如果wine依赖break,则直接返回。
        if ((m_packageMd5DependsStatus[currentPackageMd5].package == "deepin-wine") 
               && m_packageMd5DependsStatus[currentPackageMd5].status != DebListModel::DependsOk)
            return;
    }
    // reload backend cache
    //reloadCache必须要加
    m_backendFuture.result()->reloadCache();;
    m_packageMd5DependsStatus.remove(currentPackageMd5);  //删除当前包的依赖状态（之后会重新获取此包的依赖状态）
}

/**
 * @brief PackagesManager::removePackage 删除指定下标的包
 * @param index 指定的下标
 */
void PackagesManager::removePackage(const int index)
{
    if (index < 0 || index >= m_preparedPackages.size()) {
        qWarning() << "[PackagesManager]" << "[removePackage]" << "Subscript boundary check error";
        return;
    }

    // 如果此前的文件已经被修改,则获取到的MD5的值与之前不同,因此从现有的md5中寻找.
    const auto md5 = m_packageMd5[index];

    //提前删除标记list中的md5 否则在删除最后一个的时候会崩溃
    if (m_dependInstallMark.contains(md5))      //如果这个包是wine包，则在wine标记list中删除
        m_dependInstallMark.removeOne(md5);

    m_appendedPackagesMd5.remove(md5);
    m_preparedPackages.removeAt(index);

    m_appendedPackagesMd5.remove(md5);          //在判断是否重复的md5的集合中删除掉当前包的md5
    m_packageMd5DependsStatus.remove(md5);      //删除指定包的依赖状态
    m_packageMd5.removeAt(index);                               //在索引map中删除指定的项

    m_packageInstallStatus.clear();

    // 告诉model md5更新了
    emit signalPackageMd5Changed(m_packageMd5);

    // 如果后端只剩余一个包,刷新单包安装界面
    if (m_preparedPackages.size() == 1) {
        emit signalRefreshSinglePage();
    } else if (m_preparedPackages.size() >= 2) {
        emit signalRefreshMultiPage();
    } else if (m_preparedPackages.size() == 0) {
        emit signalRefreshFileChoosePage();
    }
}

/**
 * @brief PackagesManager::appendPackage 将前端给的包，传输到添加线程中。并开始添加
 * @param packages 要添加的包的列表
 * 此处可以优化，如果只有一两个包直接添加
 * 大于等于三个包，先添加两个再开始线程
 */
void PackagesManager::appendPackage(QStringList packages)
{
    if (packages.isEmpty())//当前放进来的包列表为空（可能拖入的是文件夹）
        return;
    if (1 == packages.size()) {
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
        QApt::DebFile pkgFile(package);
        if (pkgFile.isValid())            //只有有效文件才会计入
            m_validPackageCount ++;
    }
}

/**
 * @brief PackagesManager::dealInvalidPackage 处理不在本地的安装包
 * @param packagePath 包的路径
 * @return 包是否在本地
 *   true   : 包在本地
 *   fasle  : 文件不在本地
 */
bool PackagesManager::dealInvalidPackage(QString packagePath)
{
    QStorageInfo info(packagePath);                               //获取路径信息

    if (!info.device().startsWith("/dev/")) {                            //判断路径信息是不是本地路径
        emit signalNotLocalPackage();
        return false;
    }
    return true;
}

/**
 * @brief PackagesManager::dealPackagePath 处理路径相关的问题
 * @param packagePath 当前包的文件路径
 * @return 处理后的文件路径
 * 处理两种情况
 *      1： 相对路径             --------> 转化为绝对路径
 *      2： 包的路径中存在空格     --------> 使用软链接，链接到/tmp下
 */
QString PackagesManager::dealPackagePath(QString packagePath)
{
    //判断当前文件路径是否是绝对路径，不是的话转换为绝对路径
    if (!packagePath.startsWith("/")) {
        QFileInfo packageAbsolutePath(packagePath);
        packagePath = packageAbsolutePath.absoluteFilePath();                           //获取绝对路径
        qInfo() << "get AbsolutePath" << packageAbsolutePath.absoluteFilePath();
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

/**
 * @brief PackagesManager::appendNoThread
 * @param packages
 * @param allPackageSize
 */
void PackagesManager::appendNoThread(QStringList packages, int allPackageSize)
{
    qDebug() << "[PackagesManager]" << "[appendNoThread]" << "start add packages";
    m_validPackageCount = 0;
    for (QString debPackage : packages) {                 //通过循环添加所有的包

        // 处理包不在本地的情况。
        if (!dealInvalidPackage(debPackage)) {
            continue;
        }

        //处理package文件路径相关问题
        debPackage = dealPackagePath(debPackage);

        QApt::DebFile pkgFile(debPackage);
        //判断当前文件是否是无效文件
        if (!pkgFile.isValid()) {
            emit signalInvalidPackage();
            continue;
        }
        // 获取当前文件的md5的值,防止重复添加
        const auto md5 = pkgFile.md5Sum();
        // 如果当前已经存在此md5的包,则说明此包已经添加到程序中
        if (m_appendedPackagesMd5.contains(md5)) {
            //处理重复文件
            emit signalPackageAlreadyExists();
            continue;
        }
        // 可以添加,发送添加信号

        //管理最近文件列表
        DRecentData data;
        data.appName = "Deepin Deb Installer";
        data.appExec = "deepin-deb-installer";
        DRecentManager::addItem(debPackage, data);
        m_validPackageCount++;
        addPackage(m_validPackageCount, debPackage, md5);
    }

    //所有包都添加结束.
    if (allPackageSize == 1) {
        // 添加一个包时 发送添加结束信号,启用安装按钮
        emit signalAppendFinished(m_packageMd5);
    }
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
            emit signalRefreshSinglePage();   //刷新单包安装界面

        } else if (validPkgCount > 1) {  //当前程序中值添加了一个包，但是这次有不止一个包将会被添加到程序中
            emit signalSingle2MultiPage();     //刷新批量安装界面
            emit signalAppendStart();          //开始批量添加
        }
    } else if (packageCount == 2) {
        //当前程序中已经添加了两个包
        //1.第一次是添加了一个包，第二次又添加了多于一个包

        emit signalSingle2MultiPage();        //刷新批量安装界面
        emit signalAppendStart();             //发送批量添加信号
    } else {
        //此时批量安装界面已经刷新过。如果再添加，就只刷新model
        emit signalRefreshMultiPage();
        emit signalAppendStart();
    }
}

/**
 * @brief PackagesManager::slotAppendPackageFinished 此次添加已经结束
 */
void PackagesManager::slotAppendPackageFinished()
{
    //告诉前端，此次添加已经结束
    //向model传递 md5
    emit signalAppendFinished(m_packageMd5);
}

void PackagesManager::addPackage(int validPkgCount, QString packagePath, QByteArray packageMd5Sum)
{
    //一定要保持 m_preparedPacjages 和 packageMd5的下标保持一致
    //二者一定是一一对应的。
    //此后的依赖状态和安装状态都是与md5绑定的 md5是与index绑定
    m_preparedPackages.insert(0, packagePath);      //每次添加的包都放到最前面
    m_packageMd5.insert(0, packageMd5Sum);          //添加MD5Sum
    m_appendedPackagesMd5 << packageMd5Sum;         //将MD5添加到集合中，这里是为了判断包不再重复
    getPackageDependsStatus(0);                     //刷新当前添加包的依赖
    refreshPage(validPkgCount);                     //添加后，根据添加的状态刷新界面
}

const PackageDependsStatus PackagesManager::checkDependsPackageStatus(QSet<QString> &choosed_set,
                                                                      const QString &architecture,
                                                                      const QList<DependencyItem> &depends)
{
    PackageDependsStatus dependsStatus = PackageDependsStatus::ok();

    for (const auto &candicate_list : depends) {
        const auto r = checkDependsPackageStatus(choosed_set, architecture, candicate_list);
        dependsStatus.maxEq(r);

        if (dependsStatus.isBreak()) break;
    }

    return dependsStatus;
}

const PackageDependsStatus PackagesManager::checkDependsPackageStatus(QSet<QString> &choosed_set,
                                                                      const QString &architecture,
                                                                      const DependencyItem &candicate)
{
    PackageDependsStatus dependsStatus = PackageDependsStatus::_break(QString());

    for (const auto &info : candicate) {
        const auto r = checkDependsPackageStatus(choosed_set, architecture, info);
        dependsStatus.minEq(r);

        if (!dependsStatus.isBreak()) break;
    }

    return dependsStatus;
}

const PackageDependsStatus PackagesManager::checkDependsPackageStatus(QSet<QString> &choosed_set,
                                                                      const QString &architecture,
                                                                      const DependencyInfo &dependencyInfo)
{
    const QString package_name = dependencyInfo.packageName();

    Package *package = packageWithArch(package_name, architecture, dependencyInfo.multiArchAnnotation());

    if (!package) {
        qWarning() << "PackagesManager:" << "depends break because package" << package_name << "not available";
        return PackageDependsStatus::_break(package_name);
    }

    const RelationType relation = dependencyInfo.relationType();
    const QString &installedVersion = package->installedVersion();

    if (!installedVersion.isEmpty()) {
        const int result = Package::compareVersion(installedVersion, dependencyInfo.packageVersion());
        if (dependencyVersionMatch(result, relation))
            return PackageDependsStatus::ok();
        else {
            const QString &mirror_version = package->availableVersion();
            if (mirror_version != installedVersion) {
                const auto mirror_result = Package::compareVersion(mirror_version, dependencyInfo.packageVersion());

                if (dependencyVersionMatch(mirror_result, relation)) {
                    qDebug() << "PackagesManager:" << "availble by upgrade package" << package->name() + ":" + package->architecture() << "from"
                             << installedVersion << "to" << mirror_version;
                    // 修复卸载p7zip导致deepin-wine-helper被卸载的问题，Available 添加packageName
                    return PackageDependsStatus::available(package->name());
                }
            }

            qDebug() << "PackagesManager:" << "depends break by" << package->name() << package->architecture() << dependencyInfo.packageVersion();
            qDebug() << "PackagesManager:" << "installed version not match" << installedVersion;
            return PackageDependsStatus::_break(package->name());
        }
    } else {
        const int result = Package::compareVersion(package->version(), dependencyInfo.packageVersion());
        if (!dependencyVersionMatch(result, relation)) {
            qDebug() << "PackagesManager:" << "depends break by" << package->name() << package->architecture() << dependencyInfo.packageVersion();
            qDebug() << "PackagesManager:" << "available version not match" << package->version();
            return PackageDependsStatus::_break(package->name());
        }

        // is that already choosed?
        if (choosed_set.contains(package->name())) return PackageDependsStatus::ok();

        // check arch conflicts
        if (package->multiArchType() == MultiArchSame) {
            Backend *backend = m_backendFuture.result();
            for (const auto &arch : backend->architectures()) {
                if (arch == package->architecture()) continue;

                Package *otherArchPackage = backend->package(package->name() + ":" + arch);
                if (otherArchPackage && otherArchPackage->isInstalled()) {
                    qDebug() << "PackagesManager:" << "multiple architecture installed: " << package->name() << package->version() << package->architecture() << "but now need"
                             << otherArchPackage->name() << otherArchPackage->version() << otherArchPackage->architecture();
                    return PackageDependsStatus::_break(package->name() + ":" + package->architecture());
                }
            }
        }
        // let's check conflicts
        if (!isConflictSatisfy(architecture, package).is_ok()) {
            qDebug() << "PackagesManager:" << "depends break because conflict, ready to find providers" << package->name();

            Backend *backend = m_backendFuture.result();
            for (auto *availablePackage : backend->availablePackages()) {
                if (!availablePackage->providesList().contains(package->name())) continue;

                // is that already provide by another package?
                if (availablePackage->isInstalled()) {
                    qDebug() << "PackagesManager:" << "find a exist provider: " << availablePackage->name();
                    return PackageDependsStatus::ok();
                }

                // provider is ok, switch to provider.
                if (isConflictSatisfy(architecture, availablePackage).is_ok()) {
                    qDebug() << "PackagesManager:" << "switch to depends a new provider: " << availablePackage->name();
                    choosed_set << availablePackage->name();
                    return PackageDependsStatus::ok();
                }
            }

            qDebug() << "PackagesManager:" << "providers not found, still break: " << package->name();
            return PackageDependsStatus::_break(package->name());
        }

        // now, package dependencies status is available or break,
        // time to check depends' dependencies, but first, we need
        // to add this package to choose list
        choosed_set << package->name();

        qDebug() << "PackagesManager:" << "Check indirect dependencies for package" << package->name();

        const auto dependsStatus = checkDependsPackageStatus(choosed_set, package->architecture(), package->depends());
        if (dependsStatus.isBreak()) {
            choosed_set.remove(package->name());
            qDebug() << "PackagesManager:" << "depends break by direct depends" << package->name() << package->architecture() << dependsStatus.package;
            return PackageDependsStatus::_break(package->name());
        }

        qDebug() << "PackagesManager:" << "Check finshed for package" << package->name();

        // 修复卸载p7zip导致deepin-wine-helper被卸载的问题，Available 添加packageName
        return PackageDependsStatus::available(package->name());
    }
}

Package *PackagesManager::packageWithArch(const QString &packageName, const QString &sysArch,
                                          const QString &annotation)
{
    Backend *backend = m_backendFuture.result();
    Package *package = backend->package(packageName + resolvMultiArchAnnotation(annotation, sysArch));
    do {
        // change: 按照当前支持的CPU架构进行打包。取消对deepin-wine的特殊处理
        if (!package) 
            package = backend->package(packageName);
        if (package) 
            break;
        for (QString arch : backend->architectures()) {
            if (!package) 
                package = backend->package(packageName + ":" + arch);
            if (package) 
                break;
        }

    } while (false);

    if (package) 
        return package;

    // check virtual package providers
    for (auto *virtualPackage : backend->availablePackages())
        if (virtualPackage->name() != packageName && virtualPackage->providesList().contains(packageName))
            return packageWithArch(virtualPackage->name(), sysArch, annotation);
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

QString PackagesManager::package(const int index) const
{
    return m_preparedPackages[index];
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
    delete m_installWineThread;

    //先取消当前异步计算的后端。
    m_backendFuture.cancel();
    delete m_pAddPackageThread;

    Backend *backend = m_backendFuture.result();

    // 删除与库的连接
    backend->deleteLater();
    delete backend;
}

