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

#ifndef DEBLISTMODEL_H
#define DEBLISTMODEL_H

#include <QAbstractListModel>
#include <QFuture>
#include <QPointer>

#include <QApt/Backend>
#include <QApt/DebFile>
#include <QApt/Transaction>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <DPushButton>
#include <DSysInfo>
class PackagesManager;
class DebListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit DebListModel(QObject *parent = nullptr);
    enum PackageRole {
        PackageNameRole = Qt::DisplayRole,
        UnusedRole = Qt::UserRole,
        WorkerIsPrepareRole,
        ItemIsCurrentRole,
        PackageVersionRole,
        PackagePathRole,
        PackageInstalledVersionRole,
        PackageDescriptionRole,
        PackageVersionStatusRole,
        PackageDependsStatusRole,
        PackageAvailableDependsListRole,
        PackageFailReasonRole,
        PackageOperateStatusRole,
        PackageReverseDependsListRole,
    };

    enum WorkerStatus {
        WorkerPrepare,
        WorkerProcessing,
        WorkerFinished,
        WorkerUnInstall
    };

    enum PackageInstallStatus {
        NotInstalled,
        InstalledSameVersion,
        InstalledEarlierVersion,
        InstalledLaterVersion,
    };

    enum PackageDependsStatus {
        DependsOk,  //依赖满足
        DependsAvailable,
        DependsBreak,  //依赖不满足
        DependsVerifyFailed,
        DependsAuthCancel,
    };

    enum PackageOperationStatus {
        Prepare,
        Operating,
        Success,
        Failed,
        Waiting,
        VerifyFailed,
    };

    enum DependsAuthStatus {
        AuthBefore,   //鉴权框弹出之前
        AuthPop,      //鉴权框弹出
        CancelAuth,   //鉴权取消
        AuthConfirm,  //鉴权确认后
        AuthDependsSuccess, //安装成功
        AuthDependsErr,  //安装失败
        AnalysisErr,     //解析错误
    };

    void reset();
    void reset_filestatus();
    bool isReady() const;
    bool isWorkerPrepare() const
    {
        return m_workerStatus == WorkerPrepare;
    }
//    const QList<QApt::DebFile *> preparedPackages() const;
    const QList<QString> preparedPackages() const;
    QModelIndex first() const;

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int DebInstallFinishedFlag = 0;
    int m_workerStatus_temp = 0;

public:
    void initPrepareStatus();
    void initDependsStatus(int index = 0);

public:
    int getInstallFileSize();

signals:
    //    void workerStarted() const;
    void lockForAuth(const bool lock) const;
    void workerFinished() const;
    void workerProgressChanged(const int progress) const;
    void transactionProgressChanged(const int progress) const;
    void appendOutputInfo(const QString &info) const;
    void packageOperationChanged(const QModelIndex &index, int status) const;
    void packageDependsChanged(const QModelIndex &index, int status) const;

    void onChangeOperateIndex(int opIndex);
    void AuthCancel();
    void EnableReCancelBtn(bool bEnable);
    void onStartInstall();
    void DependResult(int, QString);
    void CommitErrorFinished();
    void enableCloseButton(bool);

public slots:
    void setCurrentIndex(const QModelIndex &idx);
    void installAll();
    void uninstallPackage(const int idx);
    void removePackage(const int idx);
    bool getPackageIsNull();
    bool appendPackage(QString packagey);
    void onTransactionErrorOccurred();
    void onTransactionStatusChanged(QApt::TransactionStatus stat);
    void DealDependResult(int iAuthRes, int iIndex, QString dependName);

private slots:
    void upWrongStatusRow();

private:
    void setEndEnable();
    void checkBoxStatus();
    void bumpInstallIndex();
    void onTransactionOutput();
    void onTransactionFinished();
    void onDependsInstallTransactionFinished();
    void installNextDeb();
    void uninstallFinished();
    void refreshOperatingPackageStatus(const PackageOperationStatus stat);
    QString packageFailedReason(const int idx) const;
    void initRowStatus();

    void installDebs();
    void showNoDigitalErrWindow();
private:
    int m_workerStatus;
    int m_operatingIndex;
    int m_operatingStatusIndex;
    QModelIndex m_currentIdx;
    PackagesManager *m_packagesManager;

    QPointer<QApt::Transaction> m_currentTransaction;

    QMap<int, int> m_packageOperateStatus;
    QMap<int, int> m_packageFailCode; //FailCode 错误代码 ，trans返回的错误代码
    QMap<int, QString> m_packageFailReason; //FailReason , trans返回的详细错误信息
    bool m_InitRowStatus;
    bool QverifyResult;
    bool bModifyFailedReason = false;
};

#endif  // DEBLISTMODEL_H
