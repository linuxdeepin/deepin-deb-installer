// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UABPACKAGELISTMODEL_H
#define UABPACKAGELISTMODEL_H

#include <QObject>

#include "uab_package.h"
#include "uab_process_controller.h"
#include "model/abstract_package_list_model.h"

namespace Uab {

class UabPackageListModel : public AbstractPackageListModel
{
    Q_OBJECT
public:
    explicit UabPackageListModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_SLOT void slotAppendPackage(const QStringList &packageList) override;
    void removePackage(int index) override;
    QString checkPackageValid(const QString &packagePath) override;

    Pkg::PackageInstallStatus checkInstallStatus(const QString &packagePath) override;
    Pkg::DependsStatus checkDependsStatus(const QString &packagePath) override;
    QStringList getPackageInfo(const QString &packagePath) override;
    QString lastProcessError() override;

    Q_SLOT bool slotInstallPackages() override;
    Q_SLOT bool slotUninstallPackage(int index) override;

    void reset() override;
    void resetInstallStatus() override;

private:
    bool installNextUab();

    Q_SLOT void slotBackendProgressChanged(float progress);
    Q_SLOT void slotBackendProcessFinished(bool success);

    void setCurrentOperation(Pkg::PackageOperationStatus s);
    bool checkIndexValid(int index) const;
    UabPackage::Ptr preCheckPackage(const QString &packagePath);
    bool packageExists(const UabPackage::Ptr &uabPtr) const;
    bool linglongExists();

private:
    int m_operatingIndex{-1};
    QList<UabPackage::Ptr> m_uabPkgList;
    UabProcessController *m_processor{nullptr};

    QStringList m_delayAppendPackages;  // wait for backend inited.
};

}  // namespace Uab

#endif  // UABPACKAGELISTMODEL_H
