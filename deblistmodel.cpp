#include "deblistmodel.h"

#include <QDebug>

using QApt::DebFile;

DebListModel::DebListModel(QObject *parent)
    : QAbstractListModel(parent)
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
    default:;
    }

    return QVariant();
}

void DebListModel::appendPackage(DebFile *package)
{
    m_preparedPackages.append(package);
}
