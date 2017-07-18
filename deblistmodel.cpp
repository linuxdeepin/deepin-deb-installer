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

const QString workerErrorString(const int e)
{
    switch (e)
    {
    case InitError:                 return QStringLiteral("InitError");
    case LockError:                 return QStringLiteral("LockError");
    case DiskSpaceError:            return QStringLiteral("DiskSpaceError");
    case FetchError:                return QStringLiteral("FetchError");
    case CommitError:               return QStringLiteral("CommitError");
    case AuthError:                 return QStringLiteral("AuthError");
    case WorkerDisappeared:         return QStringLiteral("WorkerDisappeared");
    case UntrustedError:            return QStringLiteral("UntrustedError");
    case DownloadDisallowedError:   return QStringLiteral("DownloadDisallowedError");
    case NotFoundError:             return QStringLiteral("NotFoundError");
    case WrongArchError:            return QStringLiteral("WrongArchError");
    case MarkingError:              return QStringLiteral("MarkingError");
    }

    return QString("Unknow Error");
}

DebListModel::DebListModel(QObject *parent)
    : QAbstractListModel(parent),

      m_workerStatus(WorkerPrepare),

      m_packagesManager(new PackagesManager(this))
{
}

bool DebListModel::isReady() const
{
    return m_packagesManager->isBackendReady();
}

const QList<DebFile *> DebListModel::preparedPackages() const
{
    return m_packagesManager->m_preparedPackages;
}

QModelIndex DebListModel::first() const
{
    return index(0);
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
        return m_packagesManager->packageDependsStatus(r).status;
    case PackageInstalledVersionRole:
        return m_packagesManager->packageInstalledVersion(r);
    case PackageAvailableDependsListRole:
        return m_packagesManager->packageAvailableDependsList(r);
    case PackageDescriptionRole:
        return package->shortDescription();
    case PackageFailReasonRole:
        return packageFailedReason(r);
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
    m_operatingIndex = idx;

    DebFile *deb = m_packagesManager->package(m_operatingIndex);
    Backend *b = m_packagesManager->m_backendFuture.result();
    Package *p = b->package(deb->packageName());
    Q_ASSERT(p);

    // uninstall
    qDebug() << Q_FUNC_INFO << "starting to remove package: " << p->name();
    emit workerStarted();

    refreshOperatingPackageStatus(Operating);

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

    const QApt::ErrorCode e = trans->error();
    Q_ASSERT(e);

    qWarning() << Q_FUNC_INFO << e << workerErrorString(e);
    qWarning() << trans->errorDetails() << trans->errorString();

    bool broke = false;

    switch (e)
    {
    case InitError:
    case LockError:
    case DiskSpaceError:
    case WorkerDisappeared:
    case CommitError:
        broke = true;
        break;

    case AuthError: /* restart auth */
        break;

    default:;
    }

    if (!broke)
        installNextDeb();
    else
        refreshOperatingPackageStatus(Failed); /* finished slots is execute later. */
}

void DebListModel::bumpInstallIndex()
{
    // install finished
    if (++m_operatingIndex == m_packagesManager->m_preparedPackages.size())
    {
        qDebug() << "congraulations, install finished !!!";

        m_workerStatus = WorkerFinished;
        emit workerFinished();
        emit workerProgressChanged(100);
        emit transactionProgressChanged(100);
        return;
    }

    // install next
    installNextDeb();
}

void DebListModel::refreshOperatingPackageStatus(const DebListModel::PackageOperationStatus stat)
{
    m_packageOperateStatus[m_operatingIndex] = stat;

    const QModelIndex idx = index(m_operatingIndex);

    emit dataChanged(idx, idx);
}

QString DebListModel::packageFailedReason(const int idx) const
{
    const auto stat = m_packagesManager->packageDependsStatus(idx);
    if (stat.isBreak())
        return tr("Broken Dependencies: %1").arg(stat.package->name());

    Q_ASSERT(m_packageOperateStatus.contains(idx));
    Q_ASSERT(m_packageOperateStatus[idx] == Failed);

    return workerErrorString(CommitError);
}

void DebListModel::onTransactionFinished()
{
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");
    Transaction *trans = static_cast<Transaction *>(sender());
    trans->deleteLater();

    // report new progress
    emit workerProgressChanged(100. * (m_operatingIndex + 1) / m_packagesManager->m_preparedPackages.size());

    DebFile *deb = m_packagesManager->package(m_operatingIndex);
    qDebug() << "install" << deb->packageName() << "finished with exit status:" << trans->exitStatus();

    if (trans->exitStatus())
    {
        qWarning() << trans->error() << trans->errorDetails() << trans->errorString();
        emit appendOutputInfo(trans->errorString());
    }
    else if (m_packageOperateStatus.contains(m_operatingIndex) && m_packageOperateStatus[m_operatingIndex] != Failed)
    {
        refreshOperatingPackageStatus(Success);
    }

    bumpInstallIndex();
}

void DebListModel::onDependsInstallTransactionFinished()
{
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");
    Transaction *trans = static_cast<Transaction *>(sender());
    trans->deleteLater();

    // report new progress
//    emit workerProgressChanged(100. * (m_operatingIndex + 1) / m_packagesManager->m_preparedPackages.size());

    DebFile *deb = m_packagesManager->package(m_operatingIndex);
    qDebug() << "install" << deb->packageName() << "dependencies finished with exit status:" << trans->exitStatus();

    if (trans->exitStatus())
        qWarning() << trans->error() << trans->errorDetails() << trans->errorString();

    // reset package depends status
    m_packagesManager->resetPackageDependsStatus(m_operatingIndex);

    // continue install
    installNextDeb();
}

void DebListModel::installNextDeb()
{
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");

    // fetch next deb
    DebFile *deb = m_packagesManager->package(m_operatingIndex);
    refreshOperatingPackageStatus(Operating);

    auto * const backend = m_packagesManager->m_backendFuture.result();
    Transaction *trans = nullptr;

    // check available dependencies
    const auto dependsStat = m_packagesManager->packageDependsStatus(m_operatingIndex);
    if (dependsStat.isBreak())
    {
        refreshOperatingPackageStatus(Failed);
        bumpInstallIndex();
        return;
    } else if (dependsStat.isAvailable()) {
        const QStringList availableDepends = m_packagesManager->packageAvailableDependsList(m_operatingIndex);
        for (auto const &p : availableDepends)
            backend->markPackageForInstall(p);

        qDebug() << Q_FUNC_INFO << "install" << deb->packageName() << "dependencies: " << availableDepends;

        trans = backend->commitChanges();
        connect(trans, &Transaction::statusDetailsChanged, this, &DebListModel::appendOutputInfo);
        connect(trans, &Transaction::errorOccurred, this, &DebListModel::onTransactionErrorOccurred);
        connect(trans, &Transaction::finished, this, &DebListModel::onDependsInstallTransactionFinished);
    } else {
        qDebug() << Q_FUNC_INFO << "starting to install package: " << deb->packageName();

        trans = backend->installFile(*deb);

        connect(trans, &Transaction::progressChanged, this, &DebListModel::transactionProgressChanged);
        connect(trans, &Transaction::statusDetailsChanged, this, &DebListModel::appendOutputInfo);
        connect(trans, &Transaction::finished, this, &DebListModel::onTransactionFinished);
        connect(trans, &Transaction::errorOccurred, this, &DebListModel::onTransactionErrorOccurred);
    }

    // NOTE: DO NOT remove this.
    // see: https://bugs.kde.org/show_bug.cgi?id=382272
    trans->setLocale(".UTF-8");

    m_currentTransaction = trans;
    m_currentTransaction->run();
}

void DebListModel::uninstallFinished()
{
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");

    qDebug() << Q_FUNC_INFO;

    m_workerStatus = WorkerFinished;
    refreshOperatingPackageStatus(Success);

    emit workerFinished();
}
