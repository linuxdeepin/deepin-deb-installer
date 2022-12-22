// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEBLISTMODEL_H
#define DEBLISTMODEL_H

#include "manager/packagesmanager.h"
#include "process/Pty.h"

#include <QApt/Backend>
#include <QApt/DebFile>
#include <QApt/Transaction>

#include <DSysInfo>
#include <DPushButton>
#include <DDialog>

#include <QAbstractListModel>
#include <QFuture>
#include <QPointer>
#include <QDBusInterface>
#include <QDBusReply>
#include <QProcess>
#include <QKeyEvent>

DWIDGET_USE_NAMESPACE

class AptConfigMessage;

class Dialog : public DDialog
{
    Q_OBJECT
public:
    explicit Dialog();
    void keyPressEvent(QKeyEvent *event);
signals:
    void signalClosed();
};

class DebListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit DebListModel(QObject *parent = nullptr);

    ~DebListModel() override;

    /**
     * @brief isDpkgRunning 判断当前dpkg 是否在运行
     * @return
     */
    static bool isDpkgRunning();
    /**
     * @brief netErrors
     * @return the List of The Error infomations.
     * 无网络安装依赖时，库返回错误为FetechError 偶尔为CommitError
     * 此函数处理库返回CommitError时，网络错误的各种情况，如果错误信息中包含此列表中的信息，则判断为网络原因。
     */
    static const QStringList netErrors();

    /**
     * @brief workerErrorString 根据安装失败的代码显示安装失败的原因。
     * @param errorCode 库或自定义的安装失败代码
     * @param errorInfo 库返回的或自定义的安装错误的原因
     * @return 要显示的安装失败的原因
     */
    static const QString workerErrorString(const int errorCode, const QString errorInfo);
    /**
     * @brief The PackageRole enum
     * 包的各种数据角色
     */
    enum PackageRole {
        PackageNameRole = Qt::DisplayRole,   //包名
        UnusedRole = Qt::UserRole,           //
        WorkerIsPrepareRole,                //当前工作是否处于准备状态
        ItemIsCurrentRole,                  //获取当前下标
        PackageVersionRole,                 //包的版本
        PackagePathRole,                    //包的路径
        PackageInstalledVersionRole,        //包已经安装的版本
        PackageShortDescriptionRole,        //包的短描述
        PackageLongDescriptionRole,         //包的长描述
        PackageVersionStatusRole,           //包的安装状态
        PackageDependsStatusRole,           //包的依赖状态
        PackageAvailableDependsListRole,    //包可用的依赖
        PackageFailReasonRole,              //包安装失败的原因
        PackageOperateStatusRole,           //包的操作状态
        PackageReverseDependsListRole,      //依赖于此包的程序
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
        Prohibit,               //应用被域管限制，无法安装
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
        NoDigitalSignature      = 101, //无有效的数字签名
        DigitalSignatureError,         //数字签名校验失败
        ConfigAuthCancel        = 127, //配置安装授权被取消
        ApplocationProhibit     = 404, //当前包在黑名单中禁止安装
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
     * @brief resetFilestatus
     * 重置包的操作状态
     * 清空安装错误原因的缓存
     */
    void resetFileStatus();

    /**
     * @brief isWorkerPrepare 获取当前的工作状态是否是就绪状态
     * @return 当前是否就绪
     */
    bool isWorkerPrepare() const;

    /**
     * @brief isReady 查看后端初始化的状态
     * @return 后端是否准备就绪
     */
    bool isReady() const;

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

    /**
     * @brief isDevelopMode 是否是开发者模式
     * @return
     */
    bool isDevelopMode();

    /**
     * @brief selectedIndexRow 当前选择安装包列表的行
     * @param row 行号
     */
    void selectedIndexRow(int row);

public:
    /**
     * @brief initPrepareStatus  初始化所有包的状态为Prepare
     */
    void initPrepareStatus();

public:
    /**
     * @brief getInstallFileSize 获取要安装的包的数量
     * @return 包的数量
     */
    int getInstallFileSize();

public:
    /**
     * @brief getWorkerStatus 获取当前安装器的安装状态
     * @return 当前安装器的安装状态
     */
    int getWorkerStatus();

    /**
     * @brief setWorkerStatus 设置当前安装器的安装状态
     * @param workerStatus 当前需要设置的安装状态
     */
    void setWorkerStatus(int workerStatus);

    /**
     * @brief removePackage 删除某一个包
     * @param idx 要删除的包的index
     */
    void removePackage(const int idx);

    /**
     * @brief checkPackageDigitalSignature 查找指定包安装状态
     * @param package_path 路径
     */
    int checkInstallStatus(const QString &package_path);
    /**
     * @brief searchPackageInstallInfo 查找指定包依赖
     * @param package_path 路径
     */
    int checkDependsStatus(const QString &package_path);
    /**
     * @brief searchPackageInstallInfo 查找指定包数字签名
     * @param package_path 路径
     */
    int checkDigitalSignature(const QString &package_path);
    /**
     * @brief searchPackageInstallInfo 查找指定包信息
     * @param package_path 路径
     */
    QStringList getPackageInfo(const QString &package_path);
    /**
     * @brief getInstallErrorMessage 安装错误信息
     */
    QString getInstallErrorMessage();
    /**
     * @brief checkPackageValid 查找指定包信息
     * @param package_path 路径
     */
    QString checkPackageValid(const QString &package_path);

signals:
    /**
     * @brief signalLockForAuth  授权框弹出后 禁用按钮  授权框取消后，启用按钮
     * @param lock 启用/禁用按钮
     */
    void signalLockForAuth(const bool lock) const;

    /**
     * @brief signalAuthCancel    授权取消
     */
    void signalAuthCancel();

    /**
     * @brief signalStartInstall  开始安装
     */
    void signalStartInstall();

    /**
     * @brief signalWorkerFinished  安装结束
     */
    void signalWorkerFinished() const;

    /**
     * @brief signalTransactionProgressChanged
     * 某一个包的安装进度改变
     * @param progress
     */
    void signalTransactionProgressChanged(const int progress) const;

    /**
     * @brief signalWorkerProgressChanged 整体的安装进度改变
     * @param progress
     */
    void signalWorkerProgressChanged(const int progress) const;

    /**
     * @brief signalPackageOperationChanged 包操作状态改变
     * @param index 包的index
     * @param status    修改的状态
     */
    void signalPackageOperationChanged(const QModelIndex &index, int status) const;

    /**
     * @brief signalPackageDependsChanged     包的依赖状态改变
     * @param index     包的index
     * @param status    修改后的状态
     */
    void signalPackageDependsChanged(const QModelIndex &index, int status) const;

    /**
     * @brief signalAppendOutputInfo      安装输出信息
     * @param info      要显示的信息
     */
    void signalAppendOutputInfo(const QString &info) const;

    /**
     * @brief signalChangeOperateIndex  index 被修改
     * @param opIndex       修改后的index
     */
    void signalChangeOperateIndex(int opIndex);

    /**
     * @brief signalEnableReCancelBtn         授权框弹出后禁用按钮，授权框取消后启用按钮
     * @param bEnable       启用/禁用按钮的标识
     */
    void signalEnableReCancelBtn(bool bEnable);

    /**
     * @brief signalDependResult  依赖下载的状态
     */
    void signalDependResult(int, QString);

    /**
     * @brief signalEnableCloseButton        设置关闭按钮是否可用
     */
    void signalEnableCloseButton(bool);

    /**
     * @brief signalInvalidPackage 无效包的信号
     */
    void signalInvalidPackage();

    /**
     * @brief signalNotLocalPackage 包不在本地的信号
     *
     * ps: 包不在本地无法安装
     */
    void signalNotLocalPackage();

    /**
     * @brief signalPackageCannotFind 包已经被移动的信号 通知前端发送浮动消息
     * @param packageName 被移动的文件名
     */
    void signalPackageCannotFind(QString packageName) const;


    /**
     * @brief signalPackageAlreadyExists 包已添加的信号
     */
    void signalPackageAlreadyExists();

    /**
     * @brief signalSingleDependPackages 单包依赖关系显示信号
     * @param breakPackages
     */
    void signalSingleDependPackages(DependsPair breakPackages, bool intallWineDepends);

    /**
     * @brief signalMultDependPackages 批量包依赖关系显示信号
     * @param breakPackages
     */
    void signalMultDependPackages(DependsPair breakPackages, bool intallWineDepends);
signals:
    /**
     * @brief signalRefreshSinglePage 刷新单包安装界面的信号
     */
    void signalRefreshSinglePage();

    /**
     * @brief signalRefreshMultiPage 刷新批量安装model的信号
     */
    void signalRefreshMultiPage();

    /**
     * @brief signalSingle2MultiPage 刷新批量安装的信号
     */
    void signalSingle2MultiPage();

    /**
     * @brief signalRefreshFileChoosePage 刷新首页
     */
    void signalRefreshFileChoosePage();

    /**
     * @brief signalAppendStart 正在添加的信号
     */
    void signalAppendStart();

    /**
     * @brief signalAppendFinished 添加结束的信号
     */
    void signalAppendFinished();

public slots:

    /**
     * @brief slotSetCurrentIndex   设置当前操作的index
     * @param idx       修改后的index
     */
    void slotSetCurrentIndex(const QModelIndex &idx);

    /**
     * @brief slotInstallPackages 开始安装所有的包
     */
    void slotInstallPackages();

    /**
     * @brief slotUninstallPackage     卸载某一个包
     * @param index   包的index
     */
    void slotUninstallPackage(const int index);

    /**
     * @brief slotAppendPackage 添加包
     * @param package 添加的包的路径
     */
    void slotAppendPackage(QStringList packages);

    /**
     * @brief slotTransactionErrorOccurred 安装过程中出现错误
     */
    void slotTransactionErrorOccurred();

    /**
     * @brief slotTransactionStatusChanged 安装状态出现改变
     * @param TransactionStatus
     */
    void slotTransactionStatusChanged(QApt::TransactionStatus TransactionStatus);

    /**
     * @brief slotDealDependResult 处理依赖安装的过程
     * @param authType      授权类型
     * @param dependIndex   依赖包的index（因为这个包的依赖不被满足）
     * @param dependName    发生依赖安装失败的依赖名称
     */
    void slotDealDependResult(int authType, int dependIndex, QString dependName);

public slots:
    /**
     * @brief slotConfigReadOutput 处理配置包的输出并显示
     */
    void slotConfigReadOutput(const char *buffer, int length, bool isCommandExec);

    /**
     * @brief slotConfigInstallFinish 配置结束
     * @param flag 配置安装的结果
     */
    void slotConfigInstallFinish(int flag);

    /**
     * @brief slotConfigInputWrite 配置的输入数据处理
     * @param str 输入的数据（一般是输入的选项）
     */
    void slotConfigInputWrite(QString str);

    /**
     * @brief slotCheckInstallStatus 根据命令返回的消息判断安装状态
     * @param str  命令返回的安装信息
     * 如果命令返回的信息是Cannot run program deepin-deb-installer-dependsInstall: No such file or directory
     * 意味着当前/usr/bin下没有deepin-deb-installer-dependsInstall命令，此版本有问题，需要重新安装deepin-deb-installer-dependsInstall命令
     */
    void slotCheckInstallStatus(QString str);

private slots:

    /**
     * @brief slotUpWrongStatusRow 安装完成后对安装失败的包上滚 其中包括各种状态的变更
     */
    void slotUpWrongStatusRow();

    /**
     * @brief slotTransactionOutput
     * 修改当前包的工作状态
     */
    void slotTransactionOutput();

    /**
     * @brief slotTransactionFinished
     * 当前包安装结束
     */
    void slotTransactionFinished();

    /**
     * @brief  slotDependsInstallTransactionFinished();

     * 当前包依赖安装结束
     * 依赖安装成功，安装当前包
     * 依赖安装失败，记录失败原因，安装下一个包
     */
    void slotDependsInstallTransactionFinished();

    /**
     * @brief slotUninstallFinished
     * 卸载结束槽函数
     * 切换工作状态
     * 切换操作状态
     * 发送卸载结束信号
     */
    void slotUninstallFinished();

    /**
     * @brief slotNoDigitalSignature 无数字签名
     */
    void slotNoDigitalSignature();

    /**
     * @brief slotDigitalSignatureError 数字签名校验失败
     */
    void slotDigitalSignatureError();

    /**
     * @brief showDevelopModeWindow 打开控制中心通用界面
     */
    void slotShowDevelopModeWindow();

    /**
     * @brief slotShowProhibitWindow 应用在域管黑名单中，无法安装
     */
    void slotShowProhibitWindow();

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
     * @brief refreshOperatingPackageStatus 刷新当前操作的包的操作状态
     * @param oprationStatus  要修改的操作状态
     */
    void refreshOperatingPackageStatus(PackageOperationStatus oprationStatus);

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
     * @brief showDevelopDigitalErrWindow 开发者模式下弹出数字签名无效的弹窗
     */
    void showDevelopDigitalErrWindow(ErrorCode code);

    /**
     * @brief showProhibitWindow 弹出数字签名校验错误的错误弹窗
     */
    void showProhibitWindow();

    /**
     * @brief 检查当前将要安装的包是否在黑名单中。
     *
     * @return true 当前要安装的包在黑名单中
     * @return false 当前要安装的包不在黑名单中
     */
    bool checkBlackListApplication();

    /**
     * @brief 数字签名校验失败 弹窗处理的槽函数
     *
     * @param errorCode 错误原因代码
     */
    void digitalVerifyFailed(ErrorCode errorCode);

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
     * @brief Get the Package Md5 object
     *
     * @param packagesMD5 所有包的md5的列表
     */
    void getPackageMd5(const QList<QByteArray> &packagesMD5);

//// 文件移动、删除、修改检查
private:
    /**
     * @brief recheckPackagePath 重新检查文件路径
     * @param packagePath 当前需要检查的文件路径
     * @return 检查的结果
     *         true : 文件存在
     *         false: 文件不存在
     */
    bool recheckPackagePath(QString packagePath) const;

private:
    /**
     * @brief initConnections 初始化信号与槽的链接
     */
    void initConnections();

    /**
     * @brief initInstallConnecions 链接安装过程中的信号与槽
     */
    void initInstallConnections();

    /**
     * @brief initAppendConnection 链接添加过程中的信号与槽
     */
    void initAppendConnection();

    /**
     * @brief initRefreshPageConnecions 链接页面刷新的信号与槽
     */
    void initRefreshPageConnecions();

private:
    //当前工作状态
    int m_workerStatus                  = 0;

    //当前正在操作的index
    int m_operatingIndex                = 0;

    //当前正在操作的状态的index
    int m_operatingStatusIndex          = 0;

    //当前正在处理的包的md5
    QByteArray m_operatingPackageMd5    = nullptr;


    //当前的index
    QModelIndex m_currentIdx;

    //后端类
    PackagesManager *m_packagesManager = nullptr;

    //当前正在运行的Trans
    QPointer<QApt::Transaction> m_currentTransaction;

    //所有包的操作状态Map
    QMap<QByteArray, int> m_packageOperateStatus = {};

    //FailCode 错误代码 ，trans返回的错误代码
    QMap<QByteArray, int> m_packageFailCode = {};

    //FailReason , trans返回的详细错误信息
    QMap<QByteArray, QString> m_packageFailReason = {};

    // 配置安装进程
    Konsole::Pty *m_procInstallConfig = {};

    // 配置的临时目录
    const QString tempPath = "/tmp/DEBIAN";

    QString m_brokenDepend = "";

    // 开发者模式的标志变量
    //部分系统版本无需签名验证，默认开发者模式
    bool m_isDevelopMode = true;
    bool m_isDigitalVerify = false;

    QList<QByteArray> m_packageMd5 = {};

    AptConfigMessage *configWindow = nullptr;
};

#endif  // DEBLISTMODEL_H
