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
#include <QProcess>

class PackagesManager;
class AptConfigMessage;

class DebListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit DebListModel(QObject *parent = nullptr);

    ~DebListModel();
    /**
     * @brief The PackageRole enum
     * 包的各种数据角色
     */
    enum PackageRole {
        PackageNameRole = Qt::DisplayRole,      //包名
        UnusedRole = Qt::UserRole,              //
        WorkerIsPrepareRole,                    //当前工作是否处于准备状态
        ItemIsCurrentRole,                      //获取当前下标
        PackageVersionRole,                     //包的版本
        PackagePathRole,                        //包的路径
        PackageInstalledVersionRole,            //包已经安装的版本
        PackageDescriptionRole,                 //包的描述
        PackageVersionStatusRole,               //包的安装状态
        PackageDependsStatusRole,               //包的依赖状态
        PackageAvailableDependsListRole,        //包可用的依赖
        PackageFailReasonRole,                  //包安装失败的原因
        PackageOperateStatusRole,               //包的操作状态
        PackageReverseDependsListRole,          //依赖于此包的程序
    };

    /**
     * @brief The WorkerStatus enum
     * 安装器的工作状态
     */
    enum WorkerStatus {
        WorkerPrepare,                          //准备，可以进行安装或卸载
        WorkerProcessing,                       //正在安装
        WorkerFinished,                         //安装结束
        WorkerUnInstall                         //正在卸载
    };

    /**
     * @brief The PackageInstallStatus enum
     * 包的安装状态
     */
    enum PackageInstallStatus {
        NotInstalled,                           //当前包没有被安装
        InstalledSameVersion,                   //当前已经安装过相同的版本
        InstalledEarlierVersion,                //当前已经安装过较早的版本
        InstalledLaterVersion,                  //当前已经安装过更新的版本
    };

    /**
     * @brief The PackageDependsStatus enum
     * 当前包的依赖状态
     */
    enum DependsStatus {
        DependsOk,              //依赖满足
        DependsAvailable,       //依赖可用但是需要下载
        DependsBreak,           //依赖不满足
        DependsVerifyFailed,    //签名验证失败
        DependsAuthCancel,      //依赖授权失败（wine依赖）
        ArchBreak,              //架构不满足（此前架构不满足在前端验证，此后会优化到后端）//2020-11-19 暂时未优化
    };

    /**
     * @brief The PackageOperationStatus enum
     * 包的当前操作状态
     */
    enum PackageOperationStatus {
        Prepare,                                //准备安装
        Operating,                              //正在安装
        Success,                                //安装成功
        Failed,                                 //安装失败
        Waiting,                                //等待安装
        VerifyFailed,                           //签名验证失败
    };

    /**
     * @brief The DependsAuthStatus enum
     * wine 依赖安装时的状态
     */
    enum DependsAuthStatus {
        AuthBefore,         //鉴权框弹出之前
        AuthPop,            //鉴权框弹出
        CancelAuth,         //鉴权取消
        AuthConfirm,        //鉴权确认后
        AuthDependsSuccess, //安装成功
        AuthDependsErr,     //安装失败
        AnalysisErr,        //解析错误
    };

    enum ErrorCode {
        NoDigitalSignature = 101,               //无有效的数字签名
        DigitalSignatureError,                  //数字签名校验失败
        ConfigAuthCancel   = 127,               //配置安装授权被取消
    };

    /**
     * @brief reset
     * 重置包的工作状态、安装状态
     * 重置当前处理的下标装啊提
     * 清空安装错误的缓存
     * 重置packageManage的状态
     */
    void reset();

    /**
     * @brief reset_filestatus
     * 重置包的操作状态
     * 清空安装错误原因的缓存
     */
    void reset_filestatus();

    /**
     * @brief isReady 查看后端初始化的状态
     * @return 后端是否准备就绪
     */
    bool isReady() const;

    /**
     * @brief isWorkerPrepare 获取当前的工作状态是否是就绪状态
     * @return 当前是否就绪
     */
    bool isWorkerPrepare() const
    {
        return m_workerStatus == WorkerPrepare;
    }

    /**
     * @brief preparedPackages 获取当前已经添加的包的列表
     * @return 添加的包的列表
     */
    const QList<QString> preparedPackages() const;

    /**
     * @brief first 获取添加的第一个包
     * @return 第一个包的index
     */
    QModelIndex first() const;

    /**
     * @brief rowCount 当前行的数量
     * @param parent
     * @return 当前包的数量
     */
    int rowCount(const QModelIndex &parent) const override;

    /**
     * @brief data  获取某一项的数据
     * @param index 下表
     * @param role  数据的角色
     * @return
     */
    QVariant data(const QModelIndex &index, int role) const override;
    int m_workerStatus_temp = 0;

public:
    /**
     * @brief initPrepareStatus  初始化所有包的状态为Prepare
     */
    void initPrepareStatus();

    /**
     * @brief initDependsStatus 获取某一个包的依赖状态
     * @param index     包的下标
     */
    void initDependsStatus(int index = 0);

public:
    /**
     * @brief getInstallFileSize 获取要安装的包的数量
     * @return 包的数量
     */
    int getInstallFileSize();

signals:
    /**
     * @brief lockForAuth  授权框弹出后 禁用按钮  授权框取消后，启用按钮
     * @param lock 启用/禁用按钮
     */
    void lockForAuth(const bool lock) const;

    /**
     * @brief AuthCancel    授权取消
     */
    void AuthCancel();

    /**
     * @brief onStartInstall  开始安装
     */
    void onStartInstall();

    /**
     * @brief workerFinished  安装结束
     */
    void workerFinished() const;

    /**
     * @brief transactionProgressChanged
     * 某一个包的安装进度改变
     * @param progress
     */
    void transactionProgressChanged(const int progress) const;

    /**
     * @brief workerProgressChanged 整体的安装进度改变
     * @param progress
     */
    void workerProgressChanged(const int progress) const;

    /**
     * @brief packageOperationChanged 包操作状态改变
     * @param index 包的index
     * @param status    修改的状态
     */
    void packageOperationChanged(const QModelIndex &index, int status) const;

    /**
     * @brief packageDependsChanged     包的依赖状态改变
     * @param index     包的index
     * @param status    修改后的状态
     */
    void packageDependsChanged(const QModelIndex &index, int status) const;

    /**
     * @brief appendOutputInfo      安装输出信息
     * @param info      要显示的信息
     */
    void appendOutputInfo(const QString &info) const;

    /**
     * @brief onChangeOperateIndex  index 被修改
     * @param opIndex       修改后的index
     */
    void onChangeOperateIndex(int opIndex);

    /**
     * @brief EnableReCancelBtn         授权框弹出后禁用按钮，授权框取消后启用按钮
     * @param bEnable       启用/禁用按钮的标识
     */
    void EnableReCancelBtn(bool bEnable);

    /**
     * @brief DependResult  依赖下载的状态
     */
    void DependResult(int, QString);

    /**
     * @brief enableCloseButton        设置关闭按钮是否可用
     */
    void enableCloseButton(bool);

    /**
     * @brief invalidPackage 无效包的信号
     */
    void invalidPackage();

    /**
     * @brief packageAlreadyExists 包已添加的信号
     */
    void packageAlreadyExists();

    /**
     * @brief refreshSinglePage 刷新单包安装界面的信号
     */
    void refreshSinglePage();

    /**
     * @brief refreshMultiPage 刷新批量安装model的信号
     */
    void refreshMultiPage();

    /**
     * @brief single2MultiPage 刷新批量安装的信号
     */
    void single2MultiPage();

    /**
     * @brief appendStart 正在添加的信号
     */
    void appendStart();

    /**
     * @brief appendFinished 添加结束的信号
     */
    void appendFinished();

public slots:

    /**
     * @brief setCurrentIndex   设置当前操作的index
     * @param idx       修改后的index
     */
    void setCurrentIndex(const QModelIndex &idx);

    /**
     * @brief installPackages 开始安装所有的包
     */
    void installPackages();

    /**
     * @brief uninstallPackage     卸载某一个包
     * @param idx   包的index
     */
    void uninstallPackage(const int idx);

    /**
     * @brief removePackage 删除某一个包
     * @param idx 要删除的包的index
     */
    void removePackage(const int idx);

    /**
     * @brief appendPackage 添加包
     * @param package 添加的包的路径
     * @return 是否添加成功（主要是判断是否重复添加）
     */
    void appendPackage(QStringList packages);

    /**
     * @brief onTransactionErrorOccurred 安装过程中出现错误
     */
    void onTransactionErrorOccurred();

    /**
     * @brief onTransactionStatusChanged 安装状态出现改变
     * @param stat
     */
    void onTransactionStatusChanged(QApt::TransactionStatus stat);

    /**
     * @brief DealDependResult 处理依赖安装的过程
     * @param iAuthRes      过程标识
     * @param iIndex        包的index（因为这个包的依赖不被满足）
     * @param dependName    发生依赖安装失败的依赖名称
     */
    void DealDependResult(int iAuthRes, int iIndex, QString dependName);

public slots:
    /**
     * @brief ConfigReadOutput 处理配置包的输出并显示
     */
    void ConfigReadOutput();

    /**
     * @brief ConfigInstallFinish 配置结束
     * @param flag 配置安装的结果
     */
    void ConfigInstallFinish(int flag);

    /**
     * @brief ConfigInputWrite 配置的输入数据处理
     * @param str 输入的数据（一般是输入的选项）
     */
    void ConfigInputWrite(QString str);

    /**
     * @brief checkInstallStatus 根据命令返回的消息判断安装状态
     * @param str  命令返回的安装信息
     * 如果命令返回的信息是Cannot run program deepin-deb-installer-dependsInstall: No such file or directory
     * 意味着当前/usr/bin下没有deepin-deb-installer-dependsInstall命令，此版本有问题，需要重新安装deepin-deb-installer-dependsInstall命令
     */
    void checkInstallStatus(QString str);

private slots:

    /**
     * @brief upWrongStatusRow 安装完成后对安装失败的包上滚 其中包括各种状态的变更
     */
    void upWrongStatusRow();

private:

    /**
     * @brief setEndEnable  授权框取消，按钮启用
     */
    void setEndEnable();

    /**
     * @brief checkBoxStatus 检查授权框的弹出状态，判断当前按钮是否需要被禁用
     */
    void checkBoxStatus();

    /**
     * @brief bumpInstallIndex  安装包整体的流程控制
     * 安装完成上一个之后 index 增加
     * 全部安装完成之后 发送安装完成信号
     */
    void bumpInstallIndex();

    /**
     * @brief installNextDeb
     *  判断操作系统的状态，判断签名的状态
     *  检查当前包是否需要配置
     *  安装包
     */
    void installNextDeb();

    /**
     * @brief installDebs 安装单个包。
     */
    void installDebs();

    /**
     * @brief onTransactionOutput
     * 修改当前包的工作状态
     */
    void onTransactionOutput();

    /**
     * @brief onTransactionFinished
     * 当前包安装结束
     */
    void onTransactionFinished();

    /**
     * @brief onDependsInstallTransactionFinished
     * 当前包依赖安装结束
     * 依赖安装成功，安装当前包
     * 依赖安装失败，记录失败原因，安装下一个包
     */
    void onDependsInstallTransactionFinished();

    /**
     * @brief uninstallFinished
     * 卸载结束槽函数
     * 切换工作状态
     * 切换操作状态
     * 发送卸载结束信号
     */
    void uninstallFinished();

    /**
     * @brief refreshOperatingPackageStatus 刷新当前操作的包的操作状态
     * @param stat  要修改的操作状态
     */
    void refreshOperatingPackageStatus(const PackageOperationStatus stat);

    /**
     * @brief packageFailedReason  获取包安装失败的原因
     * @param idx       包的下标
     * @return  包失败原因的描述
     */
    QString packageFailedReason(const int idx) const;

    /**
     * @brief initRowStatus 初始化每一个项的 操作状态
     */
    void initRowStatus();

private:
    /**
     * @brief checkSystemVersion  check 当前操作系统的版本
     * 个人版专业版需要验证数字签名，其余版本不需要
     */
    void checkSystemVersion();

    /**
     * @brief checkDigitalSignature  检查数字签名
     * @return  检查当前包是否有数字签名
     */
    bool checkDigitalSignature();

    /**
     * @brief showNoDigitalErrWindow 弹出无数字签名的错误弹窗
     */
    void showNoDigitalErrWindow();

    /**
     * @brief showNoDigitalErrWindow 弹出数字签名校验错误的错误弹窗
     */
    void showDigitalErrWindow();

    /**
     * @brief DigitalVerifyFailed 数字签名校验失败 弹窗处理的槽函数
     */
    void digitalVerifyFailed(ErrorCode code);

    /**
     * @brief checkDigitalVerifyFailReason 检查当前验证错误的原因
     * @return
     * 如果所有的包安装失败都是由于无数字签名，则弹出前往控制中心的弹窗
     */
    bool checkDigitalVerifyFailReason();

    /**
     * @brief showDevelopModeWindow 打开控制中心通用界面
     */
    void showDevelopModeWindow();

private:

    /**
     * @brief checkTemplate 检查是否需要配置
     * @param debPath   包的路径
     * @return  是否需要配置
     */
    bool checkTemplate(QString debPath);

    /**
     * @brief getDebian    获取包的DEBIAN文件
     * @param debPath      包的路径
     */
    void getDebian(QString debPath);

    /**
     * @brief mkdir     创建临时目录 ，存放DEBIAN文件
     * @return  是否创建成功
     */
    bool mkdir();

    /**
     * @brief rmdir     删除临时目录
     *
     */
    void rmdir();

    /**
     * @brief enableTitleBarFocus
     * 启用TitleBar焦点切换策略
     */
    void enableTitleBarFocus();

    /**
     * @brief getPackageMd5 获取当前操作的包的md5值
     */
    void getPackageMd5(QList<QByteArray> md5);
private:
    int m_workerStatus;                                 //当前工作状态
    int m_operatingIndex;                               //当前正在操作的index
    QByteArray m_operatingPackageMd5;                   //当前正在处理的包的md5
    int m_operatingStatusIndex;                         //当前正在操作的状态的index

    QModelIndex m_currentIdx;                           //当前的index
    PackagesManager *m_packagesManager;                 //后端类

    QPointer<QApt::Transaction> m_currentTransaction;   //当前正在运行的Trans

    // 修改 operateStatus的存放结构，现在与Md5绑定。
    QMap<QByteArray, int> m_packageOperateStatus;              //所有包的操作状态Map

    // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
    QMap<QByteArray, int> m_packageFailCode;                   //FailCode 错误代码 ，trans返回的错误代码
    QMap<QByteArray, QString> m_packageFailReason;             //FailReason , trans返回的详细错误信息

    QProcess *m_procInstallConfig;                      // 配置安装进程
    const QString tempPath = "/tmp/DEBIAN";             // 配置的临时目录

    bool m_isDevelopMode = true;                      // 开发者模式的标志变量 ps：部分系统版本无需签名验证，默认开发者模式

    QList<QByteArray> m_packageMd5;

    AptConfigMessage *configWindow = nullptr;
};

#endif  // DEBLISTMODEL_H
