#ifndef DEBLISTMODEL_H
#define DEBLISTMODEL_H

#include <QAbstractListModel>

#include <QApt/DebFile>

class DebListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit DebListModel(QObject *parent = 0);
    ~DebListModel();

    enum PackageRole
    {
        PackageNameRole = Qt::DisplayRole,
        UnusedRole = Qt::UserRole,
        PackageVersionRole,
        PackagePathRole,
        PackageDescriptionRole,
    };

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    const QList<QApt::DebFile *> preparedPackages() const { return m_preparedPackages; }

public slots:
    void appendPackage(QApt::DebFile *package);

private:
    QList<QApt::DebFile *> m_preparedPackages;
};

#endif // DEBLISTMODEL_H
