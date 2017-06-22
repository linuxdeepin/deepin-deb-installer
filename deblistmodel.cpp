#include "deblistmodel.h"

#include <QDebug>
#include <QSize>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

#include <QApt/Package>
#include <QApt/Backend>

using namespace QApt;

Backend *init_backend()
{
    Backend *b = new Backend;

    if (b->init())
        return b;

    qFatal("%s", b->initErrorMessage().toStdString().c_str());
    return nullptr;
}

DebListModel::DebListModel(QObject *parent)
    : QAbstractListModel(parent),

      m_installerStatus(InstallerPrepare)
{
    m_backendFuture = QtConcurrent::run(init_backend);
}

DebListModel::~DebListModel()
{
    m_backendFuture.result()->deleteLater();

    qDeleteAll(m_preparedPackages);
}

int DebListModel::packageInstallStatus(const QModelIndex &index)
{
    const int r = index.row();

    if (m_packageInstallStatus.contains(r))
        return m_packageInstallStatus[r];

    const QString packageName = m_preparedPackages[r]->packageName();
    Backend *b = m_backendFuture.result();
    Package *p = b->package(packageName);

    const QString installedVersion = p->installedVersion();
    if (installedVersion.isEmpty())
    {
        m_packageInstallStatus.insert(r, NotInstalled);
        return NotInstalled;
    }

    const QString packageVersion = m_preparedPackages[r]->version();
    const int result = Package::compareVersion(packageVersion, installedVersion);

    int ret;
    if (result == 0)
        ret = InstalledSameVersion;
    else if (result > 0)
        ret = InstalledLaterVersion;
    else
        ret = InstalledEarlierVersion;

    m_packageInstallStatus.insert(r, ret);
    return ret;
}

int DebListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_preparedPackages.size();
}

QVariant DebListModel::data(const QModelIndex &index, int role) const
{
    const int r = index.row();
    const DebFile *package = m_preparedPackages[r];

    switch (role)
    {
    case PackageNameRole:
        return package->packageName();
    case PackagePathRole:
        return package->filePath();
    case PackageVersionRole:
        return package->version();
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
    m_opIter = m_preparedPackages.begin();

    // start first
    installNextDeb();
}

void DebListModel::appendPackage(DebFile *package)
{
    Q_ASSERT_X(m_installerStatus == InstallerPrepare, Q_FUNC_INFO, "installer status error");

    m_preparedPackages.append(package);
}

void DebListModel::installNextDeb()
{
    Q_ASSERT_X(m_installerStatus == InstallerInstalling, Q_FUNC_INFO, "installer status error");

    // install finished
    if (m_opIter == m_preparedPackages.end())
    {
        qDebug() << "congraulations, install finished !!!";

        m_installerStatus = InstallerFinished;
        emit installerFinished();
        return;
    }

    // fetch next deb
    DebFile *deb = *m_opIter++;

    qDebug() << Q_FUNC_INFO << "starting to install package: " << deb->packageName();

    m_currentTransaction = m_backendFuture.result()->installFile(*deb);
    Transaction *trans = m_currentTransaction.data();

    connect(trans, &Transaction::statusDetailsChanged, this, &DebListModel::appendOutputInfo);
    connect(trans, &Transaction::finished, this, &DebListModel::installNextDeb);
    connect(trans, &Transaction::finished, trans, &Transaction::deleteLater);

    trans->run();
}
