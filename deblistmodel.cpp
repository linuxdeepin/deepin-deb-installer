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

      m_workerStatus(WorkerPrepare),

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
    const DebFile *package = m_packagesManager->package(r);

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
    case PackageDependsStatusRole:
        return m_packagesManager->packageDependsStatus(r);
    case PackageInstalledVersionRole:
        return m_packagesManager->packageInstalledVersion(r);
    case PackageDescriptionRole:
        return package->shortDescription();
    case PackageOperateStatusRole:
        if (m_packageOperateStatus.contains(r))
            return m_packageOperateStatus[r];
        else
            return Prepare;
    case Qt::SizeHintRole:
        return QSize(0, 60);
    default:;
    }

    return QVariant();
}

void DebListModel::installAll()
{
    Q_ASSERT_X(m_workerStatus == WorkerPrepare, Q_FUNC_INFO, "installer status error");

    m_workerStatus = WorkerProcessing;
    m_operatingIndex = 0;

    emit workerStarted();

    // start first
    installNextDeb();
}

void DebListModel::uninstallPackage(const int idx)
{
    Q_ASSERT_X(m_workerStatus == WorkerPrepare, Q_FUNC_INFO, "installer status error");

    m_workerStatus = WorkerProcessing;

    DebFile *deb = m_packagesManager->package(idx);
    Backend *b = m_packagesManager->m_backendFuture.result();
    Package *p = b->package(deb->packageName());
    Q_ASSERT(p);

    // uninstall
    qDebug() << Q_FUNC_INFO << "starting to remove package: " << p->name();
    emit workerStarted();

    m_currentTransaction = b->removePackages(QList<Package *> { p });
    Transaction *trans = m_currentTransaction.data();

    connect(trans, &Transaction::progressChanged, this, &DebListModel::transactionProgressChanged);
    connect(trans, &Transaction::statusDetailsChanged, this, &DebListModel::appendOutputInfo);
    connect(trans, &Transaction::finished, this, &DebListModel::uninstallFinished);
    connect(trans, &Transaction::finished, trans, &Transaction::deleteLater);

    trans->run();
}

void DebListModel::appendPackage(DebFile *package)
{
    Q_ASSERT_X(m_workerStatus == WorkerPrepare, Q_FUNC_INFO, "installer status error");

    m_packagesManager->m_preparedPackages.append(package);
}

void DebListModel::onTransactionErrorOccurred()
{
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");
    Transaction *trans = static_cast<Transaction *>(sender());
    trans->deleteLater();

    const QApt::ErrorCode e = trans->error();
    Q_ASSERT(e);

    // package filaed
    refreshOperatingPackageStatus(Failed);

    bool broke = false;

    switch (e)
    {
    case InitError:
    case LockError:
    case DiskSpaceError:
    case WorkerDisappeared:
        broke = true;
        break;

    case AuthError: /* restart auth */
        break;

    default:
        qWarning() << e << trans->errorDetails() << trans->errorString();
    }

    if (!broke)
    {
        installNextDeb();
    } else {
        m_workerStatus = WorkerFinished;
    }
}

void DebListModel::refreshOperatingPackageStatus(const DebListModel::PackageOperationStatus stat)
{
    m_packageOperateStatus[m_operatingIndex] = stat;

    const QModelIndex idx = index(m_operatingIndex);

    emit dataChanged(idx, idx);
}

void DebListModel::onTransactionFinished()
{
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");
    Transaction *trans = static_cast<Transaction *>(sender());
    trans->deleteLater();

    // report new progress
    emit workerProgressChanged(100. * (m_operatingIndex + 1) / m_packagesManager->m_preparedPackages.size());

    DebFile *deb = m_packagesManager->package(m_operatingIndex);
    refreshOperatingPackageStatus(Success);
    qDebug() << "install" << deb->packageName() << "finished with exit status:" << trans->exitStatus();
    if (trans->exitStatus())
        qWarning() << trans->error() << trans->errorDetails() << trans->errorString();

    // install finished
    if (++m_operatingIndex == m_packagesManager->m_preparedPackages.size())
    {
        qDebug() << "congraulations, install finished !!!";

        m_workerStatus = WorkerFinished;
        emit workerFinished();
        return;
    }

    // install next
    installNextDeb();
}

void DebListModel::installNextDeb()
{
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");

    // fetch next deb
    DebFile *deb = m_packagesManager->package(m_operatingIndex);
    refreshOperatingPackageStatus(Operating);

    qDebug() << Q_FUNC_INFO << "starting to install package: " << deb->packageName();

    m_currentTransaction = m_packagesManager->m_backendFuture.result()->installFile(*deb);
    Transaction *trans = m_currentTransaction.data();

    connect(trans, &Transaction::progressChanged, this, &DebListModel::transactionProgressChanged);
    connect(trans, &Transaction::statusDetailsChanged, this, &DebListModel::appendOutputInfo);
    connect(trans, &Transaction::finished, this, &DebListModel::onTransactionFinished);
    connect(trans, &Transaction::errorOccurred, this, &DebListModel::onTransactionErrorOccurred);

    trans->run();
}

void DebListModel::uninstallFinished()
{
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");

    qDebug() << Q_FUNC_INFO;

    m_workerStatus = WorkerFinished;

    emit workerFinished();
}
