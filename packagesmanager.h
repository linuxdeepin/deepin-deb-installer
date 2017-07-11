#ifndef PACKAGESMANAGER_H
#define PACKAGESMANAGER_H

#include <QObject>
#include <QFuture>

#include <QApt/Backend>
#include <QApt/DebFile>

class DebListModel;
class PackagesManager : public QObject
{
    Q_OBJECT

    friend class DebListModel;

public:
    explicit PackagesManager(QObject *parent = 0);

    bool isBackendReady();
    int packageInstallStatus(const int index);
    int packageDependsStatus(const int index);
    const QString packageInstalledVersion(const int index);
    const QStringList packageAvailableDependsList(const int index);

    void resetPackageDependsStatus(const int index);

    QApt::DebFile * const package(const int index) const { return m_preparedPackages[index]; }
    QApt::Backend * const backend() const { return m_backendFuture.result(); }

private:
    int checkDependsPackageStatus(const QString &architecture, const QApt::DependencyInfo &dependencyInfo);

private:
    QFuture<QApt::Backend *> m_backendFuture;
    QList<QApt::DebFile *> m_preparedPackages;
    QHash<int, int> m_packageInstallStatus;
    QHash<int, int> m_packageDependsStatus;
};

#endif // PACKAGESMANAGER_H
