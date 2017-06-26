#include "deblistmodel.h"
#include "packagesmanager.h"

#include <QDebug>
#include <QSize>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

#include <QApt/Package>
#include <QApt/Backend>

using namespace QApt;

DebListModel::DebListModel(QObject *parent)
    : QAbstractListModel(parent),

      m_installerStatus(InstallerPrepare),

      m_packagesManager(new PackagesManager(this))
{
}

const QList<DebFile *> DebListModel::preparedPackages() const
{
    return m_packagesManager->m_preparedPackages;
}

int DebListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_packagesManager->m_preparedPackages.size();
}

QVariant DebListModel::data(const QModelIndex &index, int role) const
{
    const int r = index.row();
    const DebFile *package = m_packagesManager->m_preparedPackages[r];

    switch (role)
    {
    case PackageNameRole:
        return package->packageName();
    case PackagePathRole:
        return package->filePath();
    case PackageVersionRole:
        return package->version();
    case PackageVersionStatusRole:
        return m_packagesManager->packageInstallStatus(r);
    case PackageDescriptionRole:
        return package->shortDescription();
    case Qt::SizeHintRole:
        return QSize(0, 60);
    default:;
    }

    return QVariant();
}

void DebListModel::installAll()
{
    Q_ASSERT_X(m_installerStatus == InstallerPrepare, Q_FUNC_INFO, "installer status error");

    m_installerStatus = InstallerInstalling;
    m_opIter = m_packagesManager->m_preparedPackages.begin();

    // start first
    installNextDeb();
}

void DebListModel::appendPackage(DebFile *package)
{
    Q_ASSERT_X(m_installerStatus == InstallerPrepare, Q_FUNC_INFO, "installer status error");

    m_packagesManager->m_preparedPackages.append(package);
}

void DebListModel::installNextDeb()
{
    Q_ASSERT_X(m_installerStatus == InstallerInstalling, Q_FUNC_INFO, "installer status error");

    // install finished
    if (m_opIter == m_packagesManager->m_preparedPackages.end())
    {
        qDebug() << "congraulations, install finished !!!";

        m_installerStatus = InstallerFinished;
        emit installerFinished();
        return;
    }

    // fetch next deb
    DebFile *deb = *m_opIter++;

    qDebug() << Q_FUNC_INFO << "starting to install package: " << deb->packageName();

    m_currentTransaction = m_packagesManager->m_backendFuture.result()->installFile(*deb);
    Transaction *trans = m_currentTransaction.data();

    connect(trans, &Transaction::statusDetailsChanged, this, &DebListModel::appendOutputInfo);
    connect(trans, &Transaction::finished, this, &DebListModel::installNextDeb);
    connect(trans, &Transaction::finished, trans, &Transaction::deleteLater);

    trans->run();
}

void DebListModel::fetchPackageInstallStatus(const QModelIndex &index)
{
}
