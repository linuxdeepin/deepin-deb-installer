// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACT_PACKAGE_LIST_MODEL_H
#define ABSTRACT_PACKAGE_LIST_MODEL_H

#include "utils/package_defines.h"

#include <QAbstractListModel>

// interface for deb / uab package
class AbstractPackageListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    // 包的各种数据角色
    enum PackageRole {
        PackageNameRole = Qt::DisplayRole,  // 包名
        UnusedRole = Qt::UserRole,          //
        WorkerIsPrepareRole,                // 当前工作是否处于准备状态
        ItemIsCurrentRole,                  // 获取当前下标
        PackageVersionRole,                 // 包的版本
        PackagePathRole,                    // 包的路径
        PackageInstalledVersionRole,        // 包已经安装的版本
        PackageShortDescriptionRole,        // 包的短描述
        PackageLongDescriptionRole,         // 包的长描述
        PackageVersionStatusRole,           // 包的安装状态
        PackageDependsStatusRole,           // 包的依赖状态
        PackageAvailableDependsListRole,    // 包可用的依赖
        PackageFailReasonRole,              // 包安装失败的原因
        PackageOperateStatusRole,           // 包的操作状态
        PackageReverseDependsListRole,      // 依赖于此包的程序

        PackageDependsDetailRole,  // details for unfinished depends, empty if no errors occur, sa Pkg::DependsPair
    };

    // 安装器的工作状态
    enum WorkerStatus {
        WorkerPrepare,     // 准备，可以进行安装或卸载
        WorkerProcessing,  // 正在安装
        WorkerFinished,    // 安装结束
        WorkerUnInstall    // 正在卸载
    };

    // wine 依赖安装时的状态
    enum DependsAuthStatus {
        AuthBefore,          // 鉴权框弹出之前
        AuthPop,             // 鉴权框弹出
        CancelAuth,          // 鉴权取消
        AuthConfirm,         // 鉴权确认后
        AuthDependsSuccess,  // 安装成功
        AuthDependsErr,      // 安装失败
        AnalysisErr,         // 解析错误
        VerifyDependsErr,    // 验证签名失败(分级管控)
    };

    // Signature fail error code
    enum ErrorCode {
        NoDigitalSignature = 101,   // 无有效的数字签名
        DigitalSignatureError,      // 数字签名校验失败
        ConfigAuthCancel = 127,     // 配置安装授权被取消
        ApplocationProhibit = 404,  // 当前包在黑名单中禁止安装
    };

    explicit AbstractPackageListModel(QObject *parent = nullptr);

    WorkerStatus getWorkerStatus() const;
    void setWorkerStatus(WorkerStatus status);
    inline bool isWorkerPrepare() const { return WorkerPrepare == getWorkerStatus(); }

    Q_SLOT virtual void slotAppendPackage(const QStringList &packageList) = 0;
    virtual void removePackage(const int index) = 0;
    virtual QString checkPackageValid(const QString &packagePath) = 0;

    // trigger install / uninstall
    Q_SLOT virtual void slotInstallPackages() = 0;
    Q_SLOT virtual void slotUninstallPackage(const int index) = 0;

    virtual void reset() = 0;
    virtual void resetInstallStatus() = 0;

Q_SIGNALS:
    // package append flow control
    void signalAppendStart();
    void signalAppendFinished();
    // manange package insert failed reason
    void signalAppendFailMessage(Pkg::AppendFailReason reason);
    // manage package appended and remove
    void signalPackageCountChanged(int count);

    // install/upgrade/uninstall process output info
    void signalAppendOutputInfo(const QString &output);
    // current install progress
    void signalCurrentPacakgeProgressChanged(int progress);
    // whole install progress
    void signalWholeProgressChanged(int progress);
    // current process install package
    void signalCurrentProcessPackageIndex(int index);

    // package removed or rename
    void signalPackageCannotFind(QString packageName) const;

    // install/uninstall flow status, start and finish
    void signalWorkerStart();
    void signalWorkerFinished();

    // These interfaces are used by authorization and wine dependencies, no changes for compatibility.
    // They may be moved to singletons.
    void signalLockForAuth(bool locked) const;
    void signalAuthCancel();
    // set cancel button enabled while open authorization check dialog (pkexec)
    void signalEnableReCancelBtn(bool bEnable);

    // wine depends install progress info.
    void signalDependResult(int, QString);
    // set close button enabled while install wine depends
    void signalEnableCloseButton(bool);

protected:
    WorkerStatus m_workerStatus{WorkerPrepare};  // current worker status

private:
    Q_DISABLE_COPY(AbstractPackageListModel)
};

#endif  // ABSTRACT_PACKAGE_LIST_MODEL_H
