/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PACKAGESMANAGER_H
#define PACKAGESMANAGER_H

#include "result.h"

#include <QFuture>
#include <QObject>

#include <QApt/Backend>
#include <QApt/DebFile>

typedef Result<QString> ConflictResult;

namespace  PackagesManagerDependsStatus {

class PackageDependsStatus
{
public:
    static PackageDependsStatus ok();
    static PackageDependsStatus available();
    static PackageDependsStatus _break(const QString &package);

    PackageDependsStatus();
    PackageDependsStatus(const int status, const QString &package);
    PackageDependsStatus operator=(const PackageDependsStatus &other);

    PackageDependsStatus max(const PackageDependsStatus &other);
    PackageDependsStatus maxEq(const PackageDependsStatus &other);
    PackageDependsStatus min(const PackageDependsStatus &other);
    PackageDependsStatus minEq(const PackageDependsStatus &other);

    bool isBreak() const;
    bool isAvailable() const;
    bool isForbid() const;
    bool isArchBreak() const;

public:
    int status;
    QString package;
};
}

using namespace  PackagesManagerDependsStatus;

class DebListModel;
class PackagesManager : public QObject
{
    Q_OBJECT

    friend class DebListModel;

public:
    explicit PackagesManager(QObject *parent = nullptr);

    bool isBackendReady();
    bool isArchError(const int idx);
    bool isArchError(QString debPath);
    const ConflictResult packageConflictStat(const int index);
    const ConflictResult isConflictSatisfy(const QString &arch, QApt::Package *package);
    const ConflictResult isInstalledConflict(const QString &packageName, const QString &packageVersion,
                                             const QString &packageArch);
    const ConflictResult isConflictSatisfy(const QString &arch, const QList<QApt::DependencyItem> &conflicts);
    int packageInstallStatus(const int index);
    void addPackageInstallStatus(QString debPath);
    PackageDependsStatus packageDependsStatus(const int index);
    void addDependsStatus(QString packagePath);
    const QString packageInstalledVersion(const int index);
    const QStringList packageAvailableDepends(const int index);
    void packageCandidateChoose(QSet<QString> &choosed_set, const QString &debArch,
                                const QList<QApt::DependencyItem> &dependsList);
    void packageCandidateChoose(QSet<QString> &choosed_set, const QString &debArch,
                                const QApt::DependencyItem &candidateItem);
    const QStringList packageReverseDependsList(const QString &packageName, const QString &sysArch);

    void reset();
    void resetInstallStatus();
    void resetPackageDependsStatus(const int index);
    void removePackage(const int index);
    bool appendPackage(QString packagePath);
    bool QverifyResult;
    QString package(const int index) const {return m_preparedPackages[index];  }
    QApt::Backend *backend() const { return m_backendFuture.result(); }

private:
    const PackageDependsStatus checkDependsPackageStatus(QSet<QString> &choosed_set, const QString &architecture,
                                                         const QList<QApt::DependencyItem> &depends);
    const PackageDependsStatus checkDependsPackageStatus(QSet<QString> &choosed_set, const QString &architecture,
                                                         const QApt::DependencyItem &candicate);
    const PackageDependsStatus checkDependsPackageStatus(QSet<QString> &choosed_set, const QString &architecture,
                                                         const QApt::DependencyInfo &dependencyInfo);
    QApt::Package *packageWithArch(const QString &packageName, const QString &sysArch,
                                   const QString &annotation = QString());

    bool checkAppPermissions(QString packagePath);
    QStringList getAppList(QString listPath);
    QStringList getPermissionList(QStringList whiteList, QStringList blackList);
    bool detectAppPermission(QString tempPath);
    bool deleteDirectory(const QString &path);

private:
    QFuture<QApt::Backend *> m_backendFuture;
    QList<QString> m_preparedPackages;

    QList<int> m_packageInstallStatus;
    QList<PackageDependsStatus>m_packageDependsStatus;
    QSet<QByteArray> m_appendedPackagesMd5;

    QList<QByteArray> m_packageMd5;
    QMap<QByteArray, PackageDependsStatus> m_packageMd5Status;

    QMap<QString, int >m_packagePermissionStatus;
    QStringList authorizedAppList;
};

#endif  // PACKAGESMANAGER_H
