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
#include <QThread>
#include <QProcess>

typedef Result<QString> ConflictResult;

namespace  PackagesManagerDependsStatus {

class dealDependThread : public QThread
{
    Q_OBJECT
public:
    dealDependThread(QObject *parent = nullptr);
    virtual ~dealDependThread();
    void setDependName(QString tmp, int index);
    void run();
signals:
    void DependResult(int, int);

public slots:
    void onFinished(int);
    void on_readoutput();
private:
    QProcess *proc;
    int m_index = -1;
    QString m_dependName;
    bool bDependsStatusErr = false;
};

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
    bool isAuthCancel() const;
    bool isAvailable() const;

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
    ~PackagesManager();

    bool isBackendReady();
    bool isArchError(const int idx);
    const ConflictResult packageConflictStat(const int index);
    const ConflictResult isConflictSatisfy(const QString &arch, QApt::Package *package);
    const ConflictResult isInstalledConflict(const QString &packageName, const QString &packageVersion,
                                             const QString &packageArch);
    const ConflictResult isConflictSatisfy(const QString &arch, const QList<QApt::DependencyItem> &conflicts);
    int packageInstallStatus(const int index);
    PackageDependsStatus packageDependsStatus(const int index);
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
    void removePackage(const int index, QList<int> listDependInstallMark);
    void removeLastPackage();
    bool getPackageIsNull();
    bool appendPackage(QApt::DebFile *debPackage, bool isEmpty);
    bool appendPackage(QString debPackage, bool isEmpty);
    bool QverifyResult;
    QString package(const int index) const { return m_preparedPackages[index]; }
    QApt::Backend *backend() const { return m_backendFuture.result(); }

    QList<int> m_errorIndex;  //Store the subscript that failed to install Deepin-Wine

private:
    const PackageDependsStatus checkDependsPackageStatus(QSet<QString> &choosed_set, const QString &architecture,
                                                         const QList<QApt::DependencyItem> &depends);
    const PackageDependsStatus checkDependsPackageStatus(QSet<QString> &choosed_set, const QString &architecture,
                                                         const QApt::DependencyItem &candicate);
    const PackageDependsStatus checkDependsPackageStatus(QSet<QString> &choosed_set, const QString &architecture,
                                                         const QApt::DependencyInfo &dependencyInfo);
    QApt::Package *packageWithArch(const QString &packageName, const QString &sysArch,
                                   const QString &annotation = QString());

private:
    QFuture<QApt::Backend *> m_backendFuture;
    QList<QString> m_preparedPackages;
    QList<QByteArray> m_preparedMd5;
    QMap<int, int> m_packageInstallStatus;
    QMap<int, PackageDependsStatus> m_packageDependsStatus;
    QSet<QByteArray> m_appendedPackagesMd5;
    int m_DealDependIndex = -1;
    dealDependThread *dthread = nullptr;
    QList<int> m_dependInstallMark;

private:

    // fix bug:https://pms.uniontech.com/zentao/bug-view-37220.html
    // 卸载deepin-wine-plugin-virture 时无法卸载deepin-wine-helper. Temporary solution：Special treatment for these package
    QMap<QString, QString> specialPackage();

public slots:
    void DealDependResult(int iAuthRes, int iIndex);
signals:
    void DependResult(int, int);
};

#endif  // PACKAGESMANAGER_H
