// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "packagesmanager.h"
#include "DealDependThread.h"
#include "PackageDependsStatus.h"
#include "AddPackageThread.h"
#include "utils/utils.h"
#include "model/deblistmodel.h"
#include "model/dependgraph.h"

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
bool PackagesManager::isArchMatches(QString sysArch, const QString &packageArch, const int multiArchType)
{
    Q_UNUSED(multiArchType);

    if (sysArch.startsWith(':'))
        sysArch.remove(0, 1);
    if ("all" == sysArch || "any" == sysArch)
        return true;

    // bug119619  安装包时，仓库中的依赖fcitx是全架构，导致系统架构与包架构不匹配
    // 增加依赖包为全架构的判断
    if ("all" == packageArch || "any" == packageArch)
        return true;

    return sysArch == packageArch;
}

QString PackagesManager::resolvMultiArchAnnotation(const QString &annotation, const QString &debArch, const int multiArchType)
{
    if ("native" == annotation || "any" == annotation)
        return QString();
    if ("all" == annotation)
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

bool PackagesManager::dependencyVersionMatch(const int result, const RelationType relation)
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
        return true;
    }
}

void PackagesManager::selectedIndexRow(int row)
{
    if (row < m_packageMd5.size() && row >= 0)
        emit signalMultDependPackages(m_dependsPackages.value(m_packageMd5[row]), installWineDepends);
}

int PackagesManager::checkInstallStatus(const QString &package_path)
{
    DebFile debFile(package_path);
    if (!debFile.isValid())
        return DebListModel::NotInstalled;
    const QString packageName = debFile.packageName();
    const QString packageArch = debFile.architecture();
    Backend *backend = m_backendFuture.result();
    if (!backend) {
        qWarning() << "Failed to load libqapt backend";
        return DebListModel::NotInstalled;
    }
    Package *package = packageWithArch(packageName, packageArch);


    if (!package)
        return DebListModel::NotInstalled;

    const QString installedVersion = package->installedVersion();
    package = nullptr;
    if (installedVersion.isEmpty())
        return DebListModel::NotInstalled;

    const QString packageVersion = debFile.version();
    const int result = Package::compareVersion(packageVersion, installedVersion);

    int ret;
    if (result == 0)
        ret = DebListModel::InstalledSameVersion;
    else if (result < 0)
        ret = DebListModel::InstalledLaterVersion;
    else
        ret = DebListModel::InstalledEarlierVersion;

    return ret;

}

PackageDependsStatus PackagesManager::checkDependsStatus(const QString &package_path)
{
    DebFile debFile(package_path);
    if (!debFile.isValid())
        return PackageDependsStatus::_break("");
    m_currentPkgName = debFile.packageName();
    m_orDepends.clear();
    m_checkedOrDependsStatus.clear();
    m_unCheckedOrDepends.clear();
    m_dependsInfo.clear();
    //用debFile.packageName()无法打开deb文件，故替换成debFile.filePath()
    //更新m_dependsInfo
    getPackageOrDepends(debFile.filePath(), debFile.architecture(), true);

    const QString architecture = debFile.architecture();
    PackageDependsStatus dependsStatus = PackageDependsStatus::ok();

    if (isBlackApplication(debFile.packageName())) {
        dependsStatus.status  = DebListModel::Prohibit;
        dependsStatus.package = debFile.packageName();
        qWarning() << debFile.packageName() << "In the blacklist";
        return dependsStatus;
    }

    if (isArchErrorQstring(package_path)) {
        dependsStatus.status = DebListModel::ArchBreak;       //添加ArchBreak错误。
        dependsStatus.package = debFile.packageName();
        QString packageName = debFile.packageName();
        return PackageDependsStatus::_break(packageName);
    }

    // conflicts
    const ConflictResult debConflitsResult = isConflictSatisfy(architecture, debFile.conflicts(), debFile.replaces());

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
            for (auto ditem : debFile.depends()) {      //每一个list中的关系是或的关系
                for (auto dinfo : ditem) {
                    Package *depend = packageWithArch(dinfo.packageName(), debFile.architecture());
                    if (depend) {
                        if (depend->name() == "deepin-elf-verify")   //deepi-elf-verify 是amd64架构非i386
                            dependList << depend->name();
                        else
                            dependList << depend->name() + ":" + depend->architecture();

                        depend = nullptr;

                        if (dinfo.packageName().contains("deepin-wine"))             // 如果依赖中出现deepin-wine字段。则是wine应用
                            isWineApplication = true;

                    }
                }
            }
            installWineDepends = false; //标记wine依赖下载线程开启
            isDependsExists = false; //标记多架构依赖冲突
            m_pair.first.clear(); //清空available依赖
            m_pair.second.clear(); //清空broken依赖
            if (m_dependsPackages.contains(m_currentPkgMd5))
                m_dependsPackages.remove(m_currentPkgMd5);

            dependsStatus = checkDependsPackageStatus(choose_set, debFile.architecture(), debFile.depends());
            // 删除无用冗余的日志
            //由于卸载p7zip会导致wine依赖被卸载，再次安装会造成应用闪退，因此判断的标准改为依赖不满足即调用pkexec
            //wine应用+非wine依赖不满足即可导致出问题
            do {
                if (isWineApplication && dependsStatus.status != DebListModel::DependsOk) {               //增加是否是wine应用的判断
                    //额外判断wine依赖是否已安装，同时剔除非wine依赖
                    auto removedIter = std::remove_if(dependList.begin(), dependList.end(), [&debFile, this](const QString & eachDepend) {
                        if (!eachDepend.contains("deepin-wine")) {
                            return true;
                        }
                        auto package = packageWithArch(eachDepend, debFile.architecture());
                        return !package->installedVersion().isEmpty();
                    });
                    dependList.erase(removedIter, dependList.end());
                    if (dependList.isEmpty()) { //所有的wine依赖均已安装
                        break;
                    }
                    dependsStatus.status = DebListModel::DependsBreak;                                    //只要是下载，默认当前wine应用依赖为break
                }
            } while (0);
        }
    }
    if (dependsStatus.isBreak())
        Q_ASSERT(!dependsStatus.package.isEmpty());

    return dependsStatus;
}

QStringList PackagesManager::getPackageInfo(const QString &package_path)
{
    QStringList value_list;
    const DebFile deb(package_path);
    if (!deb.isValid())
        return value_list;
    QString packageName = deb.packageName(); //包名
    QString filePath = deb.filePath(); //包的路径
    QString version = deb.version(); //包的版本
    QString architecture = deb.architecture(); //包可用的架构
    QString shortDescription = deb.shortDescription(); //包的短描述
    QString longDescription = deb.longDescription(); //包的长描述
    value_list << packageName << filePath << version << architecture << shortDescription << longDescription;
    return value_list;
}

QString PackagesManager::checkPackageValid(const QStringList &package_path)
{
    for (QString debPackage : package_path) {                 //通过循环添加所有的包
        QStorageInfo info(debPackage);                               //获取路径信息
        QString device = info.device();   //获取设备信息
        // 处理包不在本地的情况。
        if (!device.startsWith("/dev/") && device != QString::fromLocal8Bit("tmpfs")) {  //判断路径信息是不是本地路径
            return "You can only install local deb packages";
        }
        QString debPkg = debPackage;
        //处理package文件路径相关问题
        debPackage = dealPackagePath(debPackage);

        QApt::DebFile pkgFile(debPackage);
        //判断当前文件是否是无效文件
        if (!pkgFile.isValid()) {
            return "The deb package may be broken";
        }
        //获取当前文件的md5的值,防止重复添加
        //在checkInvalid中已经获取过md5,避免2次获取影响性能
        QByteArray md5 = m_allPackages.value(debPkg);
        if (md5.isEmpty())
            md5 = pkgFile.md5Sum();

        // 如果当前已经存在此md5的包,则说明此包已经添加到程序中
        if (m_appendedPackagesMd5.contains(md5)) {
            return "The deb package Already Added";
        }
    }
    return "";
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

    getBlackApplications();
}

bool PackagesManager::isBackendReady()
{
    return m_backendFuture.isFinished();
}

bool PackagesManager::isArchError(const int idx)
{
    if (idx < 0 || idx >= m_preparedPackages.size())
        return true;

    Backend *backend = m_backendFuture.result();
    if (!backend) {
        qWarning() << "Failed to load libqapt backend";
        return true;
    }
    DebFile deb(m_preparedPackages[idx]);

    if (!deb.isValid())
        return false;

    const QString arch = deb.architecture();

    if ("all" == arch || "any" == arch)
        return false;

    bool architectures = !backend->architectures().contains(deb.architecture());

    return architectures;
}

bool PackagesManager::isArchErrorQstring(const QString &package_name)
{
    Backend *backend = m_backendFuture.result();
    if (!backend) {
        qWarning() << "Failed to load libqapt backend";
        return true;
    }
    DebFile deb(package_name);

    if (!deb.isValid())
        return false;

    const QString arch = deb.architecture();

    if ("all" == arch || "any" == arch)
        return false;

    bool architectures = !backend->architectures().contains(deb.architecture());

    return architectures;
}

const ConflictResult PackagesManager::packageConflictStat(const int index)
{
    if (index < 0 || index >= m_preparedPackages.size())
        return ConflictResult::err("");

    DebFile debfile(m_preparedPackages[index]);
    if (!debfile.isValid())
        return ConflictResult::err("");
    ConflictResult ConflictResult = isConflictSatisfy(debfile.architecture(), debfile.conflicts());
    return ConflictResult;
}

const ConflictResult PackagesManager::isConflictSatisfy(const QString &arch, Package *package)
{
    if (!package) {
        qWarning() << "invalid package pointer";
        return ConflictResult::err("");
    }

    const QString &packageName = package->name();
    const auto ret_installed = isInstalledConflict(packageName, package->version(), package->architecture());
    if (!ret_installed.is_ok()) {
        qWarning() << packageName << "check installed conflict not satisfied";
        return ret_installed;
    }

    const auto conflictStatus = isConflictSatisfy(arch, package->conflicts());

    return conflictStatus;
}

const ConflictResult PackagesManager::isInstalledConflict(const QString &packageName, const QString &packageVersion,
                                                          const QString &packageArch)
{
    static QList<QPair<QLatin1String, DependencyInfo>> sysConflicts;

    if (sysConflicts.isEmpty()) {
        Backend *backend = m_backendFuture.result();
        for (Package *pkg : backend->availablePackages()) {
            if (!pkg)
                continue;
            if (!pkg->isInstalled()) {
                pkg = nullptr;
                continue;
            }
            const auto &conflicts = pkg->conflicts();
            if (conflicts.isEmpty()) {
                pkg = nullptr;
                continue;
            }

            for (const auto &conflict_list : conflicts)
                for (const auto &conflict : conflict_list) {
                    sysConflicts << QPair<QLatin1String, DependencyInfo>(pkg->name(), conflict);
                }
            pkg = nullptr;
        }
    }

    Package *pkg = packageWithArch(packageName, packageArch);
    if (pkg && pkg->installedVersion() == packageVersion)
        return ConflictResult::ok(QString());

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

const ConflictResult PackagesManager::isConflictSatisfy(const QString &arch,
                                                        const QList<DependencyItem> &conflicts,
                                                        const QList<DependencyItem> &replaces)
{
    for (const auto &conflict_list : conflicts) {
        for (const auto &conflict : conflict_list) {
            const QString name = conflict.packageName();

            //修复依赖中 conflict与provides 存在相同 virtual package
            //此前使用packageWithArch, 在package打包失败时，会寻找virtual package的提供的其他包
            //在dde-daemon中 lastore-daemon-migration与dde-daemon为conflict,
            //lastore-daemon-migration 又提供了dde-daemon,导致最后打成的包不是virtual package而是provides的包
            Backend *backend = m_backendFuture.result();
            if (!backend)
                return ConflictResult::err(QString());
            Package *package = backend->package(name);

            if (!package)
                continue;

            if (!package->isInstalled()) {
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
            if (dependencyVersionMatch(mirror_result, type) && name != m_currentPkgName) { //此处即可确认冲突成立
                //额外判断是否会替换此包
                bool conflict_yes = true;
                for (auto replace_list : replaces) {
                    for (auto replace : replace_list) {
                        if (replace.packageName() == name) { //包名符合
                            auto replaceType = replace.relationType(); //提取版本号规则
                            auto versionCompare = Package::compareVersion(installed_version, replace.packageVersion()); //比较版本号
                            if (dependencyVersionMatch(versionCompare, replaceType)) { //如果版本号符合要求，即判定replace成立
                                conflict_yes = false;
                                break;
                            }
                        }
                    }
                    if (!conflict_yes) {
                        break;
                    }
                }

                if (!conflict_yes) {
                    package = nullptr;
                    continue;
                }

                qWarning() << "PackagesManager:" <<  "conflicts package installed: "
                           << arch << package->name() << package->architecture()
                           << package->multiArchTypeString() << mirror_version << conflict_version;
                return ConflictResult::err(name);
            }

        }
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
            if (!backend)
                return ConflictResult::err(QString());
            Package *package = backend->package(name);

            if (!package)
                continue;

            if (!package->isInstalled()) {
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
            if (dependencyVersionMatch(mirror_result, type) && name != m_currentPkgName) {
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
    if (index < 0 || index >= m_preparedPackages.size())
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
    if (!backend) {
        qWarning() << "Failed to load libqapt backend";
        return DebListModel::NotInstalled;
    }
    Package *package = packageWithArch(packageName, packageArch);


    if (!package)
        return DebListModel::NotInstalled;

    const QString installedVersion = package->installedVersion();
    package = nullptr;
    if (installedVersion.isEmpty())
        return DebListModel::NotInstalled;

    const QString packageVersion = debFile.version();
    const int result = Package::compareVersion(packageVersion, installedVersion);

    int ret;
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
    if (iIndex < 0 || iIndex > m_preparedPackages.size())
        return;
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
    if (iAuthRes == DebListModel::AuthDependsErr || iAuthRes == DebListModel::AnalysisErr) {
        for (int num = 0; num < m_dependInstallMark.size(); num++) {
            m_packageMd5DependsStatus[m_dependInstallMark.at(num)].status = DebListModel::DependsBreak;//更换依赖的存储结构
            if (!m_errorIndex.contains(m_dependInstallMark[num]))
                m_errorIndex.push_back(m_dependInstallMark[num]);
        }
        if (installWineDepends) { //下载wine依赖失败时，考虑出现依赖缺失的情况
            qInfo() << "check wine depends again !" << iIndex;
            getPackageDependsStatus(iIndex);
            if (!m_dependsPackages.isEmpty()) {
                qInfo() << m_dependsPackages.size() << m_dependsPackages.value(m_currentPkgMd5).second.size();
                if (1 == m_preparedPackages.size())
                    emit signalSingleDependPackages(m_dependsPackages.value(m_currentPkgMd5), false);
                else if (m_preparedPackages.size() > 1)
                    installWineDepends = false;
            }
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
    if (index < m_packageMd5.size())
        return m_packageMd5[index];
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
    if (index < 0 || index > m_preparedPackages.size()) {
        qWarning() << "invalid param index";
        return PackageDependsStatus::_break("");
    }
    auto currentPackageMd5 = m_packageMd5[index];
    m_currentPkgMd5 = currentPackageMd5;

    if (m_packageMd5DependsStatus.contains(currentPackageMd5))
        return m_packageMd5DependsStatus[currentPackageMd5];


    DebFile debFile(m_preparedPackages[index]);
    if (!debFile.isValid())
        return PackageDependsStatus::_break("");
    m_currentPkgName = debFile.packageName();
    m_orDepends.clear();
    m_checkedOrDependsStatus.clear();
    m_unCheckedOrDepends.clear();
    m_dependsInfo.clear();
    //用debFile.packageName()无法打开deb文件，故替换成debFile.filePath()
    //更新m_dependsInfo
    getPackageOrDepends(debFile.filePath(), debFile.architecture(), true);

    const QString architecture = debFile.architecture();
    PackageDependsStatus dependsStatus = PackageDependsStatus::ok();

    if (isBlackApplication(debFile.packageName())) {
        dependsStatus.status  = DebListModel::Prohibit;
        dependsStatus.package = debFile.packageName();
        m_packageMd5DependsStatus.insert(currentPackageMd5, dependsStatus);
        qWarning() << debFile.packageName() << "In the blacklist";
        return dependsStatus;
    }

    if (isArchError(index)) {
        dependsStatus.status = DebListModel::ArchBreak;       //添加ArchBreak错误。
        dependsStatus.package = debFile.packageName();
        m_packageMd5DependsStatus.insert(currentPackageMd5, dependsStatus);//更换依赖的存储方式
        QString packageName = debFile.packageName();
        return PackageDependsStatus::_break(packageName);
    }

    // conflicts
    const ConflictResult debConflitsResult = isConflictSatisfy(architecture, debFile.conflicts(), debFile.replaces());

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
            for (auto ditem : debFile.depends()) {      //每一个list中的关系是或的关系
                for (auto dinfo : ditem) {
                    Package *depend = packageWithArch(dinfo.packageName(), debFile.architecture());
                    if (depend) {
                        if (depend->name() == "deepin-elf-verify")   //deepi-elf-verify 是amd64架构非i386
                            dependList << depend->name();
                        else
                            dependList << depend->name() + ":" + depend->architecture();

                        depend = nullptr;

                        if (dinfo.packageName().contains("deepin-wine"))             // 如果依赖中出现deepin-wine字段。则是wine应用
                            isWineApplication = true;

                    }
                }
            }
            installWineDepends = false; //标记wine依赖下载线程开启
            isDependsExists = false; //标记多架构依赖冲突
            m_pair.first.clear(); //清空available依赖
            m_pair.second.clear(); //清空broken依赖
            if (m_dependsPackages.contains(m_currentPkgMd5))
                m_dependsPackages.remove(m_currentPkgMd5);

            dependsStatus = checkDependsPackageStatus(choose_set, debFile.architecture(), debFile.depends());
            // 删除无用冗余的日志
            //由于卸载p7zip会导致wine依赖被卸载，再次安装会造成应用闪退，因此判断的标准改为依赖不满足即调用pkexec
            //wine应用+非wine依赖不满足即可导致出问题
            do {
                if (isWineApplication && dependsStatus.status != DebListModel::DependsOk) {               //增加是否是wine应用的判断
                    //额外判断wine依赖是否已安装，同时剔除非wine依赖
                    auto removedIter = std::remove_if(dependList.begin(), dependList.end(), [&debFile, this](const QString & eachDepend) {
                        if (!eachDepend.contains("deepin-wine")) {
                            return true;
                        }
                        auto package = packageWithArch(eachDepend, debFile.architecture());
                        return !package->installedVersion().isEmpty();
                    });
                    dependList.erase(removedIter, dependList.end());
                    if (dependList.isEmpty()) { //所有的wine依赖均已安装
                        break;
                    }

                    if (!m_dependInstallMark.contains(currentPackageMd5)) {           //更换判断依赖错误的标记
                        installWineDepends = true;
                        if (!m_installWineThread->isRunning()) {
                            m_dependInstallMark.append(currentPackageMd5);            //依赖错误的软件包的标记 更改为md5取代验证下标
                            qInfo() << "PackagesManager:" << "command install depends:" << dependList;
                            m_installWineThread->setDependsList(dependList, index);
                            if (m_brokenDepend.isEmpty())
                                m_brokenDepend = dependsStatus.package;
                            m_installWineThread->setBrokenDepend(m_brokenDepend);
                            m_installWineThread->run();
                        }
                    }
                    dependsStatus.status = DebListModel::DependsBreak;                                    //只要是下载，默认当前wine应用依赖为break
                }
            } while (0);
        }
    }
    if (dependsStatus.isBreak())
        Q_ASSERT(!dependsStatus.package.isEmpty());

    m_packageMd5DependsStatus.insert(currentPackageMd5, dependsStatus);
    return dependsStatus;
}

void PackagesManager::getPackageOrDepends(const QString &package, const QString &arch, bool flag)
{
    /*
         * 解析安装包依赖，若存在或依赖关系则进行处理并且存储
         *such as,teamviewer depends: "libc6 (>= 2.17), libdbus-1-3, libqt5gui5 (>= 5.5)| qt56-teamviewer,
         * libqt5widgets5 (>= 5.5) | qt56-teamviewer, libqt5qml5 (>= 5.5) | qt56-teamviewer, libqt5quick5 (>= 5.5) | qt56-teamviewer..."
        */

    //更新m_dependsInfo
    auto insertToDependsInfo = [this](const QList<DependencyItem> &depends) {
        for (auto candicate_list : depends) {
            for (const auto &info : candicate_list) {
                m_dependsInfo.insert(info.packageName(), info);
            }
        }
    };
    QString controlDepends;
    if (flag) {
        DebFile debFile(package);
        if (!debFile.isValid())
            return;
        controlDepends = debFile.controlField("Depends");
        //软件包
        insertToDependsInfo(debFile.depends());
    } else {
        QApt::Package *pkg = packageWithArch(package, arch);
        if (!pkg)
            return;
        controlDepends = pkg->controlField("Depends");
        //子依赖
        insertToDependsInfo(pkg->depends());
    }
    qInfo() << __func__ << controlDepends;
    QStringList dependsList = controlDepends.split(",");

    auto removedIter = std::remove_if(dependsList.begin(), dependsList.end(), [](const QString & str) {
        return !str.contains("|");
    });
    dependsList.erase(removedIter, dependsList.end());

    //使用二维数组进行存储
    for (QString depend : dependsList) {
        depend = depend.remove(QRegExp("\\s"));
        QStringList orDepends = depend.split("|");
        QVector<QString> dependStatus;
        for (QString ordepend : orDepends) {
            //截取依赖包名
            if (ordepend.contains("(")) {
                int mid = ordepend.indexOf("(");
                ordepend = ordepend.left(mid);
            }
            dependStatus.append(ordepend);
        }
        m_orDepends.append(dependStatus);
        m_unCheckedOrDepends.append(dependStatus);
    }
    qInfo() << __func__ << m_orDepends;
}

const QString PackagesManager::packageInstalledVersion(const int index)
{
    //更换安装状态的存储结构
    DebFile debFile(m_preparedPackages[index]);
    if (!debFile.isValid())
        return "";
    const QString packageName = debFile.packageName();
    const QString packageArch = debFile.architecture();
    Backend *backend = m_backendFuture.result();
    if (!backend) {
        qWarning() << "libqapt backend loading error";
        return "";
    }
    Package *package = backend->package(packageName + ":" + packageArch);

    //修复可能某些包无法package的错误，如果遇到此类包，返回安装版本为空
    if (package) {
        return package->installedVersion();   //能正常打包，返回包的安装版本
    } else
        return "";                      //此包无法正常package，返回空
}

const QStringList PackagesManager::packageAvailableDepends(const int index)
{
    DebFile debFile(m_preparedPackages[index]);
    if (!debFile.isValid())
        return QStringList();
    QSet<QString> choose_set;
    const QString debArch = debFile.architecture();
    const auto &depends = debFile.depends();
    m_unCheckedOrDepends = m_orDepends;
    packageCandidateChoose(choose_set, debArch, depends);

    // TODO: check upgrade from conflicts
    return choose_set.toList();
}

void PackagesManager::packageCandidateChoose(QSet<QString> &choosed_set, const QString &debArch,
                                             const QList<DependencyItem> &dependsList)
{
    for (auto const &candidate_list : dependsList)
        packageCandidateChoose(choosed_set, debArch, candidate_list);
}

void PackagesManager::packageCandidateChoose(QSet<QString> &choosed_set, const QString &debArch,
                                             const DependencyItem &candidateList)
{
    for (const auto &info : candidateList) {
        Package *package = packageWithArch(info.packageName(), debArch, info.multiArchAnnotation());
        if (!package)
            continue;

        const auto choosed_name = package->name() + resolvMultiArchAnnotation(QString(), package->architecture());
        if (choosed_set.contains(choosed_name))
            break;

        QVector<QString> infos;
        if (!m_unCheckedOrDepends.isEmpty()) {
            for (auto dInfo : m_unCheckedOrDepends) { //遍历或依赖容器中容器中是否存在当前依赖
                if (!dInfo.contains(package->name())) {
                    continue;
                } else {
                    infos = dInfo;
                    m_unCheckedOrDepends.removeOne(dInfo);
                }
            }
        }
        qInfo() << __func__ << infos << package->name();
        if (infos.isEmpty()) { //没有或依赖关系或者当前依赖不属于或依赖关系
            qInfo() << "not ordepends or not contain depend";
            //当前依赖未安装，则安装当前依赖。
            if (package->installedVersion().isEmpty()) {
                choosed_set << choosed_name;
            } else {
                // 当前依赖已安装，判断是否需要升级
                //  修复升级依赖时，因为依赖包版本过低，造成安装循环。
                // 删除无用冗余的日志
                if (Package::compareVersion(package->installedVersion(), info.packageVersion()) < 0) {
                    Backend *backend = m_backendFuture.result();
                    if (!backend) {
                        qWarning() << "libqapt backend loading error";
                        return;
                    }
                    Package *updatePackage = backend->package(package->name()
                                                              + resolvMultiArchAnnotation(QString(), package->architecture()));
                    if (updatePackage)
                        choosed_set << updatePackage->name() + resolvMultiArchAnnotation(QString(), package->architecture());
                    else
                        choosed_set << info.packageName() + " not found";

                } else { //若依赖包符合版本要求,则不进行升级
                    continue;
                }
            }
        } else { // 存在或依赖且当前依赖属于或依赖关系
            bool isInstalling = false;
            for (auto iter = infos.begin(); iter != infos.end(); iter++) {
                Backend *backend = m_backendFuture.result();
                if (!backend) {
                    qWarning() << "libqapt backend loading error";
                    return;
                }
                Package *otherPackage = backend->package(*iter + resolvMultiArchAnnotation(QString(), debArch));
                if (!otherPackage)
                    continue;
                qInfo() << __func__ << *iter << otherPackage->installedVersion() << m_dependsInfo[*iter].packageVersion();
                if (otherPackage->compareVersion(otherPackage->installedVersion(), m_dependsInfo[*iter].packageVersion()) >= 0 && !otherPackage->installedVersion().isEmpty()) {
                    //如果或依赖中有依赖已安装且符合版本要求，则当前依赖不进行下载
                    isInstalling = true;
                    break;
                }
            }
            if (!isInstalling) { //若或依赖中其他依赖也需要下载或者存在缺失，则当前依赖需要下载
                qInfo() << __func__ << isInstalling << "need to install current depend";
                if (package->installedVersion().isEmpty()) {
                    choosed_set << choosed_name;
                } else {
                    if (Package::compareVersion(package->installedVersion(), info.packageVersion()) < 0) {
                        Backend *backend = m_backendFuture.result();
                        if (!backend)
                            return;
                        Package *updatePackage = backend->package(package->name()
                                                                  + resolvMultiArchAnnotation(QString(), package->architecture()));
                        if (updatePackage)
                            choosed_set << updatePackage->name() + resolvMultiArchAnnotation(QString(), package->architecture());
                        else
                            choosed_set << info.packageName() + " not found";

                    } else {
                        continue;
                    }
                }
            } else {
                qInfo() << __func__ << isInstalling << "other ordepend is installed";
                break;
            }
        }

        if (!isConflictSatisfy(debArch, package->conflicts()).is_ok())
            continue;

        QSet<QString> upgradeDependsSet = choosed_set;
        upgradeDependsSet << choosed_name;
        const auto stat = checkDependsPackageStatus(upgradeDependsSet, package->architecture(), package->depends());
        if (stat.isBreak())
            continue;

        choosed_set << choosed_name;
        packageCandidateChoose(choosed_set, debArch, package->depends());
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
    if (!package) {
        qWarning() << "Failed to package from" << packageName << "with" << sysArch;
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
        if (!currentPackage || !currentPackage->isInstalled()) {
            currentPackage = nullptr;
            continue;
        }
        if (currentPackage->recommendsList().contains(packageName)) {
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
            if (!subPackage || !subPackage->isInstalled()) {     //增加对package指针的检查
                subPackage = nullptr;
                continue;
            }
            if (subPackage->recommendsList().contains(item)) {
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
    m_dependGraph.reset();

    //reloadCache必须要加
    m_backendFuture.result()->reloadCache();
    m_dependsPackages.clear();
}

void PackagesManager::resetPackageDependsStatus(const int index)
{
    // 查看此包是否已经存储依赖状态。
    //提前获取package 的md5
    auto currentPackageMd5 = m_packageMd5[index];
    if (!m_packageMd5DependsStatus.contains(currentPackageMd5)) {
        return;
    } else {
        // 针对wine依赖做一个特殊处理，如果wine依赖break,则直接返回。
        if ((m_packageMd5DependsStatus[currentPackageMd5].package == "deepin-wine")
                && m_packageMd5DependsStatus[currentPackageMd5].status != DebListModel::DependsOk)
            return;
    }
    // reload backend cache
    //reloadCache必须要加
    m_backendFuture.result()->reloadCache();
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
    m_dependsPackages.remove(md5); //删除指定包的依赖关系

    m_dependGraph.remove(md5); //从依赖关系图中删除对应节点

    m_packageInstallStatus.clear();

    // 告诉model md5更新了
    emit signalPackageMd5Changed(m_packageMd5);

    // 如果后端只剩余一个包,刷新单包安装界面
    if (1 == m_preparedPackages.size()) {
        emit signalRefreshSinglePage();
        if (m_packageMd5.size() != 1)
            return;
        emit signalSingleDependPackages(m_dependsPackages.value(m_packageMd5.at(0)), installWineDepends);
    } else if (m_preparedPackages.size() >= 2) {
        emit signalRefreshMultiPage();
    } else if (0 == m_preparedPackages.size()) {
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
    checkInvalid(packages);
    if (1 == packages.size()) {
        appendNoThread(packages, packages.size());
    } else {
        QStringList subPackages;
        subPackages << packages[0];
        appendNoThread(subPackages, packages.size());
        packages.removeAt(0);

        if (packages.isEmpty())
            return;
        if (!m_allPackages.isEmpty())
            m_pAddPackageThread->setSamePackageMd5(m_allPackages);
        m_pAddPackageThread->setPackages(packages, m_validPackageCount); //传递要添加的包到添加线程中
        m_pAddPackageThread->setAppendPackagesMd5(m_appendedPackagesMd5);       //传递当前已经添加的包的MD5 判重时使用

        m_pAddPackageThread->start();   //开始添加线程
    }
}

/**
 * @brief AddPackageThread::checkInvalid 检查有效文件的数量
 */
void PackagesManager::checkInvalid(QStringList packages)
{
    m_allPackages.clear();
    m_validPackageCount = packages.size();
    int validCount = 0; //计入有效包的数量
    QSet<qint64> pkgSize; //存储安装包的安装大小,初步去除无效包以及可能重复的包
    for (auto package : packages) {
        QApt::DebFile file(package);
        if (!file.isValid()) {
            m_validPackageCount--;
            continue;
        }
        validCount++;
        auto size = file.installedSize();
        if (pkgSize.contains(size)) {
            m_validPackageCount--;
            continue;
        }
        pkgSize << size;
    }
    //m_validPackageCount==1,可能有效包都是重复包，需要最终根据md5值来判定
    QSet<QByteArray> pkgMd5; //最后通过md5来区分是否是重复包
    if (1 == m_validPackageCount && validCount > 1) {
        for (auto package : packages) {
            QApt::DebFile pkgFile(package);
            if (!pkgFile.isValid())
                continue;
            auto md5 = pkgFile.md5Sum();
            m_allPackages.insert(package, md5);
            if (!pkgMd5.isEmpty() && !pkgMd5.contains(md5)) { //根据md5判断，有不是重复的包，刷新批量界面
                m_validPackageCount = 2;
                break;
            }
            pkgMd5 << md5;
        }
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

    QString device = info.device();                               //获取设备信息
    if (!device.startsWith("/dev/") && device != QString::fromLocal8Bit("tmpfs")) {  //判断路径信息是不是本地路径
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
    for (QString debPackage : packages) {                 //通过循环添加所有的包

        // 处理包不在本地的情况。
        if (!dealInvalidPackage(debPackage))
            continue;
        QString debPkg = debPackage;
        //处理package文件路径相关问题
        debPackage = dealPackagePath(debPackage);

        QApt::DebFile pkgFile(debPackage);
        //判断当前文件是否是无效文件
        if (!pkgFile.isValid()) {
            emit signalInvalidPackage();
            continue;
        }
        //获取当前文件的md5的值,防止重复添加
        //在checkInvalid中已经获取过md5,避免2次获取影响性能
        QByteArray md5 = m_allPackages.value(debPkg);
        if (md5.isEmpty())
            md5 = pkgFile.md5Sum();

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

        addPackage(m_validPackageCount, debPackage, md5);
    }

    //所有包都添加结束.
    if (1 == allPackageSize) {
        emit signalAppendFinished(m_packageMd5);// 添加一个包时 发送添加结束信号,启用安装按钮

        if (!m_dependsPackages.isEmpty() && 1 == m_preparedPackages.size()) //过滤掉依赖冲突的包不显示依赖关系的情况
            emit signalSingleDependPackages(m_dependsPackages.value(m_currentPkgMd5), installWineDepends);
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
    if (1 == packageCount) { //当前程序中只添加了一个包
        if (1 == validPkgCount) { //此次只有一个包将会被添加的程序中
            emit signalRefreshSinglePage();   //刷新单包安装界面

        } else if (validPkgCount > 1) { //当前程序中值添加了一个包，但是这次有不止一个包将会被添加到程序中
            emit signalSingle2MultiPage();     //刷新批量安装界面
            emit signalAppendStart();          //开始批量添加
        }
    } else if (2 == packageCount) {
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
    //预先校验包是否有效或是否重复
    DebFile currentDebfile(packagePath);
    if (!currentDebfile.isValid() || m_appendedPackagesMd5.contains(packageMd5Sum)) {
        return;
    }

    //加入md5集合
    m_appendedPackagesMd5 << packageMd5Sum;

    //使用依赖图计算安装顺序
    auto currentDebDepends = currentDebfile.depends();
    m_dependGraph.addNode(packagePath, packageMd5Sum, currentDebfile.packageName(), currentDebDepends); //添加图节点
    auto installQueue = m_dependGraph.getBestInstallQueue(); //输出最佳安装顺序
    m_preparedPackages = installQueue.first;
    m_packageMd5 = installQueue.second;
    int indexRow = 0;
    for (; indexRow != m_packageMd5.size(); ++indexRow) {
        if (m_packageMd5[indexRow] == packageMd5Sum) {
            break;
        }
    }
    if (indexRow == m_packageMd5.size()) { //error
        return;
    }

    //需要在此之前刷新出正确的安装顺序
    getPackageDependsStatus(indexRow);                     //刷新当前添加包的依赖
    refreshPage(validPkgCount);                     //添加后，根据添加的状态刷新界面
}

QList<QString> PackagesManager::getAllDepends(const QList<DependencyItem> &depends, QString architecture)
{
    //检索当前包的所有依赖
    for (const auto &list : depends) {
        for (auto info : list) { //获取当前包的依赖
            QList<QString> dList = getAllDepends(info.packageName(), architecture);
            m_allDependsList << info.packageName() << dList; //存储所有依赖
        }
    }
    m_allDependsList = m_allDependsList.toSet().toList();

    return m_allDependsList;
}

QList<QString> PackagesManager::getAllDepends(const QString &packageName, QString architecture)
{
    QList<QString> dDepends; //存储依赖的依赖
    dDepends << packageName;
    //对安装包依赖的依赖进行检索
    Package *package = packageWithArch(packageName, architecture);
    if (package) {
        for (const auto &list : package->depends()) {
            for (auto info : list) {
                //若列表中已存在该依赖，直接跳过
                static QSet<QString> set;
                if (set.contains(info.packageName()))
                    continue;
                set << info.packageName();
                //将依赖存入列表并进行下一层依赖检索
                dDepends << info.packageName() << getAllDepends(info.packageName(), architecture);
            }
        }
    }
    return dDepends;
}

const PackageDependsStatus PackagesManager::checkDependsPackageStatus(QSet<QString> &choosed_set,
                                                                      const QString &architecture,
                                                                      const QList<DependencyItem> &depends)
{
    PackageDependsStatus dependsStatus = PackageDependsStatus::ok();
    QList<DependInfo> break_list;
    QList<DependInfo> available_list;
    for (const auto &candicate_list : depends) {
        const auto r = checkDependsPackageStatus(choosed_set, architecture, candicate_list);
        dependsStatus.maxEq(r);
        if (!m_dinfo.packageName.isEmpty()) {
            if (r.isBreak()) {
                break_list.append(m_dinfo);
            } else if (r.isAvailable()) {
                available_list.append(m_dinfo);
            }
        }
    }

    m_pair.first.append(available_list);
    m_pair.second.append(break_list);
    m_dependsPackages.insert(m_currentPkgMd5, m_pair);
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

        if (!m_unCheckedOrDepends.isEmpty() && r.isBreak()) { //安装包存在或依赖关系且当前依赖状态为break
            QVector<QString> depends;
            for (auto orDepends : m_unCheckedOrDepends) { //遍历或依赖组，检测当前依赖是否存在或依赖关系
                depends = orDepends;
                if (orDepends.contains(info.packageName())) {
                    m_unCheckedOrDepends.removeOne(orDepends);
                    m_checkedOrDependsStatus.insert(info.packageName(), r);
                    depends.removeOne(info.packageName()); //将当前依赖从或依赖中删除，检测或依赖中剩余依赖状态
                    qInfo() << depends << orDepends;
                    for (auto otherDepend : depends) {
                        // 避免检测过的或依赖重复检测
                        PackageDependsStatus status;
                        if (m_checkedOrDependsStatus.contains(otherDepend)) {
                            status = m_checkedOrDependsStatus[otherDepend];
                        } else {
                            status = checkDependsPackageStatus(choosed_set, architecture, m_dependsInfo.find(otherDepend).value());
                            m_checkedOrDependsStatus.insert(otherDepend, status);
                        }
                        qInfo() << status.status;
                        if (status.isBreak()) //若剩余依赖中存在状态不为break，则说明依赖关系满足
                            continue;
                        dependsStatus.minEq(status);
                    }
                    break;
                }
            }
        }

        if (dependsStatus.status == DebListModel::DependsOk) {
            break;
        }
    }
    return dependsStatus;
}

const PackageDependsStatus PackagesManager::checkDependsPackageStatus(QSet<QString> &choosed_set,
                                                                      const QString &architecture,
                                                                      const DependencyInfo &dependencyInfo)
{
    m_dinfo.packageName.clear();
    m_dinfo.version.clear();
    const QString package_name = dependencyInfo.packageName();

    Package *package = packageWithArch(package_name, architecture, dependencyInfo.multiArchAnnotation());

    if (!package) {
        qWarning() << "PackagesManager:" << "depends break because package" << package_name << "not available";
        isDependsExists = true;
        m_dinfo.packageName = package_name + ":" + architecture;
        m_dinfo.version = dependencyInfo.packageVersion();
        return PackageDependsStatus::_break(package_name);
    }

    const RelationType relation = dependencyInfo.relationType();
    const QString &installedVersion = package->installedVersion();

    if (!installedVersion.isEmpty()) {
        const int result = Package::compareVersion(installedVersion, dependencyInfo.packageVersion());
        if (dependencyVersionMatch(result, relation)) {
            return PackageDependsStatus::ok();
        } else {
            const QString &mirror_version = package->availableVersion();
            if (mirror_version != installedVersion) {
                const auto mirror_result = Package::compareVersion(mirror_version, dependencyInfo.packageVersion());
                if (dependencyVersionMatch(mirror_result, relation)) {
                    qInfo() << "PackagesManager:" << "availble by upgrade package" << package->name() + ":" + package->architecture() << "from"
                            << installedVersion << "to" << mirror_version;
                    // 修复卸载p7zip导致deepin-wine-helper被卸载的问题，Available 添加packageName
                    m_dinfo.packageName = package_name + ":" + package->architecture();
                    m_dinfo.version = package->availableVersion();
                    return PackageDependsStatus::available(package->name());
                }
            }

            qWarning() << "PackagesManager:" << "depends break by" << package->name() << package->architecture() << dependencyInfo.packageVersion();
            qWarning() << "PackagesManager:" << "installed version not match" << installedVersion;
            m_dinfo.packageName = package_name + ":" + package->architecture();
            m_dinfo.version = dependencyInfo.packageVersion();
            return PackageDependsStatus::_break(package->name());
        }
    } else {
        const int result = Package::compareVersion(package->version(), dependencyInfo.packageVersion());
        if (!dependencyVersionMatch(result, relation)) {
            qWarning() << "PackagesManager:" << "depends break by" << package->name() << package->architecture() << dependencyInfo.packageVersion();
            qWarning() << "PackagesManager:" << "available version not match" << package->version();
            m_dinfo.packageName = package_name + ":" + package->architecture();
            m_dinfo.version = dependencyInfo.packageVersion();
            return PackageDependsStatus::_break(package->name());
        }

        // is that already choosed?
        if (choosed_set.contains(package->name()))
            return PackageDependsStatus::ok();

        // check arch conflicts
        if (package->multiArchType() == MultiArchSame) {
            Backend *backend = m_backendFuture.result();
            for (const auto &arch : backend->architectures()) {
                if (arch == package->architecture())
                    continue;

                Package *otherArchPackage = backend->package(package->name() + ":" + arch);
                if (otherArchPackage && otherArchPackage->isInstalled()) {
                    isDependsExists = true; //依赖冲突不属于依赖缺失
                    qWarning() << "PackagesManager:"
                               << "multiple architecture installed: " << package->name() << package->version() << package->architecture() << "but now need"
                               << otherArchPackage->name() << otherArchPackage->version() << otherArchPackage->architecture() << isDependsExists;
                    m_brokenDepend = package->name() + ":" + package->architecture();
                    return PackageDependsStatus::available(package->name() + ":" + package->architecture());
                }
            }
        }
        // let's check conflicts
        if (!isConflictSatisfy(architecture, package).is_ok()) {

            Backend *backend = m_backendFuture.result();
            for (auto *availablePackage : backend->availablePackages()) {
                if (!availablePackage->providesList().contains(package->name())) {
                    availablePackage = nullptr;
                    continue;
                }

                // is that already provide by another package?
                if (availablePackage->isInstalled()) {
                    qInfo() << "PackagesManager:" << "find a exist provider: " << availablePackage->name();
                    availablePackage = nullptr;
                    return PackageDependsStatus::ok();
                }

                // provider is ok, switch to provider.
                if (isConflictSatisfy(architecture, availablePackage).is_ok()) {
                    qInfo() << "PackagesManager:" << "switch to depends a new provider: " << availablePackage->name();
                    choosed_set << availablePackage->name();
                    availablePackage = nullptr;
                    return PackageDependsStatus::ok();
                }
                availablePackage = nullptr;
            }

            qWarning() << "PackagesManager:" << "providers not found, still break: " << package->name();
            m_dinfo.packageName = package_name + ":" + package->architecture();
            m_dinfo.version = dependencyInfo.packageVersion();
            return PackageDependsStatus::_break(package->name());
        }

        // now, package dependencies status is available or break,
        // time to check depends' dependencies, but first, we need
        // to add this package to choose list
        choosed_set << package->name();
        // 判断并获取依赖的或依赖关系
        getPackageOrDepends(package->name(), package->architecture(), false);
        const auto dependsStatus = checkDependsPackageStatus(choosed_set, package->architecture(), package->depends());
        if (dependsStatus.isBreak()) {
            choosed_set.remove(package->name());
            qWarning() << "PackagesManager:"
                       << "depends break by direct depends" << package->name() << package->architecture() << dependsStatus.package << isDependsExists;
            if (!isDependsExists) {
                m_dinfo.packageName = package_name + ":" + package->architecture();
                m_dinfo.version = dependencyInfo.packageVersion();
            } else {
                m_dinfo.packageName = "";
                m_dinfo.version = "";
            }
            return PackageDependsStatus::_break(package->name());
        }

        qInfo() << "PackagesManager:" << "Check finished for package" << package->name();
        // 修复卸载p7zip导致deepin-wine-helper被卸载的问题，Available 添加packageName
        m_dinfo.packageName = package_name;
        m_dinfo.version = package->availableVersion();
        return PackageDependsStatus::available(package->name());
    }
}

Package *PackagesManager::packageWithArch(const QString &packageName, const QString &sysArch,
                                          const QString &annotation)
{
    Backend *backend = m_backendFuture.result();
    if (!backend) {
        qWarning() << "Failed to load libqapt backend";
        return nullptr;
    }

    Package *package = backend->package(packageName + resolvMultiArchAnnotation(annotation, sysArch));
    // change: 按照当前支持的CPU架构进行打包。取消对deepin-wine的特殊处理
    if (!package)
        package = backend->package(packageName);
    if (package)
        return package;
    for (QString arch : backend->architectures()) {
        package = backend->package(packageName + ":" + arch);
        if (package)
            return package;
    }

    // check virtual package providers
    for (auto *virtualPackage : backend->availablePackages()) {
        if (virtualPackage->name() != packageName && virtualPackage->providesList().contains(packageName)) {
            return packageWithArch(virtualPackage->name(), sysArch, annotation);
        }
    }

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
    if (!tempPath.exists())      //如果临时目录不存在则返回创建结果
        return tempPath.mkdir(m_tempLinkDir);
    else
        return true;
}

/**
 * @brief PackagesManager::link 创建软链接
 * @param linkPath              原文件的路径
 * @param packageName           包的packageName
 * @return                      软链接之后的路径
 */
QString PackagesManager::link(QString linkPath, QString packageName)
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
            qWarning() << "PackagesManager:" << "A file with the same name exists in the current temporary directory,"
                       "and the current file name is changed to"
                       << tempName;
            count++;
        } else {
            break;
        }
    }
    //创建软链接
    if (linkDeb.link(linkPath, m_tempLinkDir + tempName)) {
        return m_tempLinkDir + tempName;    //创建成功,返回创建的软链接的路径.
    } else {
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
    if (tempPath.exists())            //如果临时目录存在，则删除临时目录
        return tempPath.removeRecursively();
    else
        return true;                    //临时目录不存在，返回删除成功
}

QString PackagesManager::package(const int index) const
{
    return m_preparedPackages[index];
}

void PackagesManager::getBlackApplications()
{
    QFile blackListFile(BLACKFILE);
    if (blackListFile.exists()) {
        blackListFile.open(QFile::ReadOnly);
        QString blackApplications = blackListFile.readAll();
        blackApplications.replace(" ", "");
        blackApplications = blackApplications.replace("\n", "");
        m_blackApplicationList =  blackApplications.split(",");
        blackListFile.close();
        return;
    }
    qWarning() << "Black File not Found";
}

bool PackagesManager::isBlackApplication(QString applicationName)
{
    if (m_blackApplicationList.contains(applicationName))
        return true;
    return false;
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

    delete backend;
    backend = nullptr;
}

