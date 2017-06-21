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
    : QAbstractListModel(parent)
{
    m_backendFuture = QtConcurrent::run(init_backend);
}

DebListModel::~DebListModel()
{
    m_backendFuture.result()->deleteLater();

    qDeleteAll(m_preparedPackages);
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

void DebListModel::appendPackage(DebFile *package)
{
    m_preparedPackages.append(package);

    // test
    Backend *b = m_backendFuture.result();
    Package *p = b->package(package->packageName());

    qDebug() << p->installedVersion();
}
