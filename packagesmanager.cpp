#include "packagesmanager.h"
#include "deblistmodel.h"

#include <QtConcurrent>

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

QString resolvMultiArchAnnotation(const QString &annotation, const QString &debArch)
{
    if (annotation == "native" || annotation == "any")
        return QString();

    QString arch;
    if (annotation.isEmpty())
        arch = debArch;
    else
        arch = annotation;

    if (!arch.startsWith(':'))
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

int PackagesManager::packageInstallStatus(const int index)
{
    if (m_packageInstallStatus.contains(index))
        return m_packageInstallStatus[index];

    const QString packageName = m_preparedPackages[index]->packageName();
    Backend *b = m_backendFuture.result();
    Package *p = b->package(packageName);

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
        else if (result > 0)
            ret = DebListModel::InstalledLaterVersion;
        else
            ret = DebListModel::InstalledEarlierVersion;
    } while (false);

    m_packageInstallStatus.insert(index, ret);
    return ret;
}

int PackagesManager::packageDependsStatus(const int index)
{
    if (m_packageDependsStatus.contains(index))
        return m_packageDependsStatus[index];

//    Backend *b = m_backendFuture.result();
    DebFile *deb = m_preparedPackages[index];
    const QString architecture = deb->architecture();

    int ret = DebListModel::DependsOk;

    // conflicts
//    qDebug() << "conflicts:";
//    const auto &conflicts = deb->conflicts();
//    for (auto const &item : conflicts)
//    {
//        const auto conflict_package = item.first();
//        const auto package_name = conflict_package.packageName() +
//                                  resolvMultiArchAnnotation(conflict_package.multiArchAnnotation(), architecture);

//        qDebug() << package_name;
//        Package *p = b->package(package_name);
//        if (p && p->isInstalled())
//        {
//            ret = DebListModel::DependsBreak;
//            break;
//        }
//    }

    qDebug() << "depends:";
    const auto &depends = deb->depends();
    for (auto const &item : depends)
    {
        const auto &info = item.first();
        const int r = checkDependsPackageStatus(architecture, info);

        ret = std::max(r, ret);
        if (ret == DebListModel::DependsBreak)
            break;
    }

    m_packageDependsStatus[index] = ret;

    qDebug() << "result: " << ret;

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

const QStringList PackagesManager::packageAvailableDependsList(const int index)
{
    Q_ASSERT(m_packageDependsStatus.contains(index));
    Q_ASSERT(m_packageDependsStatus[index] == DebListModel::DependsAvailable);

    QStringList availablePackages;
    Backend *b = m_backendFuture.result();
    DebFile *deb = m_preparedPackages[index];

    const QString debArch = deb->architecture();

    const auto &depends = deb->depends();
    for (auto const &item : depends)
    {
        const auto &info = item.first();
        const QString archAnnotation = info.multiArchAnnotation();
        const QString arch = resolvMultiArchAnnotation(archAnnotation, debArch);

        Package *dep = b->package(info.packageName() + arch);
        if (!dep->installedVersion().isEmpty())
            continue;

        availablePackages << dep->name() + arch;
    }

    return availablePackages;
}

void PackagesManager::resetPackageDependsStatus(const int index)
{
    Q_ASSERT(m_packageDependsStatus.contains(index));

    // reload backend cache
    Q_ASSERT(m_backendFuture.result()->reloadCache());

    m_packageDependsStatus.remove(index);
}

int PackagesManager::checkDependsPackageStatus(const QString &architecture, const DependencyInfo &dependencyInfo)
{
    const QString arch = resolvMultiArchAnnotation(dependencyInfo.multiArchAnnotation(), architecture);

    qDebug() << DependencyInfo::typeName(dependencyInfo.dependencyType())
             << dependencyInfo.packageName()
             << arch
             << relationName(dependencyInfo.relationType())
             << dependencyInfo.packageVersion();

    Backend *b = m_backendFuture.result();
    Package *p = b->package(dependencyInfo.packageName() + arch);

    // package not found
    if (!p)
    {
        qDebug() << "depends break because package" << dependencyInfo.packageName() + arch << "not available";
        return DebListModel::DependsBreak;
    }

    if (dependencyInfo.packageVersion().isEmpty())
        return DebListModel::DependsOk;

    const RelationType relation = dependencyInfo.relationType();
    const QString &installedVersion = p->installedVersion();

    if (!installedVersion.isEmpty())
    {
        const int result = Package::compareVersion(installedVersion, dependencyInfo.packageVersion());
        if (dependencyVersionMatch(result, relation))
            return DebListModel::DependsOk;
        else
        {
            qDebug() << "depends break by" << p->name() << arch << dependencyInfo.packageVersion();
            qDebug() << "installed version not match" << installedVersion;
            return DebListModel::DependsBreak;
        }
    } else {
        const int result = Package::compareVersion(p->version(), dependencyInfo.packageVersion());
        if (!dependencyVersionMatch(result, relation))
        {
            qDebug() << "depends break by" << p->name() << arch << dependencyInfo.packageVersion();
            qDebug() << "available version not match" << p->version();
            return DebListModel::DependsBreak;
        }

        // now, package dependencies status is available or break,
        // time to check depends' dependencies
        const auto &depends = p->depends();
        for (auto const &item : depends)
        {
            const auto &info = item.first();
            const QString archAnnotation = info.multiArchAnnotation();
            const QString arch = resolvMultiArchAnnotation(archAnnotation, p->architecture());

            const int r = checkDependsPackageStatus(arch, info);
            if (r == DebListModel::DependsBreak)
            {
                qDebug() << "depends break by direct depends" << p->name() << arch << dependencyInfo.packageVersion();
                return DebListModel::DependsBreak;
            }
        }

        return DebListModel::DependsAvailable;
    }
}
