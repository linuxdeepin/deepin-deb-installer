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

#include "PackageStatus.h"

#include <QtConcurrent>
#include <QDebug>
#include <QSet>

#include <QApt/DebFile>

using namespace QApt;

QApt::Backend *init_backend()
{
    QApt::Backend *b = new QApt::Backend;

    if (b->init()) return b;

    return nullptr;
}

PackageStatus::PackageStatus()
    : m_backendFuture(QtConcurrent::run(init_backend))
{
    qInfo() << "[PackageStatus]" << "Constructor function PackageStatus";

}

PackageStatus::PackageStatus(DependsStatus ds, const QString &pkg)
    : m_backendFuture(QtConcurrent::run(init_backend))
{
    qInfo() << "[PackageStatus]" << "Constructor function PackageStatus" << ds << package;
    status = ds;
    package = pkg;
}

PackageStatus &PackageStatus::operator=(const PackageStatus &other)
{
    status = other.status;
    package = other.package;
    m_backendFuture = other.m_backendFuture;

    return *this;
}

PackageStatus PackageStatus::max(const PackageStatus &other)
{
    if (other.status > status) *this = other;

    return *this;
}

PackageStatus PackageStatus::maxEq(const PackageStatus &other)
{
    if (other.status >= status) *this = other;

    return *this;
}

PackageStatus PackageStatus::min(const PackageStatus &other)
{
    if (other.status < status) *this = other;

    return *this;
}

PackageStatus PackageStatus::minEq(const PackageStatus &other)
{
    if (other.status <= status) *this = other;

    return *this;
}

bool PackageStatus::isBreak() const
{
    return status == DependsBreak;
}

bool PackageStatus::isAuthCancel() const
{
    return status == DependsAuthCancel;
}

bool PackageStatus::isAvailable() const
{
    return status == DependsAvailable;
}


QString PackageStatus::resolvMultiArchAnnotation(const QString &annotation, const QString &debArch, const int multiArchType)
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
QApt::Package *PackageStatus::packageWithArch(const QString &packageName, const QString &sysArch,
                                              const QString &annotation)
{
    Backend *backend = m_backendFuture.result();
    QApt::Package *p = backend->package(packageName + resolvMultiArchAnnotation(annotation, sysArch));
    do {
        // change: 按照当前支持的CPU架构进行打包。取消对deepin-wine的特殊处理
        if (!p) p = backend->package(packageName);
        if (p) break;
        for (QString arch : backend->architectures()) {
            if (!p) p = backend->package(packageName + ":" + arch);
            if (p) break;
        }

    } while (false);

    if (p) return p;

    qInfo() << "PackagesManager:" << "check virtual package providers for" << packageName << sysArch << annotation;

    // check virtual package providers
    for (auto *ap : backend->availablePackages())
        if (ap->name() != packageName && ap->providesList().contains(packageName))
            return packageWithArch(ap->name(), sysArch, annotation);
    return nullptr;
}

/**
 * @brief isArchMatches 判断包的架构是否符合系统要求
 * @param sysArch       系统架构
 * @param packageArch   包的架构
 * @param multiArchType 系统多架构类型
 * @return 是否符合多架构要求
 */
bool PackageStatus::isArchMatches(QString sysArch, const QString &packageArch, const int multiArchType)
{
    Q_UNUSED(multiArchType);

    if (sysArch.startsWith(':')) sysArch.remove(0, 1);

    if (sysArch == "all" || sysArch == "any") return true;

    return sysArch == packageArch;
}

bool PackageStatus::dependencyVersionMatch(const int result, const RelationType relation)
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

const ConflictResult PackageStatus::isConflictSatisfy(const QString &arch, Package *package)
{
    const QString &name = package->name();
    qInfo() << "PackagesManager:" <<  "check conflict for package" << name << arch;

    const auto ret_installed = isInstalledConflict(name, package->version(), package->architecture());
    if (!ret_installed.is_ok()) return ret_installed;

    qInfo() << "PackagesManager:" << "check conflict for local installed package is ok.";

    const auto ret_package = isConflictSatisfy(arch, package->conflicts());

    qInfo() << "PackagesManager:" << "check finished, conflict is satisfy:" << package->name() << bool(ret_package.is_ok());

    return ret_package;
}

const ConflictResult PackageStatus::isConflictSatisfy(const QString &arch, const QList<DependencyItem> &conflicts)
{
    for (const auto &conflict_list : conflicts) {
        for (const auto &conflict : conflict_list) {
            const QString name = conflict.packageName();
            QApt::Package *p = packageWithArch(name, arch, conflict.multiArchAnnotation());

            if (!p || !p->isInstalled()) continue;

            // arch error, conflicts
            if (!isArchMatches(arch, p->architecture(), p->multiArchType())) {
                qInfo() << "PackagesManager:" << "conflicts package installed: " << arch << p->name() << p->architecture()
                        << p->multiArchTypeString();
                return ConflictResult::err(name);
            }

            const QString conflict_version = conflict.packageVersion();
            const QString installed_version = p->installedVersion();
            const auto type = conflict.relationType();
            const auto result = QApt::Package::compareVersion(installed_version, conflict_version);

            // not match, ok
            if (!dependencyVersionMatch(result, type)) continue;

            // test package
            const QString mirror_version = p->availableVersion();

            // mirror version is also break
            const auto mirror_result = QApt::Package::compareVersion(mirror_version, conflict_version);
            if (dependencyVersionMatch(mirror_result, type)) {
                qInfo() << "PackagesManager:" <<  "conflicts package installed: " << arch << p->name() << p->architecture()
                        << p->multiArchTypeString() << mirror_version << conflict_version;
                return ConflictResult::err(name);
            }
        }
    }

    return ConflictResult::ok(QString());
}

bool PackageStatus::isArchError(QString packagePath)
{
    DebFile deb(packagePath);
    Backend *backend = m_backendFuture.result();

    const QString arch = deb.architecture();

    if (arch == "all" || arch == "any") return false;

    bool architectures = !backend->architectures().contains(deb.architecture());

    return architectures;
}

const ConflictResult PackageStatus::isInstalledConflict(const QString &packageName, const QString &packageVersion,
                                                        const QString &packageArch)
{
    static QList<QPair<QString, DependencyInfo>> sysConflicts;

    Backend *backend = m_backendFuture.result();
    if (sysConflicts.isEmpty()) {
        for (QApt::Package *p : backend->availablePackages()) {
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

        const int relation = QApt::Package::compareVersion(packageVersion, conflict.packageVersion());
        // match, so is bad
        if (dependencyVersionMatch(relation, conflict.relationType())) return ConflictResult::err(info.first);
    }

    return ConflictResult::ok(QString());
}

DependsStatus PackageStatus::checkDependsPackageStatus(QSet<QString> &choosed_set,
                                                       const QString &architecture,
                                                       const QList<DependencyItem> &depends)
{
    DependsStatus ret = DependsOk;
    for (const auto &candicate_list : depends) {
        const auto r = checkDependsPackageStatus(choosed_set, architecture, candicate_list);
//        ret.maxEq(r);
        if (ret >= r)
            this->status = ret;
        else {
            this->status = r;
        }

        if (ret == DependsBreak) break;
    }

    return this->status;
}

DependsStatus PackageStatus::checkDependsPackageStatus(QSet<QString> &choosed_set,
                                                       const QString &architecture,
                                                       const DependencyItem &candicate)
{
//    PackageStatus ret = PackageStatus::_break(QString());

    DependsStatus ret = DependsBreak;

    for (const auto &info : candicate) {
        const auto r = checkDependsPackageStatus(choosed_set, architecture, info);
//        ret.minEq(r);
        if (ret <= r)
            this->status = ret;
        else {
            this->status = r;
        }

        if (ret != DependsBreak) break;
    }

    return this->status;
}

DependsStatus PackageStatus::checkDependsPackageStatus(QSet<QString> &choosed_set,
                                                       const QString &architecture,
                                                       const DependencyInfo &dependencyInfo)
{
    const QString package_name = dependencyInfo.packageName();

    QApt::Package *p = packageWithArch(package_name, architecture, dependencyInfo.multiArchAnnotation());

    if (!p) {
        qInfo() << "PackagesManager:" << "depends break because package" << package_name << "not available";
//        return PackageStatus::_break(package_name);
        return DependsBreak;
    }
    Backend *backend = m_backendFuture.result();

    const RelationType relation = dependencyInfo.relationType();
    const QString &installedVersion = p->installedVersion();

    if (!installedVersion.isEmpty()) {
        const int result = QApt::Package::compareVersion(installedVersion, dependencyInfo.packageVersion());
        if (dependencyVersionMatch(result, relation))
//            return PackageStatus::ok();
            return  DependsOk;
        else {
            const QString &mirror_version = p->availableVersion();
            if (mirror_version != installedVersion) {
                const auto mirror_result = QApt::Package::compareVersion(mirror_version, dependencyInfo.packageVersion());

                if (dependencyVersionMatch(mirror_result, relation)) {
                    qInfo() << "PackagesManager:" << "availble by upgrade package" << p->name() + ":" + p->architecture() << "from"
                            << installedVersion << "to" << mirror_version;
                    // 修复卸载p7zip导致deepin-wine-helper被卸载的问题，Available 添加packageName
//                    return PackageStatus::available(p->name());
                    return DependsAvailable;
                }
            }

            qInfo() << "PackagesManager:" << "depends break by" << p->name() << p->architecture() << dependencyInfo.packageVersion();
            qInfo() << "PackagesManager:" << "installed version not match" << installedVersion;
//            return PackageStatus::_break(p->name());
            return DependsBreak;
        }
    } else {
        const int result = QApt::Package::compareVersion(p->version(), dependencyInfo.packageVersion());
        if (!dependencyVersionMatch(result, relation)) {
            qInfo() << "PackagesManager:" << "depends break by" << p->name() << p->architecture() << dependencyInfo.packageVersion();
            qInfo() << "PackagesManager:" << "available version not match" << p->version();
//            return PackageStatus::_break(p->name());
            return DependsBreak;
        }

        // is that already choosed?
//        if (choosed_set.contains(p->name())) return PackageStatus::ok();
        if (choosed_set.contains(p->name())) {
            return DependsOk;
        }
        // check arch conflicts
        if (p->multiArchType() == MultiArchSame) {
            for (const auto &arch : backend->architectures()) {
                if (arch == p->architecture()) continue;
                QApt::Package *tp = packageWithArch(p->name(), arch);
                if (tp && tp->isInstalled()) {
                    qInfo() << "PackagesManager:" << "multiple architecture installed: " << p->name() << p->version() << p->architecture() << "but now need"
                            << tp->name() << tp->version() << tp->architecture();
//                    return PackageStatus::_break(p->name() + ":" + p->architecture());
                    return DependsBreak;
                }
            }
        }
        // let's check conflicts
        if (!isConflictSatisfy(architecture, p).is_ok()) {
            qInfo() << "PackagesManager:" << "depends break because conflict, ready to find providers" << p->name();

            for (auto *ap : backend->availablePackages()) {
                if (!ap->providesList().contains(p->name())) continue;

                // is that already provide by another package?
                if (ap->isInstalled()) {
                    qInfo() << "PackagesManager:" << "find a exist provider: " << ap->name();
//                    return PackageStatus::ok();
                    return DependsOk;
                }

                // provider is ok, switch to provider.
                if (isConflictSatisfy(architecture, ap).is_ok()) {
                    qInfo() << "PackagesManager:" << "switch to depends a new provider: " << ap->name();
                    choosed_set << ap->name();
//                    return PackageStatus::ok();
                    return DependsOk;
                }
            }

            qInfo() << "PackagesManager:" << "providers not found, still break: " << p->name();
//            return PackageStatus::_break(p->name());
            return DependsBreak;
        }

        // now, package dependencies status is available or break,
        // time to check depends' dependencies, but first, we need
        // to add this package to choose list
        choosed_set << p->name();

        qInfo() << "PackagesManager:" << "Check indirect dependencies for package" << p->name();

        const auto r = checkDependsPackageStatus(choosed_set, p->architecture(), p->depends());
        if (r == DependsBreak) {
            choosed_set.remove(p->name());
            qInfo() << "PackagesManager:" << "depends break by direct depends" << p->name() << p->architecture() << r;
//            return PackageStatus::_break(p->name());
            return DependsBreak;
        }

        qInfo() << "PackagesManager:" << "Check finshed for package" << p->name();

        // 修复卸载p7zip导致deepin-wine-helper被卸载的问题，Available 添加packageName
//        return PackageStatus::available(p->name());
        qInfo() << "PackageManager:" << "return DependsAvailable";
        return DependsAvailable;
    }
}

DependsStatus PackageStatus::getPackageDependsStatus(QString packagePath)
{
    QTime depends;
    depends.start();
    while (true) {
        if (m_backendFuture.isFinished()) {
            break;
        }
        qInfo() << "Initializing backend, please wait";
        usleep(10 * 1000);
    }

    m_backendFuture.result()->reloadCache();
    DebFile *deb = new DebFile(packagePath);
    const QString architecture = deb->architecture();
    DependsStatus ret = DependsOk;

    if (isArchError(packagePath)) {
//        ret.status = ArchBreak;       //添加ArchBreak错误。
//        ret.package = deb->packageName();
//        return ret.status;
        return ArchBreak;
    }

    // conflicts
    const ConflictResult debConflitsResult = isConflictSatisfy(architecture, deb->conflicts());

    if (!debConflitsResult.is_ok()) {
        qInfo() << "PackagesManager:" << "depends break because conflict" << deb->packageName();
//        ret.package = debConflitsResult.unwrap();
//        ret.status = DependsBreak;
//        this->status = DependsBreak;
        ret = DependsBreak;
    } else {
        const ConflictResult localConflictsResult =
            isInstalledConflict(deb->packageName(), deb->version(), architecture);
        if (!localConflictsResult.is_ok()) {
            qInfo() << "PackagesManager:" << "depends break because conflict with local package" << deb->packageName();
//            ret.package = localConflictsResult.unwrap();
//            ret.status = DependsBreak;
            ret = DependsBreak;
        } else {
            QSet<QString> choose_set;
            choose_set << deb->packageName();

            ret = checkDependsPackageStatus(choose_set, deb->architecture(), deb->depends());
            qInfo() << "PackagesManager:" << "Check" << deb->packageName() << "depends:" << ret;

        }
    }
//    if (ret.isBreak()) Q_ASSERT(!ret.package.isEmpty());

    delete deb;
//    return ret.status;
    qInfo() << "PackageStatus" << "getPackageDependsStatus" << "获取依赖用时" << depends.elapsed();
    return ret;
}


const QStringList PackageStatus::getPackageAvailableDepends(QString packagePath)
{
    DebFile *deb = new DebFile(packagePath);
    QSet<QString> choose_set;
    const QString debArch = deb->architecture();
    const auto &depends = deb->depends();
    packageCandidateChoose(choose_set, debArch, depends);

    // TODO: check upgrade from conflicts
    delete deb;
    return choose_set.toList();
}

InstallStatus PackageStatus::getPackageInstallStatus(QString packagePath)
{

    QTime install;
    install.start();
    m_backendFuture.result()->reloadCache();
    DebFile *debFile = new DebFile(packagePath);

    const QString packageName = debFile->packageName();
    const QString packageArch = debFile->architecture();
    const QString packageVersion = debFile->version();

    delete debFile;

    Backend *backend = m_backendFuture.result();
    Package *pkg = backend->package(packageName + ":" + packageArch);

    InstallStatus ret = InstallStatusUnknown;

    do {
        if (!pkg) break;

        const QString installedVersion = pkg->installedVersion();
        if (installedVersion.isEmpty()) {
            qInfo() << "PackageStatus" << "InstallStatus" << "获取安装状态用时：" << install.elapsed();
            return NotInstalled;
        }

        const int result = Package::compareVersion(packageVersion, installedVersion);

        if (result == 0) {
            qInfo() << "PackageStatus" << "InstallStatus" << "获取安装状态用时：" << install.elapsed();
            return InstalledSameVersion;
        } else if (result < 0) {

            qInfo() << "PackageStatus" << "InstallStatus" << "获取安装状态用时：" << install.elapsed();
            return InstalledLaterVersion;
        } else {

            qInfo() << "PackageStatus" << "InstallStatus" << "获取安装状态用时：" << install.elapsed();
            return InstalledEarlierVersion;
        }
    } while (false);

    qInfo() << "PackageStatus" << "InstallStatus" << "获取安装状态用时：" << install.elapsed();
    return ret;
}

void PackageStatus::packageCandidateChoose(QSet<QString> &choosed_set, const QString &debArch,
                                           const QList<DependencyItem> &dependsList)
{
    for (auto const &candidate_list : dependsList) packageCandidateChoose(choosed_set, debArch, candidate_list);
}

void PackageStatus::packageCandidateChoose(QSet<QString> &choosed_set, const QString &debArch,
                                           const DependencyItem &candidateList)
{
    for (const auto &info : candidateList) {
        Package *dep = packageWithArch(info.packageName(), debArch, info.multiArchAnnotation());
        if (!dep) continue;

        const auto choosed_name = dep->name() + resolvMultiArchAnnotation(QString(), dep->architecture());
        if (choosed_set.contains(choosed_name)) {
            break;
        }

        // TODO: upgrade?
        //        if (!dep->installedVersion().isEmpty()) return;
        //  修复升级依赖时，因为依赖包版本过低，造成安装循环。
        // 删除无用冗余的日志
        if (Package::compareVersion(dep->installedVersion(), info.packageVersion()) < 0) {
            Backend *b = m_backendFuture.result();
            Package *p = b->package(dep->name() + resolvMultiArchAnnotation(QString(), dep->architecture()));
            if (p) {
                choosed_set << dep->name() + resolvMultiArchAnnotation(QString(), dep->architecture());
            } else {
                choosed_set << dep->name() + " not found";
            }
        }

        if (!isConflictSatisfy(debArch, dep->conflicts()).is_ok()) {
            qInfo() << "PackagesManager:" << "conflict error in choose candidate" << dep->name();
            continue;
        }

        // pass if break
        QSet<QString> set = choosed_set;
        set << choosed_name;
        const auto stat = checkDependsPackageStatus(set, dep->architecture(), dep->depends());
        if (stat == DependsBreak) {
            qInfo() << "PackagesManager:" << "depends error in choose candidate" << dep->name();
            continue;
        }

        choosed_set << choosed_name;
        packageCandidateChoose(choosed_set, debArch, dep->depends());
        break;
    }

}

QMap<QString, QString> PackageStatus::specialPackage()
{
    QMap<QString, QString> sp;
    sp.insert("deepin-wine-plugin-virtual", "deepin-wine-helper");
    sp.insert("deepin-wine32", "deepin-wine");

    return sp;
}

const QStringList PackageStatus::getPackageReverseDependsList(const QString &packageName, const QString &sysArch)
{
    Package *package = packageWithArch(packageName, sysArch);

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
            // fix bug: https://pms.uniontech.com/zentao/bug-view-54930.html
            // 部分wine应用在系统中有一个替换的名字，使用requiredByList 可以获取到这些名字
            if (subPackage && !subPackage->requiredByList().isEmpty()) {    //增加对package指针的检查
                QStringList rdepends = subPackage->requiredByList();

                //对添加到testQueue的包进行检查，
                for (QString depend : rdepends) {
                    Package *pkg = packageWithArch(depend, sysArch);
                    if (!pkg || !pkg->isInstalled())      //增加对package指针的检查
                        continue;
                    if (pkg->recommendsList().contains(r))
                        continue;
                    if (pkg->suggestsList().contains(r))
                        continue;
                    //只添加和当前依赖是依赖关系的包
                    testQueue.append(depend);
                }

            }
            if (!subPackage || !subPackage->isInstalled())      //增加对package指针的检查
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

PackageStatus::~PackageStatus()
{
    qInfo() << "~PackageStatus()";
}
