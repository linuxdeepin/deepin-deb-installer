#include "packagesmanager.h"
#include "deblistmodel.h"

#include <QtConcurrent>

using namespace QApt;

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

int PackagesManager::packageInstallStatus(const int index)
{
    if (m_packageInstallStatus.contains(index))
        return m_packageInstallStatus[index];

    const QString packageName = m_preparedPackages[index]->packageName();
    Backend *b = m_backendFuture.result();
    Package *p = b->package(packageName);

    int ret;
    do {
        const QString installedVersion = p->installedVersion();
        if (installedVersion.isEmpty())
        {
            ret = DebListModel::NotInstalled;
            break;
        }

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

    return DebListModel::DependsOk;
}
