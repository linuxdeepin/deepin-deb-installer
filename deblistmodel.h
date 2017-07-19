#ifndef DEBLISTMODEL_H
#define DEBLISTMODEL_H

#include <QAbstractListModel>
#include <QFuture>
#include <QPointer>

#include <QApt/DebFile>
#include <QApt/Backend>
#include <QApt/Transaction>

class PackagesManager;
class DebListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit DebListModel(QObject *parent = 0);

    enum PackageRole
    {
        PackageNameRole = Qt::DisplayRole,
        UnusedRole = Qt::UserRole,
        PackageVersionRole,
        PackagePathRole,
        PackageInstalledVersionRole,
        PackageDescriptionRole,
        PackageVersionStatusRole,
        PackageDependsStatusRole,
        PackageAvailableDependsListRole,
        PackageFailReasonRole,
        PackageOperateStatusRole,
    };

    enum WorkerStatus
    {
        WorkerPrepare,
        WorkerProcessing,
        WorkerFinished,
    };

    enum PackageInstallStatus
    {
        NotInstalled,
        InstalledSameVersion,
        InstalledEarlierVersion,
        InstalledLaterVersion,
    };

    enum PackageDependsStatus
    {
        DependsOk,
        DependsAvailable,
        DependsBreak,
    };

    enum PackageOperationStatus
    {
        Prepare,
        Operating,
        Success,
        Failed,
    };

    bool isReady() const;
    const QList<QApt::DebFile *> preparedPackages() const;
    QModelIndex first() const;

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

signals:
    void workerStarted() const;
    void workerFinished() const;
    void workerProgressChanged(const int progress) const;
    void transactionProgressChanged(const int progress) const;
    void appendOutputInfo(const QString &info) const;
    void packageOperationChanged(const QModelIndex &index, int status) const;
    void packageDependsChanged(const QModelIndex &index, int status) const;

public slots:
    void installAll();
    void uninstallPackage(const int idx);
    void appendPackage(QApt::DebFile *package);
    void onTransactionErrorOccurred();

private:
    void bumpInstallIndex();
    void onTransactionOutput();
    void onTransactionFinished();
    void onDependsInstallTransactionFinished();
    void installNextDeb();
    void uninstallFinished();
    void refreshOperatingPackageStatus(const PackageOperationStatus stat);
    QString packageFailedReason(const int idx) const;

private:
    int m_workerStatus;
    int m_operatingIndex;
    PackagesManager *m_packagesManager;

    QPointer<QApt::Transaction> m_currentTransaction;

    QHash<int, int> m_packageOperateStatus;

};

#endif // DEBLISTMODEL_H
