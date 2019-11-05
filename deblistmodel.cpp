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

#include "deblistmodel.h"
#include "packagesmanager.h"
#include "utils.h"

#include <QApplication>
#include <QDebug>
#include <QFuture>
#include <QFutureWatcher>
#include <QSize>
#include <QtConcurrent>

#include <QApt/Backend>
#include <QApt/Package>

using namespace QApt;

bool isDpkgRunning()
{
    QProcess proc;
    proc.start("ps", QStringList() << "-e"
               << "-o"
               << "comm");
    proc.waitForFinished();

    const QString output = proc.readAllStandardOutput();
    for (const auto &item : output.split('\n'))
        if (item == "dpkg")
            return true;

    return false;
}

const QString workerErrorString(const int e)
{
    switch (e) {
    case FetchError:
    case DownloadDisallowedError:
        return QApplication::translate("DebListModel", "Installation failed, please check your network connection");
    case NotFoundError:
        return QApplication::translate("DebListModel",
                                       "Installation failed, please check updates in Control Center");
    case DiskSpaceError:
        return QApplication::translate("DebListModel", "Installation failed, insufficient disk space");
    }
    return QApplication::translate("DebListModel", "Installation Failed");
}

DebListModel::DebListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_workerStatus(WorkerPrepare)
    , m_packagesManager(new PackagesManager(this)) {

     connect(this, &DebListModel::workerFinished, this, &DebListModel::upWrongStatusRow);
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

    const DebFile *deb = m_packagesManager->package(r);

    switch (role) {
    case WorkerIsPrepareRole:
        return isWorkerPrepare();
    case ItemIsCurrentRole:
        return m_currentIdx == index;
    case PackageNameRole:
        return deb->packageName();
    case PackagePathRole:
        return deb->filePath();
    case PackageVersionRole:
        return deb->version();
    case PackageVersionStatusRole:
        return m_packagesManager->packageInstallStatus(r);
    case PackageDependsStatusRole:
        return m_packagesManager->packageDependsStatus(r).status;
    case PackageInstalledVersionRole:
        return m_packagesManager->packageInstalledVersion(r);
    case PackageAvailableDependsListRole:
        return m_packagesManager->packageAvailableDepends(r);
    case PackageReverseDependsListRole:
        return m_packagesManager->packageReverseDependsList(deb->packageName(), deb->architecture());
    case PackageDescriptionRole:
        return Utils::fromSpecialEncoding(deb->shortDescription());
    case PackageFailReasonRole:
        return packageFailedReason(r);
    case PackageOperateStatusRole:
        if (m_packageOperateStatus.contains(r))
            return m_packageOperateStatus[r];
        else
            return Prepare;
    case Qt::SizeHintRole:
        return QSize(0, 48);
    default:
        ;
    }

    return QVariant();
}

void DebListModel::installAll()
{
    Q_ASSERT_X(m_workerStatus == WorkerPrepare, Q_FUNC_INFO, "installer status error");
    if (m_workerStatus != WorkerPrepare) return;

    m_workerStatus = WorkerProcessing;
    m_operatingIndex = 0;
    m_InitRowStatus = false;

    //    emit workerStarted();

    // start first
    installNextDeb();
}

void DebListModel::uninstallPackage(const int idx)
{
    Q_ASSERT(idx == 0);
    Q_ASSERT_X(m_workerStatus == WorkerPrepare, Q_FUNC_INFO, "installer status error");

    m_workerStatus = WorkerProcessing;
    m_operatingIndex = idx;

    DebFile *deb = m_packagesManager->package(m_operatingIndex);

    const QStringList rdepends = m_packagesManager->packageReverseDependsList(deb->packageName(), deb->architecture());
    Backend *b = m_packagesManager->m_backendFuture.result();
    for (const auto &r : rdepends) b->markPackageForRemoval(r);
    b->markPackageForRemoval(deb->packageName() + ':' + deb->architecture());

    // uninstall
    qDebug() << Q_FUNC_INFO << "starting to remove package: " << deb->packageName() << rdepends;

    refreshOperatingPackageStatus(Operating);
    Transaction *trans = b->commitChanges();

    connect(trans, &Transaction::progressChanged, this, &DebListModel::transactionProgressChanged);
    connect(trans, &Transaction::statusDetailsChanged, this, &DebListModel::appendOutputInfo);
    connect(trans, &Transaction::statusChanged, this, &DebListModel::onTransactionStatusChanged);
    connect(trans, &Transaction::errorOccurred, this, &DebListModel::onTransactionErrorOccurred);
    connect(trans, &Transaction::finished, this, &DebListModel::uninstallFinished);
    connect(trans, &Transaction::finished, trans, &Transaction::deleteLater);

    m_currentTransaction = trans;

    trans->run();
}

void DebListModel::removePackage(const int idx)
{
    Q_ASSERT_X(m_workerStatus == WorkerPrepare, Q_FUNC_INFO, "installer status error");

    m_packagesManager->removePackage(idx);
}

bool DebListModel::appendPackage(DebFile *package)
{
    Q_ASSERT_X(m_workerStatus == WorkerPrepare, Q_FUNC_INFO, "installer status error");

    return m_packagesManager->appendPackage(package);
}

void DebListModel::onTransactionErrorOccurred()
{
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");
    Transaction *trans = static_cast<Transaction *>(sender());

    const QApt::ErrorCode e = trans->error();
    Q_ASSERT(e);

    qWarning() << Q_FUNC_INFO << e << workerErrorString(e);
    qWarning() << trans->errorDetails() << trans->errorString();

    if (trans->isCancellable()) trans->cancel();

    if (e == AuthError) {
        trans->deleteLater();

        qDebug() << "reset env to prepare";

        // reset env
        emit AuthCancel();
        emit lockForAuth(false);

        m_workerStatus = WorkerPrepare;
        return;
    }

    // DO NOT install next, this action will finished and will be install next automatic.
    trans->setProperty("exitStatus", QApt::ExitFailed);
}

void DebListModel::onTransactionStatusChanged(TransactionStatus stat)
{
    switch (stat) {
    case TransactionStatus::AuthenticationStatus:
        emit lockForAuth(true);
        break;
    case TransactionStatus::WaitingStatus:
        emit lockForAuth(false);
        break;
    default:
        ;
    }

}

int DebListModel::getInstallFileSize()
{
    return m_packagesManager->m_preparedPackages.size();
}

void DebListModel::reset()
{
    //Q_ASSERT_X(m_workerStatus == WorkerFinished, Q_FUNC_INFO, "worker status error");

    m_workerStatus = WorkerPrepare;
    m_operatingIndex = 0;

    m_packageOperateStatus.clear();
    m_packageFailReason.clear();
    m_packagesManager->reset();
}

void DebListModel::bumpInstallIndex()
{
    Q_ASSERT_X(m_currentTransaction.isNull(), Q_FUNC_INFO, "previous transaction not finished");

    // install finished
    if (++m_operatingIndex == m_packagesManager->m_preparedPackages.size()) {
        qDebug() << "congraulations, install finished !!!";

        m_workerStatus = WorkerFinished;
        emit workerFinished();
        emit workerProgressChanged(100);
        emit transactionProgressChanged(100);
        return;
    }

    emit onChangeOperateIndex(m_operatingIndex);

    // install next
    installNextDeb();
}

void DebListModel::refreshOperatingPackageStatus(const DebListModel::PackageOperationStatus stat)
{
    m_packageOperateStatus[m_operatingIndex] = stat;  //将失败包的索引和状态修改保存,用于更新

    const QModelIndex idx = index(m_operatingIndex);

    emit dataChanged(idx, idx);
}

QString DebListModel::packageFailedReason(const int idx) const
{
    const auto stat = m_packagesManager->packageDependsStatus(idx);
    if (stat.isBreak()) {
        if (!stat.package.isEmpty()) return tr("Broken Dependencies: %1").arg(stat.package);

        if (m_packagesManager->isArchError(idx)) return tr("Unmatched package architecture");

        const auto conflict = m_packagesManager->packageConflictStat(idx);
        if (!conflict.is_ok()) return tr("Broken Dependencies: %1").arg(conflict.unwrap());
        //            return tr("Conflicts: %1").arg(conflict.unwrap());

        Q_UNREACHABLE();
    }

    Q_ASSERT(m_packageOperateStatus.contains(idx));
    Q_ASSERT(m_packageOperateStatus[idx] == Failed);
    if(!m_packageFailReason.contains(idx))
        qDebug()<<"ggy"<<m_packageFailReason.size()<<idx;
    Q_ASSERT(m_packageFailReason.contains(idx));

    return workerErrorString(m_packageFailReason[idx]);
}

void DebListModel::onTransactionFinished()
{
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");
    Transaction *trans = static_cast<Transaction *>(sender());

    // prevent next signal
    disconnect(trans, &Transaction::finished, this, &DebListModel::onTransactionFinished);

    // report new progress
    emit workerProgressChanged(100. * (m_operatingIndex + 1) / m_packagesManager->m_preparedPackages.size());

    DebFile *deb = m_packagesManager->package(m_operatingIndex);
    qDebug() << "install" << deb->packageName() << "finished with exit status:" << trans->exitStatus();

    if (trans->exitStatus()) {
        qWarning() << trans->error() << trans->errorDetails() << trans->errorString();
        m_packageFailReason[m_operatingIndex] = trans->error();
        refreshOperatingPackageStatus(Failed);
        emit appendOutputInfo(trans->errorString());
    } else if (m_packageOperateStatus.contains(m_operatingIndex) &&
               m_packageOperateStatus[m_operatingIndex] != Failed) {
        refreshOperatingPackageStatus(Success);
    }

    //    delete trans;
    trans->deleteLater();
    m_currentTransaction = nullptr;

    bumpInstallIndex();
}

void DebListModel::onDependsInstallTransactionFinished()
{
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");
    Transaction *trans = static_cast<Transaction *>(sender());

    const auto ret = trans->exitStatus();

    DebFile *deb = m_packagesManager->package(m_operatingIndex);
    qDebug() << "install" << deb->packageName() << "dependencies finished with exit status:" << ret;

    if (ret) qWarning() << trans->error() << trans->errorDetails() << trans->errorString();

    if (ret) {
        // record error
        m_packageFailReason[m_operatingIndex] = trans->error();
        refreshOperatingPackageStatus(Failed);
        emit appendOutputInfo(trans->errorString());
    }

    //    delete trans;
    trans->deleteLater();
    m_currentTransaction = nullptr;

    // check current operate exit status to install or install next
    if (ret)
        bumpInstallIndex();
    else
        installNextDeb();
}

void DebListModel::installNextDeb()
{
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");
    Q_ASSERT_X(m_currentTransaction.isNull(), Q_FUNC_INFO, "previous transaction not finished");

    if (isDpkgRunning()) {
        qDebug() << "dpkg running, waitting...";
        QTimer::singleShot(1000 * 5, this, &DebListModel::installNextDeb);
        return;
    }

    // fetch next deb
    DebFile *deb = m_packagesManager->package(m_operatingIndex);

    auto *const backend = m_packagesManager->m_backendFuture.result();
    Transaction *trans = nullptr;

    // reset package depends status
    m_packagesManager->resetPackageDependsStatus(m_operatingIndex);

    // check available dependencies
    const auto dependsStat = m_packagesManager->packageDependsStatus(m_operatingIndex);
    if (dependsStat.isBreak()) {
        refreshOperatingPackageStatus(Failed);
        bumpInstallIndex();
        return;
    } else if (dependsStat.isAvailable()) {
        Q_ASSERT_X(m_packageOperateStatus[m_operatingIndex] == Prepare, Q_FUNC_INFO,
                   "package operate status error when start install availble dependencies");

        const QStringList availableDepends = m_packagesManager->packageAvailableDepends(m_operatingIndex);
        for (auto const &p : availableDepends) backend->markPackageForInstall(p);

        qDebug() << Q_FUNC_INFO << "install" << deb->packageName() << "dependencies: " << availableDepends;

        trans = backend->commitChanges();
        connect(trans, &Transaction::finished, this, &DebListModel::onDependsInstallTransactionFinished);
    } else {
        qDebug() << Q_FUNC_INFO << "starting to install package: " << deb->packageName();

        trans = backend->installFile(*deb);

        connect(trans, &Transaction::progressChanged, this, &DebListModel::transactionProgressChanged);
        connect(trans, &Transaction::finished, this, &DebListModel::onTransactionFinished);
    }

    // NOTE: DO NOT remove this.
    // see: https://bugs.kde.org/show_bug.cgi?id=382272
    trans->setLocale(".UTF-8");

    if(!m_InitRowStatus)
    {
        connect(trans, &Transaction::statusDetailsChanged, this, &DebListModel::initRowStatus);
        m_InitRowStatus = true;
    }

    connect(trans, &Transaction::statusDetailsChanged, this, &DebListModel::appendOutputInfo);
    connect(trans, &Transaction::statusDetailsChanged, this, &DebListModel::onTransactionOutput);
    connect(trans, &Transaction::statusChanged, this, &DebListModel::onTransactionStatusChanged);
    connect(trans, &Transaction::errorOccurred, this, &DebListModel::onTransactionErrorOccurred);

    m_currentTransaction = trans;
    m_currentTransaction->run();
}

void DebListModel::onTransactionOutput()
{
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");
    Transaction *trans = static_cast<Transaction *>(sender());
    Q_ASSERT(trans == m_currentTransaction.data());

    refreshOperatingPackageStatus(Operating);

    disconnect(trans, &Transaction::statusDetailsChanged, this, &DebListModel::onTransactionOutput);
}

void DebListModel::uninstallFinished()
{
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");

    qDebug() << Q_FUNC_INFO;

    m_workerStatus = WorkerFinished;
    refreshOperatingPackageStatus(Success);

    emit workerFinished();
}

void DebListModel::setCurrentIndex(const QModelIndex &idx)
{
    if (m_currentIdx == idx) return;

    const QModelIndex index = m_currentIdx;
    m_currentIdx = idx;

    emit dataChanged(index, index);
    emit dataChanged(m_currentIdx, m_currentIdx);
}

void DebListModel::initRowStatus()
{
    for(int i = 0; i < m_packagesManager->m_preparedPackages.size(); i++)
    {
        m_operatingIndex = i;
        refreshOperatingPackageStatus(Waiting);
    }
    m_operatingIndex = 0;

    Transaction *trans = static_cast<Transaction *>(sender());
    Q_ASSERT(trans == m_currentTransaction.data());
    disconnect(trans, &Transaction::statusDetailsChanged, this, &DebListModel::initRowStatus);
}
void DebListModel::upWrongStatusRow()
{   
    QList<int> listWrongIndex;
    int iIndex = 0;

    //find wrong index
    QMapIterator<int, int> iteratorpackageOperateStatus(m_packageOperateStatus);
    QList<int> listpackageOperateStatus;
    while(iteratorpackageOperateStatus.hasNext())
    {
        iteratorpackageOperateStatus.next();
        if(iteratorpackageOperateStatus.value() == Failed)
        {
            listWrongIndex.insert(iIndex, iteratorpackageOperateStatus.key() );
            listpackageOperateStatus.insert(iIndex++, iteratorpackageOperateStatus.value());
        }
        else if(iteratorpackageOperateStatus.value() == Success)
            listpackageOperateStatus.append( iteratorpackageOperateStatus.value() );
        else
            return;
    }
    if(listWrongIndex.size() == 0)
        return;

    //change  m_preparedPackages, m_packageOperateStatus sort.
    QList<QApt::DebFile *> listTempDebFile;
    iIndex = 0;
    for(int i = 0; i < m_packagesManager->m_preparedPackages.size(); i++)
    {
        m_packageOperateStatus[i] = listpackageOperateStatus[i];
        if( listWrongIndex.contains(i) )
            listTempDebFile.insert(iIndex++, m_packagesManager->m_preparedPackages[i] );
        else
            listTempDebFile.append(m_packagesManager->m_preparedPackages[i]);
    }
    m_packagesManager->m_preparedPackages = listTempDebFile;

    //change  m_packageFailReason sort.
    QMap<int,int> mappackageFailReason;
    QMapIterator<int, int> IteratorpackageFailReason(m_packageFailReason);
    while (IteratorpackageFailReason.hasNext()) {
         IteratorpackageFailReason.next();
         int iIndexTemp = listWrongIndex.indexOf( IteratorpackageFailReason.key() );
         mappackageFailReason[iIndexTemp] = IteratorpackageFailReason.value();
    }
    m_packageFailReason.clear();
    m_packageFailReason = mappackageFailReason;

    //change  m_packageInstallStatus sort.
    QMapIterator<int, int> MapIteratorpackageInstallStatus(m_packagesManager->m_packageInstallStatus);
    QList<int> listpackageInstallStatus;
    iIndex = 0;
    while(MapIteratorpackageInstallStatus.hasNext())
    {
        MapIteratorpackageInstallStatus.next();
        if( listWrongIndex.contains( MapIteratorpackageInstallStatus.key()) )
            listpackageInstallStatus.insert( iIndex++, MapIteratorpackageInstallStatus.value() );
        else
            listpackageInstallStatus.append(MapIteratorpackageInstallStatus.value());
    }
    for(int i = 0; i < listpackageInstallStatus.size(); i++)
        m_packagesManager->m_packageInstallStatus[i] = listpackageInstallStatus[i];

    //change  m_packageDependsStatus sort.
    QMapIterator<int, PackagesManagerDependsStatus::PackageDependsStatus> MapIteratorpackageDependsStatus(m_packagesManager->m_packageDependsStatus);
    QList<PackagesManagerDependsStatus::PackageDependsStatus> listpackageDependsStatus;
    iIndex = 0;
    while(MapIteratorpackageDependsStatus.hasNext())
    {
        MapIteratorpackageDependsStatus.next();
        if( listWrongIndex.contains( MapIteratorpackageDependsStatus.key()) )
            listpackageDependsStatus.insert( iIndex++, MapIteratorpackageDependsStatus.value() );
        else
            listpackageDependsStatus.append(MapIteratorpackageDependsStatus.value());
    }
    for(int i = 0; i < listpackageDependsStatus.size(); i++)
        m_packagesManager->m_packageDependsStatus[i] = listpackageDependsStatus[i];

    //update view
    const QModelIndex idxStart = index(0);
    const QModelIndex idxEnd = index(m_packageOperateStatus.size() - 1);
    emit dataChanged(idxStart, idxEnd);

    //update scroll
    emit onChangeOperateIndex(-1);
}
