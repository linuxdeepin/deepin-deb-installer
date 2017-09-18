/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
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
#include "deblistmodel.h"

#include <QtConcurrent>
#include <QSet>

using namespace QApt;

QString relationName(const RelationType type)
{
    switch (type)
    {
    case LessOrEqual:       return "<=";
    case GreaterOrEqual:    return ">=";
    case LessThan:          return "<";
    case GreaterThan:       return ">";
    case Equals:            return "=";
    case NotEqual:          return "!=";
    default:;
    }

    return QString();
}

bool isArchMatches(QString sysArch, const QString &packageArch, const int multiArchType)
{
    Q_UNUSED(multiArchType);

    if (sysArch.startsWith(':'))
        sysArch.remove(0, 1);

    if (sysArch == "all" || sysArch == "any")
        return true;

//    if (multiArchType == MultiArchForeign)
//        return true;

    return sysArch == packageArch;
}

QString resolvMultiArchAnnotation(const QString &annotation, const QString &debArch, const int multiArchType = InvalidMultiArchType)
{
    if (annotation == "native" || annotation == "any")
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
    switch (relation)
    {
    case LessOrEqual:       return result <= 0;
    case GreaterOrEqual:    return result >= 0;
    case LessThan:          return result < 0;
    case GreaterThan:       return result > 0;
    case Equals:            return result == 0;
    case NotEqual:          return result != 0;
    default:;
    }

    return true;
}

Backend *init_backend()
{
    Backend *b = new Backend;

    if (b->init())
        return b;

    qFatal("%s", b->initErrorMessage().toStdString().c_str());
    return nullptr;
}

PackagesManager::PackagesManager(QObject *parent)
    : QObject(parent)
{
    m_backendFuture = QtConcurrent::run(init_backend);
}

bool PackagesManager::isBackendReady()
{
    return m_backendFuture.isFinished();
}

bool PackagesManager::isArchError(const int idx)
{
    Backend *b = m_backendFuture.result();
    DebFile *deb = m_preparedPackages[idx];

    const QString arch = deb->architecture();

    if (arch == "all" || arch == "any")
        return false;

    return !b->architectures().contains(deb->architecture());
}

const ConflictResult PackagesManager::packageConflictStat(const int index)
{
    auto *p = m_preparedPackages[index];

    return isConflictSatisfy(p->architecture(), p->conflicts());
}

const ConflictResult PackagesManager::isConflictSatisfy(const QString &arch, Package *package)
{
    qDebug() << "check conflict for package" << package->name() << arch;

    const auto ret = isConflictSatisfy(arch, package->conflicts());

    qDebug() << "check finished, conflict is satisfy:" << package->name() << bool(ret.is_ok());

    return ret;
}

const ConflictResult PackagesManager::isConflictSatisfy(const QString &arch, const QList<DependencyItem> &conflicts)
{
    for (const auto &conflict_list : conflicts)
    {
        for (const auto &conflict : conflict_list)
        {
            const QString name = conflict.packageName();
            Package *p = packageWithArch(name, arch, conflict.multiArchAnnotation());

            if (!p || !p->isInstalled())
                continue;

            // arch error, conflicts
            if (!isArchMatches(arch, p->architecture(), p->multiArchType()))
            {
                qDebug() << "conflicts package installed: " << arch << p->name() << p->architecture() << p->multiArchTypeString();
                return ConflictResult::err(name);
            }

            const QString conflict_version = conflict.packageVersion();
            const QString installed_version = p->installedVersion();
            const auto type = conflict.relationType();
            const auto result = Package::compareVersion(installed_version, conflict_version);

            // not match, ok
            if (!dependencyVersionMatch(result, type))
                continue;

            // test package
            const QString mirror_version = p->availableVersion();
            if (mirror_version == installed_version)
                continue;

            // mirror version is also break
            const auto mirror_result = Package::compareVersion(mirror_version, conflict_version);
            if (dependencyVersionMatch(mirror_result, type))
            {
                qDebug() << "conflicts package installed: " << arch << p->name() << p->architecture() << p->multiArchTypeString() << mirror_version << conflict_version;
                return ConflictResult::err(name);
            }
        }
    }

    return ConflictResult::ok(QString());
}

int PackagesManager::packageInstallStatus(const int index)
{
    if (m_packageInstallStatus.contains(index))
        return m_packageInstallStatus[index];

    const QString packageName = m_preparedPackages[index]->packageName();
    const QString packageArch = m_preparedPackages[index]->architecture();
    Backend *b = m_backendFuture.result();
    Package *p = b->package(packageName + ":" + packageArch);

    int ret = DebListModel::NotInstalled;
    do {
        if (!p)
            break;

        const QString installedVersion = p->installedVersion();
        if (installedVersion.isEmpty())
            break;

        const QString packageVersion = m_preparedPackages[index]->version();
        const int result = Package::compareVersion(packageVersion, installedVersion);

        if (result == 0)
            ret = DebListModel::InstalledSameVersion;
        else if (result < 0)
            ret = DebListModel::InstalledLaterVersion;
        else
            ret = DebListModel::InstalledEarlierVersion;
    } while (false);

    m_packageInstallStatus.insert(index, ret);
    return ret;
}

PackageDependsStatus PackagesManager::packageDependsStatus(const int index)
{
    if (m_packageDependsStatus.contains(index))
        return m_packageDependsStatus[index];

    if (isArchError(index))
        return PackageDependsStatus::_break(QString());

    DebFile *deb = m_preparedPackages[index];
    const QString architecture = deb->architecture();

    PackageDependsStatus ret = PackageDependsStatus::ok();

    // conflicts
    if (!isConflictSatisfy(architecture, deb->conflicts()).is_ok())
    {
        qDebug() << "depends break because conflict" << deb->packageName();
        ret.status = DebListModel::DependsBreak;
    } else {
        qDebug() << "depends:";
        qDebug() << "Check for package" << deb->packageName();
//        const auto &depends = deb->depends();

        QSet<QString> choose_set;
        choose_set << deb->packageName();
        ret = checkDependsPackageStatus(choose_set, deb->architecture(), deb->depends());

//        for (auto const &candidate_list : depends)
//        {
//            if (candidate_list.isEmpty())
//                continue;

//            PackageDependsStatus tr = PackageDependsStatus::_break(QString());
//            for (auto const &info : candidate_list)
//            {
//                const auto r = checkDependsPackageStatus(choose_set, architecture, info);
//                tr = tr.minEq(r);
//            }

//            ret.maxEq(tr);
//            if (ret.isBreak())
//                break;
//        }
    }

    if (ret.isBreak())
        Q_ASSERT(!ret.package.isEmpty());

    m_packageDependsStatus[index] = ret;

    qDebug() << "Check finished for package" << deb->packageName() << ret.status;
    if (ret.status == DebListModel::DependsAvailable)
    {
        const auto list = packageAvailableDepends(index);
        qDebug() << "available depends:" << list.size() << list;
    }

    return ret;
}

const QString PackagesManager::packageInstalledVersion(const int index)
{
    Q_ASSERT(m_packageInstallStatus.contains(index));
    Q_ASSERT(m_packageInstallStatus[index] == DebListModel::InstalledEarlierVersion ||
             m_packageInstallStatus[index] == DebListModel::InstalledLaterVersion);

    Backend *b = m_backendFuture.result();
    Package *p = b->package(m_preparedPackages[index]->packageName());

    return p->installedVersion();
}

const QStringList PackagesManager::packageAvailableDepends(const int index)
{
    Q_ASSERT(m_packageDependsStatus.contains(index));
    Q_ASSERT(m_packageDependsStatus[index].isAvailable());

    DebFile *deb = m_preparedPackages[index];
    QSet<QString> choose_set;
    const QString debArch = deb->architecture();
    const auto &depends = deb->depends();
    packageCandidateChoose(choose_set, debArch, depends);

    // TODO: check upgrade from conflicts

    return choose_set.toList();
}

void PackagesManager::packageCandidateChoose(QSet<QString> &choosed_set, const QString &debArch, const QList<DependencyItem> &dependsList)
{
    for (auto const &candidate_list : dependsList)
        packageCandidateChoose(choosed_set, debArch, candidate_list);
}

void PackagesManager::packageCandidateChoose(QSet<QString> &choosed_set, const QString &debArch, const DependencyItem &candidateList)
{
    bool choosed = false;

    for (const auto &info : candidateList)
    {
        Package *dep = packageWithArch(info.packageName(), debArch, info.multiArchAnnotation());
        if (!dep)
            continue;

        const auto choosed_name = dep->name() + resolvMultiArchAnnotation(QString(), dep->architecture());
        if (choosed_set.contains(choosed_name))
        {
            choosed = true;
            break;
        }

        // TODO: upgrade?
        if (!dep->installedVersion().isEmpty())
            return;

        if (!isConflictSatisfy(debArch, dep->conflicts()).is_ok())
        {
            qDebug() << "conflict error in choose candidate" << dep->name();
            continue;
        }

        // pass if break
        QSet<QString> set = choosed_set;
        set << choosed_name;
        const auto stat = checkDependsPackageStatus(set, dep->architecture(), dep->depends());
        if (stat.isBreak())
        {
            qDebug() << "depends error in choose candidate" << dep->name();
            continue;
        }

        choosed = true;
        choosed_set << choosed_name;
        packageCandidateChoose(choosed_set, debArch, dep->depends());
        break;
    }

    Q_ASSERT(choosed);
}

const QStringList PackagesManager::packageReverseDependsList(const QString &packageName, const QString &sysArch)
{
    Package *p = packageWithArch(packageName, sysArch);
    Q_ASSERT(p);

    QSet<QString> ret { packageName };
    QQueue<QString> testQueue;

    for (const auto &item : p->requiredByList().toSet())
        testQueue.append(item);

    while (!testQueue.isEmpty())
    {
        const auto item = testQueue.first();
        testQueue.pop_front();

        if (ret.contains(item))
            continue;

        Package *p = packageWithArch(item, sysArch);
        if (!p || !p->isInstalled())
            continue;

        if (p->recommendsList().contains(packageName))
            continue;

        ret << item;

        // append new reqiure list
        for (const auto &r : p->requiredByList())
        {
            if (ret.contains(r) || testQueue.contains(r))
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
    m_preparedPackages.clear();
    m_packageInstallStatus.clear();
    m_packageDependsStatus.clear();
    m_appendedPackagesMd5.clear();
    m_backendFuture.result()->reloadCache();
}

void PackagesManager::resetPackageDependsStatus(const int index)
{
    if (!m_packageDependsStatus.contains(index))
        return;

    // reload backend cache
    m_backendFuture.result()->reloadCache();

    m_packageDependsStatus.remove(index);
}

void PackagesManager::removePackage(const int index)
{
    DebFile *deb = m_preparedPackages[index];
    const auto md5 = deb->md5Sum();

    m_appendedPackagesMd5.remove(md5);
    m_preparedPackages.removeAt(index);
    m_packageInstallStatus.clear();
    m_packageDependsStatus.clear();
}

void PackagesManager::appendPackage(DebFile *debPackage)
{
    const auto md5 = debPackage->md5Sum();
    if (m_appendedPackagesMd5.contains(md5))
        return;

    m_preparedPackages << debPackage;
    m_appendedPackagesMd5 << md5;
}

const PackageDependsStatus PackagesManager::checkDependsPackageStatus(QSet<QString> &choosed_set, const QString &architecture, const QList<DependencyItem> &depends)
{
    PackageDependsStatus ret = PackageDependsStatus::ok();

    for (const auto &candicate_list : depends)
    {
        const auto r = checkDependsPackageStatus(choosed_set, architecture, candicate_list);
        ret.maxEq(r);

        if (ret.isBreak())
            break;
    }

    return ret;
}

const PackageDependsStatus PackagesManager::checkDependsPackageStatus(QSet<QString> &choosed_set, const QString &architecture, const DependencyItem &candicate)
{
    PackageDependsStatus ret = PackageDependsStatus::_break(QString());

    for (const auto &info : candicate)
    {
        const auto r = checkDependsPackageStatus(choosed_set, architecture, info);
        ret.minEq(r);

        if (!ret.isBreak())
            break;
    }

    return ret;
}

const PackageDependsStatus PackagesManager::checkDependsPackageStatus(QSet<QString> &choosed_set, const QString &architecture, const DependencyInfo &dependencyInfo)
{
    const QString package_name = dependencyInfo.packageName();

    Package *p = packageWithArch(package_name, architecture, dependencyInfo.multiArchAnnotation());

    if (!p)
    {
        qDebug() << "depends break because package" << package_name << "not available";
        return PackageDependsStatus::_break(package_name);
    }

    qDebug() << DependencyInfo::typeName(dependencyInfo.dependencyType())
             << package_name
             << p->architecture()
             << relationName(dependencyInfo.relationType())
             << dependencyInfo.packageVersion();

//    if (dependencyInfo.packageVersion().isEmpty())
//        return PackageDependsStatus::ok();

    const RelationType relation = dependencyInfo.relationType();
    const QString &installedVersion = p->installedVersion();

    if (!installedVersion.isEmpty())
    {
        const int result = Package::compareVersion(installedVersion, dependencyInfo.packageVersion());
        if (dependencyVersionMatch(result, relation))
            return PackageDependsStatus::ok();
        else
        {
            const QString &mirror_version = p->availableVersion();
            if (mirror_version != installedVersion)
            {
                const auto mirror_result = Package::compareVersion(mirror_version, dependencyInfo.packageVersion());

                if (dependencyVersionMatch(mirror_result, relation))
                {
                    qDebug() << "availble by upgrade package" << p->name() << p->architecture() << "from" << installedVersion << "to" << mirror_version;
                    return PackageDependsStatus::available();
                }
            }

            qDebug() << "depends break by" << p->name() << p->architecture() << dependencyInfo.packageVersion();
            qDebug() << "installed version not match" << installedVersion;
            return PackageDependsStatus::_break(p->name());
        }
    } else {
        const int result = Package::compareVersion(p->version(), dependencyInfo.packageVersion());
        if (!dependencyVersionMatch(result, relation))
        {
            qDebug() << "depends break by" << p->name() << p->architecture() << dependencyInfo.packageVersion();
            qDebug() << "available version not match" << p->version();
            return PackageDependsStatus::_break(p->name());
        }

        // is that already choosed?
        if (choosed_set.contains(p->name()))
            return PackageDependsStatus::ok();

        // let's check conflicts
        if (!isConflictSatisfy(architecture, p).is_ok())
        {
            qDebug() << "depends break because conflict" << p->name();
            return PackageDependsStatus::_break(p->name());
        }

        // now, package dependencies status is available or break,
        // time to check depends' dependencies, but first, we need
        // to add this package to choose list
        choosed_set << p->name();

        qDebug() << "Check indirect dependencies for package" << p->name();

        const auto r = checkDependsPackageStatus(choosed_set, p->architecture(), p->depends());
        if (r.isBreak())
        {
            choosed_set.remove(p->name());
            qDebug() << "depends break by direct depends" << p->name() << p->architecture() << r.package;
            return PackageDependsStatus::_break(p->name());
        }

//        const auto &depends = p->depends();
//        for (auto const &item : depends)
//        {
//            PackageDependsStatus rs = PackageDependsStatus::_break(QString());
//            for (auto const &info : item)
//            {
//                const QString arch = resolvMultiArchAnnotation(info.multiArchAnnotation(), p->architecture(), p->multiArchType());

//                const auto r = checkDependsPackageStatus(choosed_set, arch, info);
//                rs.minEq(r);
//            }

//            if (rs.isBreak())
//            {
//                // we are break, remove self
//                choosed_set.remove(p->name());

//                qDebug() << "depends break by direct depends" << p->name() << p->architecture() << dependencyInfo.packageVersion();
//                return PackageDependsStatus::_break(p->name());
//            }
//        }

        qDebug() << "Check finshed for package" << p->name();

        return PackageDependsStatus::available();
    }
}

Package *PackagesManager::packageWithArch(const QString &packageName, const QString &sysArch, const QString &annotation)
{
    qDebug() << "package with arch" << packageName << sysArch << annotation;
    Backend *b = m_backendFuture.result();
    Package *p = b->package(packageName + resolvMultiArchAnnotation(annotation, sysArch));

    do {
        if (!p)
            p = b->package(packageName);
        if (!p)
            break;

        const QString arch = resolvMultiArchAnnotation(annotation, sysArch, p->multiArchType());
//        if (!arch.isEmpty())
        // reset to check foreign arch
        p = b->package(packageName + arch);
    } while(false);

    if (p)
        return p;

    qDebug() << "check virtual package providers for package" << packageName << sysArch << annotation;
    // check virtual package providers
    for (auto *ap : b->availablePackages())
        if (ap->name() != packageName && ap->providesList().contains(packageName))
            return packageWithArch(ap->name(), sysArch, annotation);

    return nullptr;
}

PackageDependsStatus PackageDependsStatus::ok()
{
    return { DebListModel::DependsOk, QString() };
}

PackageDependsStatus PackageDependsStatus::available()
{
    return { DebListModel::DependsAvailable, QString() };
}

PackageDependsStatus PackageDependsStatus::_break(const QString &package)
{
    return { DebListModel::DependsBreak, package };
}

PackageDependsStatus::PackageDependsStatus() :
    PackageDependsStatus(DebListModel::DependsOk, QString())
{

}

PackageDependsStatus::PackageDependsStatus(const int status, const QString &package) :
    status(status),
    package(package)
{

}

PackageDependsStatus PackageDependsStatus::operator =(const PackageDependsStatus &other)
{
    status = other.status;
    package = other.package;

    return *this;
}

PackageDependsStatus PackageDependsStatus::max(const PackageDependsStatus &other)
{
    if (other.status > status)
        *this = other;

    return *this;
}

PackageDependsStatus PackageDependsStatus::maxEq(const PackageDependsStatus &other)
{
    if (other.status >= status)
        *this = other;

    return *this;
}

PackageDependsStatus PackageDependsStatus::min(const PackageDependsStatus &other)
{
    if (other.status < status)
        *this = other;

    return *this;
}

PackageDependsStatus PackageDependsStatus::minEq(const PackageDependsStatus &other)
{
    if (other.status <= status)
        *this = other;

    return *this;
}

bool PackageDependsStatus::isBreak() const
{
    return status == DebListModel::DependsBreak;
}

bool PackageDependsStatus::isAvailable() const
{
    return status == DebListModel::DependsAvailable;
}
