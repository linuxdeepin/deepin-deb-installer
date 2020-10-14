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

#include "utils/result.h"

#include <QFuture>
#include <QObject>

#include <QApt/Backend>
#include <QApt/DebFile>
#include <QThread>
#include <QProcess>

typedef Result<QString> ConflictResult;

class PackageDependsStatus;
class DebListModel;
class DealDependThread;
class PackagesManager : public QObject
{
    Q_OBJECT

    friend class DebListModel;

public:
    explicit PackagesManager(QObject *parent = nullptr);
    ~PackagesManager();

public slots:
    void DealDependResult(int iAuthRes, int iIndex, QString dependName);

signals:
    void DependResult(int, int, QString);
    void enableCloseButton(bool);

public:
    bool isBackendReady();
    bool isArchError(const int idx);

public:
    int packageInstallStatus(const int index);

    const QString packageInstalledVersion(const int index);

    const ConflictResult packageConflictStat(const int index);

    const QStringList packageAvailableDepends(const int index);
    PackageDependsStatus getPackageDependsStatus(const int index);
    const QStringList packageReverseDependsList(const QString &packageName, const QString &sysArch);

    QString package(const int index) const { return m_preparedPackages[index]; }
    QApt::Backend *backend() const { return m_backendFuture.result(); }

    bool appendPackage(QString debPackage);

    void removePackage(const int index, QList<int> listDependInstallMark);

    void reset();
    void resetPackageDependsStatus(const int index);

public:
    QList<int> m_errorIndex;

private:
    const PackageDependsStatus checkDependsPackageStatus(QSet<QString> &choosed_set, const QString &architecture,
                                                         const QList<QApt::DependencyItem> &depends);
    const PackageDependsStatus checkDependsPackageStatus(QSet<QString> &choosed_set, const QString &architecture,
                                                         const QApt::DependencyItem &candicate);
    const PackageDependsStatus checkDependsPackageStatus(QSet<QString> &choosed_set, const QString &architecture,
                                                         const QApt::DependencyInfo &dependencyInfo);

    QApt::Package *packageWithArch(const QString &packageName, const QString &sysArch,
                                   const QString &annotation = QString());

    void packageCandidateChoose(QSet<QString> &choosed_set, const QString &debArch,
                                const QList<QApt::DependencyItem> &dependsList);
    void packageCandidateChoose(QSet<QString> &choosed_set, const QString &debArch,
                                const QApt::DependencyItem &candidateItem);

    const ConflictResult isInstalledConflict(const QString &packageName, const QString &packageVersion,
                                             const QString &packageArch);
    const ConflictResult isConflictSatisfy(const QString &arch, QApt::Package *package);
    const ConflictResult isConflictSatisfy(const QString &arch, const QList<QApt::DependencyItem> &conflicts);

    void resetInstallStatus();

private:
    QFuture<QApt::Backend *> m_backendFuture;

    QList<QString> m_preparedPackages;
    QList<QByteArray> m_preparedMd5;
    QSet<QByteArray> m_appendedPackagesMd5;

    QMap<int, int> m_packageInstallStatus;
    QMap<int, PackageDependsStatus> m_packageDependsStatus;

    int m_DealDependIndex = -1;
    DealDependThread *dthread = nullptr;
    QList<int> m_dependInstallMark;

private:

    // fix bug:https://pms.uniontech.com/zentao/bug-view-37220.html
    // 卸载deepin-wine-plugin-virture 时无法卸载deepin-wine-helper. Temporary solution：Special treatment for these package
    QMap<QString, QString> specialPackage();

private:
    //使用软连接方式解决文件路径中存在空格的问题。
    QString SymbolicLink(QString previousName, QString packageName);
    QString link(QString linkPath, QString packageName);
    bool mkTempDir();
    bool rmTempDir();

private:
    const QString m_tempLinkDir = "/tmp/LinkTemp/";
};

#endif  // PACKAGESMANAGER_H
