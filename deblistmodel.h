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
    };

    enum WorkerStatus
    {
        WorkerPrepare,
        WorkerProcessing,
        WorkerFailed,
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

    const QList<QApt::DebFile *> preparedPackages() const;

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

signals:
    void workerStarted() const;
    void workerFinished() const;
    void workerProgressChanged(const double progress) const;
    void transactionProgressChanged(const int progress) const;
    void appendOutputInfo(const QString &info) const;
    void packageOperationChanged(const QModelIndex &index, int status) const;
    void packageDependsChanged(const QModelIndex &index, int status) const;

public slots:
    void installAll();
    void uninstallPackage(const int idx);
    void appendPackage(QApt::DebFile *package);

private:
    void installNextDeb();
    void uninstallFinished();

private:
    int m_workerStatus;
    PackagesManager *m_packagesManager;

    QList<QApt::DebFile *>::iterator m_opIter;
    QPointer<QApt::Transaction> m_currentTransaction;

};

#endif // DEBLISTMODEL_H
