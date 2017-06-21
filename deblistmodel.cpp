#include "deblistmodel.h"

#include <QDebug>
#include <QSize>

#include <QApt/Package>
#include <QApt/Backend>

using QApt::DebFile;
using QApt::Backend;
using QApt::Package;

DebListModel::DebListModel(QObject *parent)
    : QAbstractListModel(parent),

      m_aptBackend(new Backend(this))
{
}

DebListModel::~DebListModel()
{
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
}
