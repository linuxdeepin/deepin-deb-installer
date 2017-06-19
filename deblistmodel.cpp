#include "deblistmodel.h"
#include "debpackage.h"

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
    const DebPackage *package = m_preparedPackages[r];

    switch (role)
    {
    case PackageNameRole:
        return package->name();
    case PackagePathRole:
        return package->path();
    case PackageVersionRole:
        return package->version();
    case PackageDescriptionRole:
        return package->description();
    default:;
    }

    return QVariant();
}

void DebListModel::appendPackage(DebPackage *package)
{
    m_preparedPackages.append(package);
}
