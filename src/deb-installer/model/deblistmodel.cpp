/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
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

#include "deblistmodel.h"
#include "manager/packagesmanager.h"
#include "manager/PackageDependsStatus.h"
#include "view/pages/AptConfigMessage.h"
#include "view/pages/settingdialog.h"
#include "utils/utils.h"


#include <DDialog>
#include <DSysInfo>

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFuture>
#include <QFutureWatcher>
#include <QSize>
#include <QtConcurrent>

#include <QApt/Backend>
#include <QApt/Package>

using namespace QApt;

/**
 * @brief isDpkgRunning 判断当前dpkg 是否在运行
 * @return
 */


DebListModel::DebListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_workerStatus(WorkerPrepare)
    , m_packagesManager(new PackagesManager(this))
{
    // 配置包安装的进程
    m_procInstallConfig = new QProcess;
    m_procInstallConfig->setProcessChannelMode(QProcess::MergedChannels);               //获取子进程所有的输出数据
    m_procInstallConfig->setReadChannel(QProcess::StandardOutput);                      //QProcess 当前从标准输出中读取所有的数据

    configWindow = new AptConfigMessage;

    // 链接信号与槽
    initConnections();
    //检查系统版本与是否开启了开发者模式
    checkSystemVersion();
}

bool DebListModel::isDpkgRunning()
{
    QProcess proc;

    // 获取当前的进程信息
    proc.start("ps", QStringList() << "-e"
               << "-o"
               << "comm");
    proc.waitForFinished();

    // 获取进程信息的数据
    const QString processOutput = proc.readAllStandardOutput();

    // 查看进程信息中是否存在dpkg 存在说明已经正在安装其他包
    if (processOutput.contains("dpkg"))
        return true;   //更换判断的方式

    return false;
}

const QStringList DebListModel::netErrors()
{
    QStringList errorDetails;
    errorDetails << "Address family for hostname not supported";
    errorDetails << "Temporary failure resolving";
    errorDetails << "Network is unreachable";
    errorDetails << "Cannot initiate the connection to";
    return errorDetails;
}

const QString DebListModel::workerErrorString(const int errorCode, const QString errorInfo)
{
    switch (errorCode) {
    case FetchError:
    case DownloadDisallowedError:
        return QApplication::translate("DebListModel", "Installation failed, please check your network connection");    //网络错误
    case NotFoundError:
        return QApplication::translate("DebListModel",
                                       "Installation failed, please check for updates in Control Center");
    case DiskSpaceError:
        return QApplication::translate("DebListModel", "Installation failed, insufficient disk space");                 //存储空间不足
    // lockError 也会报空间不足的错误
    case LockError:
        if (errorInfo.contains("No space left on device")) {
            return QApplication::translate("DebListModel", "Installation failed, insufficient disk space");
        }
        break;
    // fix bug:39834 网络断开时，偶现安装deb包失败时提示语不显示
    case CommitError:
        //commit Error 中会报网络错误
        for (auto error : netErrors()) {
            if (errorInfo.contains(error) && errorInfo.contains("http"))
                return QApplication::translate("DebListModel", "Installation failed, please check your network connection");
        }
        // commitError 有时会出现空间不足的错误
        if (errorInfo.contains("No space left on device")) {
            return QApplication::translate("DebListModel", "Installation failed, insufficient disk space");
        }
        break;
    // 无数字签名的错误
    case DebListModel::NoDigitalSignature:
        return QApplication::translate("DebListModel", "No digital signature");

    //无有效的数字签名
    case DebListModel::DigitalSignatureError:
        return QApplication::translate("DebListModel", "Invalid digital signature");

    // 安装配置包时，没有得到授权
    case DebListModel::ConfigAuthCancel:
        return QApplication::translate("DebListModel",
                                       "Authentication failed");
    case DebListModel::ErrorCode::ApplocationProhibit:
        return QApplication::translate("DebListModel", "The administrator has set policies to prevent installation of this package");
    default:
        break;
    }
    //其余错误，暂不提示具体的错误原因
    return QApplication::translate("DebListModel", "Installation Failed");
}
void DebListModel::initAppendConnection()
{
    //添加的包是无效的包
    connect(m_packagesManager, &PackagesManager::signalInvalidPackage, this, &DebListModel::signalInvalidPackage);

    //添加的包不在本地
    connect(m_packagesManager, &PackagesManager::signalNotLocalPackage, this, &DebListModel::signalNotLocalPackage);

    //要添加的包早就已经被添加到程序中
    connect(m_packagesManager, &PackagesManager::signalPackageAlreadyExists, this, &DebListModel::signalPackageAlreadyExists);

    //告诉前端当前处在添加过程中
    connect(m_packagesManager, &PackagesManager::signalAppendStart, this, &DebListModel::signalAppendStart);

    //提示前端当前已经添加完成
    connect(m_packagesManager, &PackagesManager::signalAppendFinished, this, &DebListModel::getPackageMd5);

    //当前由于文件路径被修改删除md5
    connect(m_packagesManager, &PackagesManager::signalPackageMd5Changed, this, &DebListModel::getPackageMd5);
}

/**
 * @brief DebListModel::initInstallConnecions 链接安装过程的信号与槽
 */
void DebListModel::initInstallConnections()
{
    //安装成功后，根据安装结果排序
    connect(this, &DebListModel::signalWorkerFinished, this, &DebListModel::slotUpWrongStatusRow);

    // 配置安装结束
    connect(m_procInstallConfig, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this, &DebListModel::slotConfigInstallFinish);

    // 配置安装的过程数据
    connect(m_procInstallConfig, &QProcess::readyReadStandardOutput, this, &DebListModel::slotConfigReadOutput);

    // 向安装进程中写入配置信息（一般是配置的序号）
    connect(configWindow, &AptConfigMessage::AptConfigInputStr, this, &DebListModel::slotConfigInputWrite);

    //处理wine依赖安装的过程
    connect(m_packagesManager, &PackagesManager::signalDependResult, this, &DebListModel::slotDealDependResult);

    //安装wine依赖的时候不允许程序退出
    connect(m_packagesManager, &PackagesManager::signalEnableCloseButton, this, &DebListModel::signalEnableCloseButton);

}

/**
 * @brief DebListModel::initRefreshPageConnecions 链接刷新界面的信号与槽
 */
void DebListModel::initRefreshPageConnecions()
{
    //刷新单包安装界面
    connect(m_packagesManager, &PackagesManager::signalRefreshSinglePage, this, &DebListModel::signalRefreshSinglePage);

    //刷新批量安装界面的model
    connect(m_packagesManager, &PackagesManager::signalRefreshMultiPage, this, &DebListModel::signalRefreshMultiPage);

    //刷新批量安装界面
    connect(m_packagesManager, &PackagesManager::signalSingle2MultiPage, this, &DebListModel::signalSingle2MultiPage);

    //刷新首页
    connect(m_packagesManager, &PackagesManager::signalRefreshFileChoosePage, this, &DebListModel::signalRefreshFileChoosePage);

    //显示单包依赖关系
    connect(m_packagesManager, &PackagesManager::signalSingleDependPackages, this, &DebListModel::signalSingleDependPackages);

    //显示批量包依赖关系
    connect(m_packagesManager, &PackagesManager::signalMultDependPackages, this, &DebListModel::signalMultDependPackages);
}

/**
 * @brief DebListModel::initConnections 链接所有的信号与槽
 */
void DebListModel::initConnections()
{
    // 链接添加时的信号与槽
    initAppendConnection();

    // 链接页面刷新的信号与槽
    initRefreshPageConnecions();

    // 链接安装过程的信号与槽
    initInstallConnections();
}

void DebListModel::slotDealDependResult(int authType, int dependIndex, QString dependName)
{
    m_brokenDepend = dependName;
    switch (authType) {
    case DebListModel::CancelAuth:
        m_packageOperateStatus[m_packagesManager->getPackageMd5(dependIndex)] = Prepare;           //取消授权后，缺失wine依赖的包的操作状态修改为prepare
        break;
    case DebListModel::AuthConfirm:                         //确认授权后，状态的修改由debinstaller进行处理
        break;
    case DebListModel::AuthDependsSuccess:                  //安装成功后，状态的修改由debinstaller进行处理
        m_packageOperateStatus[m_packagesManager->getPackageMd5(dependIndex)] = Prepare;
        m_workerStatus = Prepare;
        break;
    case DebListModel::AuthDependsErr:                      //安装失败后，状态的修改由debinstaller进行处理
        break;
    default:
        break;
    }
    emit signalDependResult(authType, dependName);                //发送信号，由debinstaller处理界面状态。
}

bool DebListModel::isReady() const
{
    return m_packagesManager->isBackendReady();
}

bool DebListModel::isWorkerPrepare() const
{
    return m_workerStatus == WorkerPrepare;
}

const QList<QString> DebListModel::preparedPackages() const
{
    return m_packagesManager->m_preparedPackages;
}


QModelIndex DebListModel::first() const
{
    return index(0);
}

int DebListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_packagesManager->m_preparedPackages.size();
}

QVariant DebListModel::data(const QModelIndex &index, int role) const
{
    const int currentRow = index.row();
    // 判断当前下标是否越界
    if (currentRow >= m_packagesManager->m_preparedPackages.size() ) {
        return QVariant();
    }
    //当前给出的路径文件已不可访问.直接删除该文件
    if (!recheckPackagePath(m_packagesManager->package(currentRow))) {
        m_packagesManager->removePackage(currentRow);
        return QVariant();
    }
    const DebFile *deb = new DebFile(m_packagesManager->package(currentRow));

    QString packageName = deb->packageName();                       //包名
    QString filePath = deb->filePath();                             //包的路径
    QString version = deb->version();                               //包的版本
    QString architecture = deb->architecture();                     //包可用的架构
    QString shortDescription = deb->shortDescription();             //包的短描述
    QString longDescription = deb->longDescription();               //包的长描述
    delete deb;                                                     //删除该指针，以免内存泄露
    switch (role) {
    case WorkerIsPrepareRole:
        return isWorkerPrepare();                                   //获取当前工作状态是否准备九局
    case ItemIsCurrentRole:
        return m_currentIdx == index;                               //获取当前的index
    case PackageNameRole:
        return packageName;                                         //获取当前index包的包名
    case PackagePathRole:
        return filePath;                                            //获取当前index包的路径
    case PackageVersionRole:
        return version;                                             //获取当前index包的版本
    case PackageVersionStatusRole:
        return m_packagesManager->packageInstallStatus(currentRow);          //获取当前index包的安装状态
    case PackageDependsStatusRole:
        return m_packagesManager->getPackageDependsStatus(currentRow).status;   //获取当前index包的依赖状态
    case PackageInstalledVersionRole:
        return m_packagesManager->packageInstalledVersion(currentRow);       //获取当前index包在系统中安装的版本
    case PackageAvailableDependsListRole:
        return m_packagesManager->packageAvailableDepends(currentRow);       //获取当前index包可用的依赖
    case PackageReverseDependsListRole:
        return m_packagesManager->packageReverseDependsList(packageName, architecture); //获取依赖于当前index包的应用
    case PackageShortDescriptionRole:
        return Utils::fromSpecialEncoding(shortDescription);        //获取当前index包的短描述
    case PackageLongDescriptionRole:
        return Utils::fromSpecialEncoding(longDescription);         //获取当前index包的长描述
    case PackageFailReasonRole:
        return packageFailedReason(currentRow);                              //获取当前index包的安装失败的原因
    case PackageOperateStatusRole: {
        auto md5 = m_packagesManager->getPackageMd5(currentRow);
        if (m_packageOperateStatus.contains(md5))                     //获取当前包的操作状态
            return m_packageOperateStatus[md5];
        else
            return Prepare;
    }
    case Qt::SizeHintRole:                                          //设置当前index的大小
        return QSize(0, 48);

    default:
        break;
    }

    return QVariant();
}

bool DebListModel::isDevelopMode()
{
    return m_isDevelopMode;
}

void DebListModel::selectedIndexRow(int row)
{
    m_packagesManager->selectedIndexRow(row);
}

void DebListModel::slotInstallPackages()
{
    if (m_workerStatus != WorkerPrepare)
        return;

    m_workerStatus = WorkerProcessing;                                  //刷新包安装器的工作状态
    m_operatingIndex = 0;                                               //初始化当前操作的index
    m_operatingStatusIndex = 0;
    m_operatingPackageMd5 = m_packageMd5[m_operatingIndex];

    // start first
    initRowStatus();                                                    //初始化包的操作状态

    //检查当前应用是否在黑名单中
    //非开发者模式且数字签名验证失败
    if (checkBlackListApplication() || !checkDigitalSignature())
        return;
    installNextDeb();                                                   //开始安装
}


void DebListModel::slotUninstallPackage(const int index)
{
    m_workerStatus = WorkerProcessing;                  //刷新当前包安装器的工作状态
    m_operatingIndex = index;                             //获取卸载的包的indx
    m_operatingPackageMd5 = m_packageMd5[m_operatingIndex];
    // fix bug : 卸载失败时不提示卸载失败。
    m_operatingStatusIndex = index;                       //刷新操作状态的index

    DebFile *debFile = new DebFile(m_packagesManager->package(m_operatingIndex));   //获取到包
    const QStringList rdepends = m_packagesManager->packageReverseDependsList(debFile->packageName(), debFile->architecture());     //检查是否有应用依赖到该包
    Backend *backend = m_packagesManager->m_backendFuture.result();
    for (const auto &r : rdepends) {                                        // 卸载所有依赖该包的应用（二者的依赖关系为depends）
        if (backend->package(r)){
            // 更换卸载包的方式，remove卸载不卸载完全会在影响下次安装的依赖判断。
            backend->package(r)->setPurge();
        }
        else
            qWarning() << "DebListModel:" << "reverse depend" << r << "error ,please check it!";
    }
    //卸载当前包 更换卸载包的方式，remove卸载不卸载完全会在影响下次安装的依赖判断。
    QApt::Package* uninstalledPackage = backend->package(debFile->packageName() + ':' + debFile->architecture());

    //未通过当前包的包名以及架构名称获取package对象，刷新操作状态为卸载失败
    if(!uninstalledPackage){
        refreshOperatingPackageStatus(Failed);
        delete debFile;
        return;
    }
    uninstalledPackage->setPurge();

    refreshOperatingPackageStatus(Operating);                                       //刷新当前index的操作状态
    Transaction *transsaction = backend->commitChanges();

    // trans 进度change 链接
    connect(transsaction, &Transaction::progressChanged, this, &DebListModel::signalTransactionProgressChanged);

    //详细状态信息（安装情况）展示链接
    connect(transsaction, &Transaction::statusDetailsChanged, this, &DebListModel::signalAppendOutputInfo);

    // trans 运行情况（授权是否成功）
    connect(transsaction, &Transaction::statusChanged, this, &DebListModel::slotTransactionStatusChanged);

    // trans运行中出现错误
    connect(transsaction, &Transaction::errorOccurred, this, &DebListModel::slotTransactionErrorOccurred);

    // 卸载结束，处理卸载成功与失败的情况并发送结束信号
    connect(transsaction, &Transaction::finished, this, &DebListModel::slotUninstallFinished);

    // 卸载结束之后 删除指针
    connect(transsaction, &Transaction::finished, transsaction, &Transaction::deleteLater);

    m_currentTransaction = transsaction;   //保存trans指针

    transsaction->run();                   //开始卸载
    delete debFile;
}

void DebListModel::removePackage(const int idx)
{
    if(WorkerPrepare != m_workerStatus){
        qWarning()<<"installer status error";
    }
    // 去除操作状态 中的index
    int packageOperateStatusCount = m_packageOperateStatus.size() - 1;
    m_packageOperateStatus.clear();
    for (int i = 0; i < packageOperateStatusCount; i++) {
        m_packageOperateStatus[m_packagesManager->getPackageMd5(i)] = DebListModel::Prepare;
    }
    m_packagesManager->removePackage(idx);       //在packageManager中删除标记的下标
}


void DebListModel::slotAppendPackage(QStringList package)
{
    if(WorkerPrepare != m_workerStatus){
        qWarning()<<"installer status error";
    }
    m_packagesManager->appendPackage(package);      //添加包，并返回添加结果
}

void DebListModel::slotTransactionStatusChanged(TransactionStatus transactionStatus)
{
    switch (transactionStatus) {
    case TransactionStatus::AuthenticationStatus:           //等待授权
        emit signalLockForAuth(true);                             //设置底层窗口按钮不可用
        break;
    case TransactionStatus::WaitingStatus:                  //当前操作在队列中等待操作
        emit signalLockForAuth(false);                            //设置底层窗口按钮可用
        break;
    default:
        break;
    }
}

void DebListModel::reset()
{
    m_workerStatus          = WorkerPrepare;                     //工作状态重置为准备态
    m_operatingIndex        = 0;                               //当前操作的index置为0
    m_operatingPackageMd5   = nullptr;
    m_operatingStatusIndex  = 0;                         //当前操作状态的index置为0

    m_packageOperateStatus.clear();                     //清空操作状态列表
    m_packageFailCode.clear();                          //清空错误原因列表
    m_packageFailReason.clear();
    m_packagesManager->reset();                         //重置packageManager
}

int DebListModel::getInstallFileSize()
{
    return m_packagesManager->m_preparedPackages.size();
}

void DebListModel::resetFileStatus()
{
    m_packageOperateStatus.clear();                     //重置包的操作状态
    m_packageFailReason.clear();                        //重置包的错误状态
    m_packageFailCode.clear();
}

void DebListModel::bumpInstallIndex()
{
    if (m_currentTransaction.isNull()) {
        qWarning() << "previous transaction not finished";
    }
    if (++m_operatingIndex == m_packagesManager->m_preparedPackages.size()) {
        m_workerStatus = WorkerFinished;                                        //设置包安装器的工作状态为Finish
        emit signalWorkerFinished();                                                  //发送安装完成信号
        emit signalWorkerProgressChanged(100);                                        //修改安装进度
        emit signalTransactionProgressChanged(100);
        return;
    }
    ++ m_operatingStatusIndex;
    m_operatingPackageMd5 = m_packageMd5[m_operatingIndex];
    emit signalChangeOperateIndex(m_operatingIndex);                                //修改当前操作的下标
    // install next
    qInfo() << "DebListModel:" << "install next deb package";

    //检查当前应用是否在黑名单中
    //非开发者模式且数字签名验证失败
    if (checkBlackListApplication() || !checkDigitalSignature())
        return;
    installNextDeb();                                                           //安装下一个包
}

void DebListModel::slotTransactionErrorOccurred()
{
    if(WorkerProcessing != m_workerStatus){
        qWarning()<<"installer status error";
    }
    Transaction *transaction = qobject_cast<Transaction *>(sender());
    if(!transaction)
        return;
    //失败时刷新操作状态为failed,并记录失败原因
    refreshOperatingPackageStatus(Failed);
    m_packageOperateStatus[m_operatingPackageMd5] = Failed;

    m_packageFailCode[m_operatingPackageMd5] = transaction->error();
    m_packageFailReason[m_operatingPackageMd5] = transaction->errorString();

    if (!transaction->errorString().contains("proper authorization was not provided"))
        emit signalAppendOutputInfo(transaction->errorString());

    const QApt::ErrorCode errorCode = transaction->error();               //trans错误的代码

    qWarning() << "DebListModel:" << "Transaction Error:" << errorCode << workerErrorString(errorCode, transaction->errorString());
    qWarning() << "DebListModel:" << "Error Infomation:" << transaction->errorDetails() << transaction->errorString();

    if (transaction->isCancellable()) transaction->cancel();

    //特殊处理授权错误
    if (AuthError == errorCode ) {
        transaction->deleteLater();                                                       //删除 trans指针
        QTimer::singleShot(100 * 1, this, &DebListModel::checkBoxStatus);           //检查授权弹窗的状态 如果弹窗仍然在只是超时，则底层窗口按钮不可用
        qWarning() << "DebListModel:" << "Authorization error";

        // reset env
        emit signalAuthCancel();                                                          //发送授权被取消的信号
        emit signalLockForAuth(false);                                                    //取消授权锁定，设置按钮可用
        emit signalEnableCloseButton(true);
        m_workerStatus = WorkerPrepare;                                             // 重置工作状态为准备态
        return;
    }

    // DO NOT install next, this action will finished and will be install next automatic.
    transaction->setProperty("exitStatus", QApt::ExitFailed);                             //设置trans的退出状态为 失败
}

void DebListModel::refreshOperatingPackageStatus(PackageOperationStatus operationStatus)
{
    m_packageOperateStatus[m_operatingPackageMd5] = operationStatus;  //将失败包的索引和状态修改保存,用于更新

    const QModelIndex modelIndex = index(m_operatingStatusIndex);

    emit dataChanged(modelIndex, modelIndex);                             //发送状态已经修改的信号
}

QString DebListModel::packageFailedReason(const int idx) const
{
    const auto dependStatus = m_packagesManager->getPackageDependsStatus(idx);                         //获取包的依赖状态
    const auto md5 = m_packagesManager->getPackageMd5(idx);                                 //获取包的md5值
    if (m_packagesManager->isArchError(idx)) return tr("Unmatched package architecture");   //判断是否架构冲突
    if(dependStatus.isProhibit())
        return tr("The administrator has set policies to prevent installation of this package");
    if (dependStatus.isBreak() || dependStatus.isAuthCancel()) {                                            //依赖状态错误
        if (!dependStatus.package.isEmpty()) {
            if (m_packagesManager->m_errorIndex.contains(md5))     //修改wine依赖的标记方式
                return tr("Failed to install %1").arg(m_brokenDepend); //wine依赖安装失败
            return tr("Broken dependencies: %1").arg(dependStatus.package);                         //依赖不满足
        }

        const auto conflictStatus = m_packagesManager->packageConflictStat(idx);                  //获取冲突情况
        if (!conflictStatus.is_ok())
            return tr("Broken dependencies: %1").arg(conflictStatus.unwrap()); //依赖冲突
        qInfo() << "================" << dependStatus.package;
    }

    if(m_packageOperateStatus.contains(md5) && m_packageOperateStatus[md5] == Failed)
        qWarning()<<"package operate status failed";
    //判断当前这个包是否错误
    if (!m_packageFailCode.contains(md5))
        qWarning() << "DebListModel:" << "failed to get reason" << m_packageFailCode.size() << idx;

    // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
    return workerErrorString(m_packageFailCode[md5], m_packageFailReason[md5]);             //根据错误代码和错误原因返回具体的错误原因
}

void DebListModel::slotTransactionFinished()
{
    if (m_workerStatus == WorkerProcessing) {
        qWarning() << "installer status error";
    }
    // 获取trans指针
    Transaction *transaction = qobject_cast<Transaction *>(sender());
    if(!transaction)
       return;
    // prevent next signal
    disconnect(transaction, &Transaction::finished, this, &DebListModel::slotTransactionFinished);  //不再接收trans结束的信号

    // report new progress
    //更新安装进度（批量安装进度控制）
    int progressValue = static_cast<int>(100. * (m_operatingIndex + 1) / m_packagesManager->m_preparedPackages.size());
    emit signalWorkerProgressChanged(progressValue);

    qInfo() << "DebListModel:" << "transaciont finished with exit status:" << transaction->exitStatus();
    if (transaction->exitStatus()) {
        //安装失败
        qWarning() << transaction->error() << transaction->errorDetails() << transaction->errorString();
        //保存错误原因和错误代码
        // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
        m_packageFailCode[m_operatingPackageMd5] = transaction->error();
        m_packageFailReason[m_operatingPackageMd5] = transaction->errorString();

        //刷新操作状态
        refreshOperatingPackageStatus(Failed);
        emit signalAppendOutputInfo(transaction->errorString());
    } else if (m_packageOperateStatus.contains(m_operatingPackageMd5) &&
               m_packageOperateStatus[m_operatingPackageMd5] != Failed) {
        //安装成功
        refreshOperatingPackageStatus(Success);

        //准备安装下一个包，修改下一个包的状态为正在安装状态
        if (m_operatingStatusIndex < m_packagesManager->m_preparedPackages.size() - 1) {
            auto md5 = m_packagesManager->getPackageMd5(m_operatingIndex + 1);
            m_packageOperateStatus[md5] = Waiting;
        }
    }
    //    delete trans;
    m_currentTransaction->deleteLater();
    m_currentTransaction = nullptr;
    transaction = nullptr;
    bumpInstallIndex();                 //进入安装进度控制
}


void DebListModel::slotDependsInstallTransactionFinished()//依赖安装关系满足
{
    if (m_workerStatus == WorkerProcessing) {
        qWarning() << "installer status error";
    }
    Transaction *transaction = qobject_cast<Transaction *>(sender());
    if(!transaction)
       return;

    const auto transExitStatus = transaction->exitStatus();

    if (transExitStatus) qWarning() << transaction->error() << transaction->errorDetails() << transaction->errorString();     //transaction发生错误

    if (transExitStatus) {
        // record error
        // 记录错误原因和错误代码
        // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
        m_packageFailCode[m_operatingPackageMd5] = transaction->error();
        m_packageFailReason[m_operatingPackageMd5] = transaction->errorString();
        refreshOperatingPackageStatus(Failed);                                  // 刷新操作状态
        emit signalAppendOutputInfo(transaction->errorString());
    }

    //    delete trans;
    m_currentTransaction->deleteLater();
    m_currentTransaction = nullptr;
    transaction = nullptr;

    // check current operate exit status to install or install next
    if (transExitStatus) {
        bumpInstallIndex();                                                     //依赖安装失败，直接安装下一个包
    } else {
        //检查当前应用是否在黑名单中
        //非开发者模式且数字签名验证失败
        if (checkBlackListApplication() || !checkDigitalSignature())
            return;
        installNextDeb();                                                       //依赖安装成功，开始安装这个包
    }
}

void DebListModel::setEndEnable()
{
    emit signalEnableReCancelBtn(true);
}


void DebListModel::checkBoxStatus()
{
    QTime startTime = QTime::currentTime();                                      //获取弹出的时间
    Transaction *transation = nullptr;
    auto *const backend = m_packagesManager->m_backendFuture.result();
    transation = backend->commitChanges();

    QTime stopTime = QTime::currentTime();
    int elapsed = startTime.msecsTo(stopTime);                                   //获取commit授权被取消的时间
    if (elapsed > 20000) {                                                      //如果时间超过20ms则不断判断当前窗口是否超时
        QTimer::singleShot(100 * 1, this, &DebListModel::checkBoxStatus);
        return;
    }

    if (transation) {
        if (transation->isCancellable()) {                                           //当前transaction已经被取消
            transation->cancel();
            QTimer::singleShot(100 * 1, this, &DebListModel::setEndEnable);     //设置按钮可用
        } else {
            QTimer::singleShot(100 * 1, this, &DebListModel::checkBoxStatus);   //当前Transaction还在运行中，继续等待并判断
        }
    } else {
        qWarning() << "DebListModel:" << "Transaction is Nullptr";
    }
}

void DebListModel::installDebs()
{
    DebFile deb(m_packagesManager->package(m_operatingIndex)) ;
    
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");
    Q_ASSERT_X(m_currentTransaction.isNull(), Q_FUNC_INFO, "previous transaction not finished");
    //在判断dpkg启动之前就发送开始安装的信号，并在安装信息中输出 dpkg正在运行的信息。
    emit signalStartInstall();

    // fetch next deb
    auto *const backend = m_packagesManager->m_backendFuture.result();

    Transaction *transaction = nullptr;

    // reset package depends status
    m_packagesManager->resetPackageDependsStatus(m_operatingStatusIndex);

    // check available dependencies
    const auto dependsStat = m_packagesManager->getPackageDependsStatus(m_operatingStatusIndex);
    if (dependsStat.isBreak() || dependsStat.isAuthCancel() || dependsStat.status == DebListModel::ArchBreak) {          //依赖不满足或者下载wine依赖时授权被取消
        refreshOperatingPackageStatus(Failed);                          //刷新错误状态

        // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
        m_packageFailCode.insert(m_operatingPackageMd5, -1);           //保存错误原因
        // 记录详细错误原因
        m_packageFailReason.insert(m_operatingPackageMd5, packageFailedReason(m_operatingStatusIndex));

        bumpInstallIndex();                                             //开始下一步的安装流程
        return;
    } else if (dependsStat.isAvailable()) {
        if (isDpkgRunning()) {
            qInfo() << "DebListModel:"
                    << "dpkg running, waitting...";
            // 缩短检查的时间，每隔1S检查当前dpkg是否正在运行。
            QTimer::singleShot(1000 * 1, this, &DebListModel::installNextDeb);
            emit signalAppendOutputInfo("dpkg running, waitting..."); //发送提示，告知用户dpkg正在运行
            return;
        }
        // 依赖可用 但是需要下载
        Q_ASSERT_X(m_packageOperateStatus[m_operatingPackageMd5], Q_FUNC_INFO,
                   "package operate status error when start install availble dependencies");

        // 获取到所有的依赖包 准备安装
        const QStringList availableDepends = m_packagesManager->packageAvailableDepends(m_operatingIndex);
        //获取到可用的依赖包并根据后端返回的结果判断依赖包的安装结果
        for (auto const &p : availableDepends) {
            if (p.contains(" not found")) {                             //依赖安装失败
                refreshOperatingPackageStatus(Failed);                  //刷新当前包的状态
                // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
                m_packageFailCode.insert(m_operatingPackageMd5, DownloadDisallowedError);                       //记录错误代码与错误原因
                m_packageFailReason.insert(m_operatingPackageMd5, p);
                emit signalAppendOutputInfo(m_packagesManager->package(m_operatingIndex) + "\'s depend " + " " + p);  //输出错误原因
                bumpInstallIndex();                                     //开始安装下一个包或结束安装
                return;
            }
            backend->markPackageForInstall(p);                          //开始安装依赖包
        }

        transaction = backend->commitChanges();
        if (!transaction)
            return;
        //依赖安装结果处理
        connect(transaction, &Transaction::finished, this, &DebListModel::slotDependsInstallTransactionFinished);
    } else {
        if (isDpkgRunning()) {
            qInfo() << "DebListModel:"
                    << "dpkg running, waitting...";
            // 缩短检查的时间，每隔1S检查当前dpkg是否正在运行。
            QTimer::singleShot(1000 * 1, this, &DebListModel::installNextDeb);
            emit signalAppendOutputInfo("dpkg running, waitting..."); //发送提示，告知用户dpkg正在运行
            return;
        }
        transaction = backend->installFile(deb);//触发Qapt授权框和安装线程
        if (!transaction)
            return;
        // 进度变化和结束过程处理
        connect(transaction, &Transaction::progressChanged, this, &DebListModel::signalTransactionProgressChanged);
        connect(transaction, &Transaction::finished, this, &DebListModel::slotTransactionFinished);
    }

    // NOTE: DO NOT remove this.
    transaction->setLocale(".UTF-8");

    //记录日志
    connect(transaction, &Transaction::statusDetailsChanged, this, &DebListModel::signalAppendOutputInfo);

    //刷新操作状态
    connect(transaction, &Transaction::statusDetailsChanged, this, &DebListModel::slotTransactionOutput);

    //授权处理
    connect(transaction, &Transaction::statusChanged, this, &DebListModel::slotTransactionStatusChanged);

    //错误处理
    connect(transaction, &Transaction::errorOccurred, this, &DebListModel::slotTransactionErrorOccurred);

    m_currentTransaction = transaction;

    m_currentTransaction->run();
}

void DebListModel::digitalVerifyFailed(ErrorCode errorCode)
{
    if (preparedPackages().size() > 1) {                        //批量安装
        refreshOperatingPackageStatus(Failed);                  //刷新操作状态
        // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
        m_packageFailCode.insert(m_operatingPackageMd5, errorCode); //记录错误代码与错误原因
        m_packageFailReason.insert(m_operatingPackageMd5, "");
        bumpInstallIndex();                                     //跳过当前包
    } else if (preparedPackages().size() == 1) {
        if (!m_isDevelopMode) {
            exit(0);
        } else { //开发者模式下，点击取消按钮，返回错误界面
            refreshOperatingPackageStatus(Failed);                  //刷新操作状态
            // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
            m_packageFailCode.insert(m_operatingPackageMd5, errorCode); //记录错误代码与错误原因
            m_packageFailReason.insert(m_operatingPackageMd5, "");
            emit signalWorkerFinished();
        }
    }
}

void DebListModel::showNoDigitalErrWindow()
{
    //批量安装时，如果不是最后一个包，则不弹窗，只记录详细错误原因。
    if (m_operatingIndex < m_packagesManager->m_preparedPackages.size() - 1) {
        digitalVerifyFailed(NoDigitalSignature);//刷新安装错误，并记录错误原因
        return;
    }
    DDialog *Ddialog = new DDialog();                   //弹出窗口
    Ddialog->setModal(true);
    Ddialog->setWindowFlag(Qt::WindowStaysOnTopHint);   //窗口一直置顶
    Ddialog->setTitle(tr("Unable to install - no digital signature"));
    Ddialog->setMessage(QString(tr("Please go to Control Center to enable developer mode and try again. Proceed?")));
    Ddialog->setIcon(QIcon::fromTheme("di_popwarning"));

    Ddialog->addButton(QString(tr("Cancel", "button")), true, DDialog::ButtonNormal);     //添加取消按钮
    Ddialog->addButton(QString(tr("Proceed", "button")), true, DDialog::ButtonRecommend);  //添加前往按钮
    Ddialog->show();    //显示弹窗

    //取消按钮
    QPushButton *btnCancel = qobject_cast<QPushButton *>(Ddialog->getButton(0));
    connect(btnCancel, &DPushButton::clicked, this, &DebListModel::slotNoDigitalSignature);
    connect(btnCancel, &DPushButton::clicked, Ddialog, &DDialog::deleteLater);

    //前往按钮1
    QPushButton *btnProceedControlCenter = qobject_cast<QPushButton *>(Ddialog->getButton(1));
    connect(btnProceedControlCenter, &DPushButton::clicked, this, &DebListModel::slotShowDevelopModeWindow);
    connect(btnProceedControlCenter, &DPushButton::clicked, this, &QApplication::exit);
    connect(btnProceedControlCenter, &DPushButton::clicked, Ddialog, &DDialog::deleteLater);

    //关闭图标
    connect(Ddialog, &DDialog::aboutToClose, this, &DebListModel::slotNoDigitalSignature);
    connect(Ddialog, &DDialog::aboutToClose, Ddialog, &DDialog::deleteLater);
}


void DebListModel::showDigitalErrWindow()
{
    //批量安装时，如果不是最后一个包，则不弹窗，只记录详细错误原因。
    if (m_operatingIndex < m_packagesManager->m_preparedPackages.size() - 1) {
        digitalVerifyFailed(DigitalSignatureError);     //刷新安装错误，并记录错误原因
        return;
    }
    DDialog *Ddialog = new DDialog();
    //设置窗口焦点
    Ddialog->setFocusPolicy(Qt::TabFocus);

    //设置弹出窗口为模态窗口
    Ddialog->setModal(true);

    //设置窗口始终置顶
    Ddialog->setWindowFlag(Qt::WindowStaysOnTopHint);

    // 设置弹出窗口显示的信息
    Ddialog->setTitle(tr("Unable to install"));
    Ddialog->setMessage(QString(tr("This package does not have a valid digital signature")));
    Ddialog->setIcon(QIcon::fromTheme("di_popwarning"));
    Ddialog->addButton(QString(tr("OK", "button")), true, DDialog::ButtonNormal);
    Ddialog->show();
    QPushButton *btnOK = qobject_cast<QPushButton *>(Ddialog->getButton(0));
    btnOK->setFocusPolicy(Qt::TabFocus);
    btnOK->setFocus();
    // 点击弹出窗口的关闭图标按钮
    connect(Ddialog, &DDialog::aboutToClose, this, &DebListModel::slotDigitalSignatureError);
    connect(Ddialog, &DDialog::aboutToClose, Ddialog, &DDialog::deleteLater);

    //点击弹出窗口的确定按钮
    connect(btnOK, &DPushButton::clicked, this, &DebListModel::slotDigitalSignatureError);
    connect(btnOK, &DPushButton::clicked, Ddialog, &DDialog::deleteLater);
}

void DebListModel::showDevelopDigitalErrWindow(ErrorCode code)
{
    Dialog *Ddialog = new Dialog();
    //设置窗口焦点
    //fix bug:https://pms.uniontech.com/zentao/bug-view-44837.html
    Ddialog->setFocusPolicy(Qt::TabFocus);

    //设置弹出窗口为模态窗口
    Ddialog->setModal(true);

    //设置窗口始终置顶
    Ddialog->setWindowFlag(Qt::WindowStaysOnTopHint);

    // 设置弹出窗口显示的信息
    Ddialog->setMessage(QString(tr("This package does not have a valid digital signature. Continue with the installation?")));
    Ddialog->setIcon(QIcon::fromTheme("di_popwarning"));
    Ddialog->addButton(QString(tr("Cancel")), true, DDialog::ButtonNormal);
    Ddialog->addButton(QString(tr("Continue", "button")), true, DDialog::ButtonRecommend); //添加前往按钮

    Ddialog->show();
    QPushButton *cancelBtn = qobject_cast<QPushButton *>(Ddialog->getButton(0));

    cancelBtn->setFocusPolicy(Qt::TabFocus);
    cancelBtn->setFocus();

    bool continueBtnClicked = false;

    // 点击弹出窗口的关闭图标按钮
    connect(Ddialog, &DDialog::aboutToClose, this, [=] {
        //刷新当前包的操作状态，失败原因为数字签名校验失败
        digitalVerifyFailed(code);
    });
    connect(Ddialog, &DDialog::aboutToClose, Ddialog, &DDialog::deleteLater);

    //点击弹出窗口的确定按钮
    connect(cancelBtn, &DPushButton::clicked, this, [=] {
        digitalVerifyFailed(code);
    });
    connect(cancelBtn, &DPushButton::clicked, Ddialog, &DDialog::deleteLater);

    QPushButton *continueBtn = qobject_cast<QPushButton *>(Ddialog->getButton(1));
    connect(continueBtn, &DPushButton::clicked, this, [&] {
        continueBtnClicked = true;
        installNextDeb();
    }); //点击继续，进入安装流程
    connect(continueBtn, &DPushButton::clicked, Ddialog, &DDialog::deleteLater);
    connect(Ddialog, &Dialog::signalClosed, this, [=] { digitalVerifyFailed(code); });
    connect(Ddialog, &Dialog::signalClosed, Ddialog, &DDialog::deleteLater);
}

void DebListModel::slotDigitalSignatureError()
{
    digitalVerifyFailed(DigitalSignatureError);
}

void DebListModel::slotNoDigitalSignature()
{
    digitalVerifyFailed(NoDigitalSignature);
}

void DebListModel::slotShowDevelopModeWindow()
{
    //弹出设置 通用窗口
    QString command = " dbus-send --print-reply "
                      "--dest=com.deepin.dde.ControlCenter "
                      "/com/deepin/dde/ControlCenter "
                      "com.deepin.dde.ControlCenter.ShowModule "
                      "\"string:commoninfo\"";
    QProcess *unlock = new QProcess(this);

    connect(unlock, static_cast<void (QProcess::*)(int)>(&QProcess::finished), unlock, &QProcess::deleteLater);

    unlock->startDetached(command);
    unlock->waitForFinished();
    return;
}

void DebListModel::checkSystemVersion()
{
    // add for judge OS Version
    // 修改获取系统版本的方式 此前为  DSysInfo::deepinType()
    switch (Dtk::Core::DSysInfo::uosEditionType()) {            //获取系统的类型
#if(DTK_VERSION > DTK_VERSION_CHECK(5,4,10,0))
    case Dtk::Core::DSysInfo::UosEducation:                     //教育版
#endif
    case Dtk::Core::DSysInfo::UosProfessional: //专业版
    case Dtk::Core::DSysInfo::UosHome: {                     //个人版
        QDBusInterface *dbusInterFace = new QDBusInterface("com.deepin.deepinid", "/com/deepin/deepinid", "com.deepin.deepinid");
        bool deviceMode = dbusInterFace->property("DeviceUnlocked").toBool();                            // 判断当前是否处于开发者模式
        qInfo() << "DebListModel:" << "system editon:" << Dtk::Core::DSysInfo::uosEditionName() << "develop mode:" << deviceMode;
        m_isDevelopMode = deviceMode;
        delete dbusInterFace;
        break;
    }
    case Dtk::Core::DSysInfo::UosCommunity: //社区版 不验证签名
    case Dtk::Core::DSysInfo::UosEnterprise:                 //服务器版
        m_isDevelopMode = true;
        break;
    default:
        m_isDevelopMode =  true;
        break;
    }
}

bool DebListModel::checkDigitalSignature()
{
    const auto stat = m_packagesManager->getPackageDependsStatus(m_operatingIndex); //获取包的依赖状态
    if (stat.isBreak() || stat.isAuthCancel())
        return true;
    SettingDialog dialog;
    m_isDigitalVerify = dialog.isDigitalVerified();
    int digitalSigntual = Utils::Digital_Verify(m_packagesManager->package(m_operatingIndex)); //判断是否有数字签名
    qInfo() << "m_isDevelopMode:" << m_isDevelopMode << " /m_isDigitalVerify:" << m_isDigitalVerify << " /digitalSigntual:" << digitalSigntual;
    if (m_isDevelopMode && !m_isDigitalVerify) { //开发者模式且未设置验签功能
        return true;
    } else if (m_isDevelopMode && m_isDigitalVerify) { //开发者模式且设置验签功能
        if (digitalSigntual == Utils::VerifySuccess) {
            return true;
        } else {
            ErrorCode code;
            if (digitalSigntual == Utils::DebfileInexistence)
                code = NoDigitalSignature;
            else
                code = DigitalSignatureError;
            showDevelopDigitalErrWindow(code); //弹出提示框
            return false;
        }
    } else { //非开发者模式
        bool verifiedResult = false;
        switch (digitalSigntual) {
        case Utils::VerifySuccess: //签名验证成功
            verifiedResult = true;
            break;
        case Utils::DebfileInexistence: //无签名文件
            showNoDigitalErrWindow();
            verifiedResult = false;
            break;
        case Utils::ExtractDebFail: //无有效的数字签名
            showDigitalErrWindow();
            verifiedResult = false;
            break;
        case Utils::DebVerifyFail:
        case Utils::OtherError: //其他原因造成的签名校验失败
            showDigitalErrWindow();
            verifiedResult = false;
            break;
        default: //其他未知错误
            qInfo() << "unknown mistake";
            verifiedResult = false;
            break;
        }
        return verifiedResult;
    }
}

void DebListModel::installNextDeb()
{
        QString sPackageName = m_packagesManager->m_preparedPackages[m_operatingIndex];
        QStringList strFilePath;
        if (checkTemplate(sPackageName)) {                      //检查当前包是否需要配置
            rmdir();                                            //删除临时路径
            m_procInstallConfig->start("pkexec", QStringList() << "deepin-deb-installer-dependsInstall" << "InstallConfig" << sPackageName);
        } else {
            installDebs();                                      //普通安装
        }
}

void DebListModel::rmdir()
{
    QDir filePath(tempPath);
    if (filePath.exists()) {
        if (!filePath.removeRecursively()) {
            qWarning() << "DebListModel:" << "remove temporary path failed";
        } 
    }
}

bool DebListModel::checkTemplate(QString debPath)
{
    rmdir();
    getDebian(debPath);
    QFile templates(tempPath + "/templates");
    if (templates.exists()) {
        qInfo() << "DebListModel:" << "Check that the template file exists";
        return true;
    }
    return false;
}

bool DebListModel::mkdir()
{
    QDir filePath(tempPath);                //获取配置包的临时路径

    if (!filePath.exists()) {               //当前临时路径不存在
        return filePath.mkdir(tempPath);    //删除临时路径，并返回删除结果
    }
    return true;
}

void DebListModel::getDebian(QString debPath)
{
    if (!mkdir()) {                                                             //创建临时路径
        qWarning() << "check error mkdir" << tempPath << "failed";              //创建失败
        return;
    }
    QProcess *m_pDpkg = new QProcess(this);
    m_pDpkg->start("dpkg", QStringList() << "-e" << debPath << tempPath);       //获取DEBIAN文件，查看当前包是否需要配置
    m_pDpkg->waitForFinished();
    QString getDebianProcessErrInfo = m_pDpkg->readAllStandardError();
    if (!getDebianProcessErrInfo.isEmpty()) {
        qWarning() << "DebListModel:" << "Failed to decompress the main control file" << getDebianProcessErrInfo;
    }
    delete m_pDpkg;
}

void DebListModel::slotTransactionOutput()
{
    if (m_workerStatus == WorkerProcessing) {
        qInfo() << "installer status error";
    }
    Transaction *trans = qobject_cast<Transaction *>(sender());
    if (!trans)
        return;

    refreshOperatingPackageStatus(Operating);                       //刷新当前包的操作状态

    disconnect(trans, &Transaction::statusDetailsChanged, this, &DebListModel::slotTransactionOutput);

}

void DebListModel::slotUninstallFinished()
{
    if (m_workerStatus == WorkerProcessing) {
        qInfo() << "installer status error";
    }

    //增加卸载失败的情况
    //此前的做法是发出commitError的信号，现在全部在Finished中进行处理。不再特殊处理。
    Transaction *trans = qobject_cast<Transaction *>(sender());
    if (!trans)
        return;

    if (trans->exitStatus()) {
        m_workerStatus = WorkerFinished;                            //刷新包安装器的工作状态
        refreshOperatingPackageStatus(Failed);                      //刷新当前包的操作状态
        m_packageOperateStatus[m_operatingPackageMd5] = Failed;
        qWarning() << "DebListModel:" << "uninstall finished with finished code:" << trans->error() << "finished details:" << trans->errorString();
    } else {
        m_workerStatus = WorkerFinished;                            //刷新包安装器的工作状态
        refreshOperatingPackageStatus(Success);                     //刷新当前包的卸载状态
        m_packageOperateStatus[m_operatingPackageMd5] = Success;
    }
    emit signalWorkerFinished();                                          //发送结束信号（只有单包卸载）卸载结束就是整个流程的结束
    trans->deleteLater();                                 
}

void DebListModel::slotSetCurrentIndex(const QModelIndex &modelIndex)
{
    if (m_currentIdx == modelIndex) return;                //要修改的index与当前index 一致

    const QModelIndex index = m_currentIdx;         //保存当前的index
    m_currentIdx = modelIndex;                             //修改当前的index

    emit dataChanged(index, index);
    emit dataChanged(m_currentIdx, m_currentIdx);   //发送index修改信号
}

void DebListModel::initPrepareStatus()
{
    for (int i = 0; i < m_packagesManager->m_preparedPackages.size(); i++) {
        auto md5 = m_packagesManager->getPackageMd5(i);
        m_packageOperateStatus.insert(md5, Prepare);                          //刷新当前所有包的状态为Prepare
    }
}

void DebListModel::initRowStatus()
{
    // 更换状态存储方式后修改更新状态的方式
    for (auto md5 : m_packageMd5) {
        m_packageOperateStatus[md5] = Waiting;
    }
}

void DebListModel::slotUpWrongStatusRow()
{
    if (m_packagesManager->m_preparedPackages.size() == 1)
        return;

    QList<QByteArray> installErrorPackages;         //安装错误的包的list
    QList<QByteArray> installSuccessPackages;       //安装成功的包的list

    //根据包的操作状态，分别找到所有安装成功的包与安装失败的包
    QMapIterator<QByteArray, int> iteratorpackageOperateStatus(m_packageOperateStatus);
    while (iteratorpackageOperateStatus.hasNext()) {
        iteratorpackageOperateStatus.next();
        // 保存安装成功的包
        if (iteratorpackageOperateStatus.value() == Failed || iteratorpackageOperateStatus.value() == VerifyFailed) {   //安装失败或签名验证失败
            installErrorPackages.append(iteratorpackageOperateStatus.key());                                          //保存下标
        }
        // 保存安装失败的包
        if (iteratorpackageOperateStatus.value() == Success) {
            installSuccessPackages.append(iteratorpackageOperateStatus.key());
        }
    }
    if (installErrorPackages.size() == 0)       //全部安装成功 直接退出
        return;

    // 先将包与md5 绑定
    // 后续要对根据MD5对包的路径进行排序，保证包名和md5的下标统一
    QMap<QByteArray, QString> md5Packages;
    for (int i = 0; i < m_packagesManager->m_packageMd5.size(); i++) {
        md5Packages.insert(m_packagesManager->m_packageMd5[i], m_packagesManager->m_preparedPackages[i]);
    }

    m_packagesManager->m_packageMd5.clear();
    m_packagesManager->m_packageMd5.append(installErrorPackages);
    m_packagesManager->m_packageMd5.append(installSuccessPackages);

    m_packagesManager->m_preparedPackages.clear();
    for (int i = 0; i < m_packagesManager->m_packageMd5.size(); i++) {
        m_packagesManager->m_preparedPackages.append(md5Packages[m_packagesManager->m_packageMd5[i]]);
    }

    //update view
    const QModelIndex idxStart = index(0);
    const QModelIndex idxEnd = index(m_packageOperateStatus.size() - 1);
    emit dataChanged(idxStart, idxEnd);

    //update scroll
    emit signalChangeOperateIndex(-1);
}

void DebListModel::slotConfigInstallFinish(int installResult)
{
    if (m_packagesManager->m_preparedPackages.size() == 0)
        return;
    int progressValue = static_cast<int>(100. * (m_operatingIndex + 1) / m_packagesManager->m_preparedPackages.size()); //批量安装时对进度进行处理
    emit signalWorkerProgressChanged(progressValue);
    if (0 == installResult) {        //安装成功
        if (m_packagesManager->m_packageMd5DependsStatus[m_packagesManager->m_packageMd5[m_operatingIndex]].status == DependsOk) {
            refreshOperatingPackageStatus(Success);                 //刷新安装状态
        }
        bumpInstallIndex();                                         //开始安装下一个
    } else {
        if (1 == m_packagesManager->m_preparedPackages.size()) {    //单包安装
            refreshOperatingPackageStatus(Prepare);                 //刷新当前包的操作状态为准备态
            m_workerStatus = WorkerPrepare;
            emit signalAuthCancel();                                      //授权取消
        } else {
            //批量安装
            refreshOperatingPackageStatus(Failed);                  //刷新当前包的状态为失败

            // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
            m_packageFailCode.insert(m_operatingPackageMd5, installResult);       //保存失败原因
            m_packageFailReason.insert(m_operatingPackageMd5, "Authentication failed");
            bumpInstallIndex();                                     //开始安装下一个
        }
    }
    configWindow->hide();                        //隐藏配置窗口
    configWindow->clearTexts();                  //清楚配置信息
    m_procInstallConfig->terminate();                               //结束配置
    m_procInstallConfig->close();
}

void DebListModel::slotConfigReadOutput()
{
    QString tmp = m_procInstallConfig->readAllStandardOutput().data();                  //获取配置读取到的信息

    //检查命令返回的结果，如果是 没有发现命令。直接报错，安装失败
    slotCheckInstallStatus(tmp);

    tmp.remove(QChar('"'), Qt::CaseInsensitive);
    tmp.remove(QChar('\n'), Qt::CaseInsensitive);

    if (tmp.contains("StartInstallAptConfig")) {                                        //获取到当前正在安装配置
        emit signalStartInstall();
        refreshOperatingPackageStatus(Operating);                                       //刷新当前的操作状态
        configWindow->show();                                        //显示配置窗口
        QString startFlagStr = "StartInstallAptConfig";
        int num = tmp.indexOf(startFlagStr) + startFlagStr.size();
        int iCutoutNum = tmp.size() - num;
        if (iCutoutNum > 0)
            configWindow->appendTextEdit(tmp.mid(num, iCutoutNum));  //显示配置信息
        return;
    }

    QString appendInfoStr = tmp;
    appendInfoStr.remove(QChar('\"'), Qt::CaseInsensitive);
    appendInfoStr.remove(QChar('"'), Qt::CaseInsensitive);
    appendInfoStr.replace("\\n", "\n");
    appendInfoStr.replace("\n\n", "\n");
    emit signalAppendOutputInfo(appendInfoStr);                                               //将信息同步显示到安装信息中
    if (tmp.contains("Not authorized")) {
        configWindow->close();                                       //没有授权，关闭配置窗口
    } else {
        configWindow->appendTextEdit(tmp);                           //授权成功，继续配置
    }
}

void DebListModel::slotConfigInputWrite(QString str)
{
    m_procInstallConfig->write(str.toUtf8());                                          //将用户输入的配置项写入到配置安装进程中。
    m_procInstallConfig->write("\n");                                                  //写入换行，配置生效
}

void DebListModel::slotCheckInstallStatus(QString installInfo)
{
    // 判断当前的信息是否是错误提示信息
    if (installInfo.contains("Cannot run program deepin-deb-installer-dependsInstall: No such file or directory")) {
        emit signalAppendOutputInfo(installInfo);                                 //输出安装错误的原因
        m_workerStatus = WorkerFinished;                            //刷新包安装器的工作状态

        refreshOperatingPackageStatus(Failed);                      //刷新当前包的操作状态

        // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
        m_packageOperateStatus[m_operatingPackageMd5] = Failed;
        m_packageFailCode.insert(m_operatingPackageMd5, 0);       //保存失败原因
        m_packageFailReason.insert(m_operatingPackageMd5, "");
        bumpInstallIndex();
        return;
    }
}

bool DebListModel::recheckPackagePath(QString packagePath) const
{
    QFile packagePathFile(packagePath);
    do {
        if (packagePathFile.readLink().isEmpty()) {
            if (packagePathFile.exists()) {
                return true;
            }
        } else {
            QFile realPath(packagePathFile.readLink());
            if (realPath.exists() && packagePathFile.exists()) {
                return true;
            }
        }
    } while (false);
    QFileInfo fileInfo(packagePath);
    qWarning() << "check file path" << packagePath << "source file and link file not exist ";
    emit signalPackageCannotFind(fileInfo.fileName());
    return false;
}

void DebListModel::getPackageMd5(const QList<QByteArray> &packagesMD5)
{
    m_packageMd5.clear();
    m_packageMd5 = packagesMD5;
    emit signalAppendFinished();
}

void DebListModel::slotShowProhibitWindow()
{
    digitalVerifyFailed(ApplocationProhibit);
}
void DebListModel::showProhibitWindow()
{
    //批量安装时，如果不是最后一个包，则不弹窗，只记录详细错误原因。
    if (m_operatingIndex < m_packagesManager->m_preparedPackages.size() - 1) {
        digitalVerifyFailed(ApplocationProhibit);     //刷新安装错误，并记录错误原因
        return;
    }
    DDialog *Ddialog = new DDialog();
    //设置窗口焦点
    Ddialog->setFocusPolicy(Qt::TabFocus);

    //设置弹出窗口为模态窗口
    Ddialog->setModal(true);

    //设置窗口始终置顶
    Ddialog->setWindowFlag(Qt::WindowStaysOnTopHint);

    // 设置弹出窗口显示的信息
    Ddialog->setTitle(tr("Unable to install"));
    Ddialog->setMessage(QString(tr("The administrator has set policies to prevent installation of this package")));
    Ddialog->setIcon(QIcon::fromTheme("di_popwarning"));
    Ddialog->addButton(QString(tr("OK", "button")), true, DDialog::ButtonNormal);
    Ddialog->show();
    QPushButton *btnOK = qobject_cast<QPushButton *>(Ddialog->getButton(0));


    btnOK->setFocusPolicy(Qt::TabFocus);
    btnOK->setFocus();
    // 点击弹出窗口的关闭图标按钮
    connect(Ddialog, &DDialog::aboutToClose, this, &DebListModel::slotShowProhibitWindow);
    connect(Ddialog, &DDialog::aboutToClose, Ddialog, &DDialog::deleteLater);

    //点击弹出窗口的确定按钮
    connect(btnOK, &DPushButton::clicked, this, &DebListModel::slotShowProhibitWindow);
    connect(btnOK, &DPushButton::clicked, Ddialog, &DDialog::deleteLater);
}

bool DebListModel::checkBlackListApplication()
{
    PackageDependsStatus dependsStat = m_packagesManager->getPackageDependsStatus(m_operatingIndex);
    if (dependsStat.isProhibit()) {
        showProhibitWindow();
        return true;
    }
    return false;
}

int DebListModel::getWorkerStatus()
{
    return m_workerStatus;
}

void DebListModel::setWorkerStatus(int workerStatus)
{
    m_workerStatus = workerStatus;
}

DebListModel::~DebListModel()
{
    delete m_packagesManager;
    delete configWindow;
    delete m_procInstallConfig;
}

Dialog::Dialog()
{
}

void Dialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        emit signalClosed();
}
