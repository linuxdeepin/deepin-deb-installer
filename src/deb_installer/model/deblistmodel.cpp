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

#include "deblistmodel.h"
#include "manager/packagesmanager.h"
#include "manager/PackageDependsStatus.h"
#include "view/pages/AptConfigMessage.h"
#include "utils/utils.h"
#include "utils/DebugTimeManager.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFuture>
#include <QFutureWatcher>
#include <QSize>
#include <QtConcurrent>

#include <DDialog>
#include <DSysInfo>

#include <QApt/Backend>
#include <QApt/Package>

using namespace QApt;

/**
 * @brief isDpkgRunning 判断当前dpkg 是否在运行
 * @return
 */
bool isDpkgRunning()
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
    if (processOutput.contains("dpkg")) return true;   //更换判断的方式

    return false;
}

/**
 * @brief netErrors
 * @return the List of The Error infomations.
 * 无网络安装依赖时，库返回错误为FetechError 偶尔为CommitError
 * 此函数处理库返回CommitError时，网络错误的各种情况，如果错误信息中包含此列表中的信息，则判断为网络原因。
 */
const QStringList netErrors()
{
    QStringList errorDetails;
    errorDetails << "Address family for hostname not supported";
    errorDetails << "Temporary failure resolving";
    errorDetails << "Network is unreachable";
    errorDetails << "Cannot initiate the connection to";
    return errorDetails;
}

/**
 * @brief workerErrorString 根据安装失败的代码显示安装失败的原因。
 * @param errorCode 库或自定义的安装失败代码
 * @param errorInfo 库返回的或自定义的安装错误的原因
 * @return 要显示的安装失败的原因
 */
const QString workerErrorString(const int errorCode, const QString errorInfo)
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
    }
    //其余错误，暂不提示具体的错误原因
    return QApplication::translate("DebListModel", "Installation Failed");
}

DebListModel::DebListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_workerStatus(WorkerPrepare)
    , m_packagesManager(new PackagesManager(this))
{

    connect(this, &DebListModel::workerFinished, this, &DebListModel::upWrongStatusRow);                //安装成功后，将安装失败的包上滚


    // 配置包安装的进程
    m_procInstallConfig = new QProcess;
    m_procInstallConfig->setProcessChannelMode(QProcess::MergedChannels);               //获取子进程所有的输出数据
    m_procInstallConfig->setReadChannel(QProcess::StandardOutput);                      //QProcess 当前从标准输出中读取所有的数据

    configWindow = new AptConfigMessage;

    // 配置安装结束
    connect(m_procInstallConfig, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this, &DebListModel::ConfigInstallFinish);

    // 配置安装的过程数据
    connect(m_procInstallConfig, &QProcess::readyReadStandardOutput, this, &DebListModel::ConfigReadOutput);

    // 向安装进程中写入配置信息（一般是配置的序号）
    connect(configWindow, &AptConfigMessage::AptConfigInputStr, this, &DebListModel::ConfigInputWrite);

    //处理wine依赖安装的过程
    connect(m_packagesManager, &PackagesManager::DependResult, this, &DebListModel::DealDependResult);

    //安装wine依赖的时候不允许程序退出
    connect(m_packagesManager, &PackagesManager::enableCloseButton, this, &DebListModel::enableCloseButton);

    //添加的包是无效的包
    connect(m_packagesManager, &PackagesManager::invalidPackage, this, &DebListModel::invalidPackage);

    //添加的包不在本地
    connect(m_packagesManager, &PackagesManager::notLocalPackage, this, &DebListModel::notLocalPackage);

    //要添加的包早就已经被添加到程序中
    connect(m_packagesManager, &PackagesManager::packageAlreadyExists, this, &DebListModel::packageAlreadyExists);

    //刷新单包安装界面
    connect(m_packagesManager, &PackagesManager::refreshSinglePage, this, &DebListModel::refreshSinglePage);

    //刷新批量安装界面的model
    connect(m_packagesManager, &PackagesManager::refreshMultiPage, this, &DebListModel::refreshMultiPage);

    //刷新批量安装界面
    connect(m_packagesManager, &PackagesManager::single2MultiPage, this, &DebListModel::single2MultiPage);

    //告诉前端当前处在添加过程中
    connect(m_packagesManager, &PackagesManager::appendStart, this, &DebListModel::appendStart);

    //提示前端当前已经添加完成
    connect(m_packagesManager, &PackagesManager::appendFinished, this, &DebListModel::getPackageMd5);
    //检查系统版本与是否开启了开发者模式
    checkSystemVersion();
}

/**
 * @brief DebListModel::DealDependResult       根据依赖安装的进程标识，处理依赖安装的流程显示
 * @param iAuthRes                             依赖安装的进度标识
 * @param iIndex                               缺失依赖的包的index
 * @param dependName                           依赖的名称
 */
void DebListModel::DealDependResult(int iAuthRes, int iIndex, QString dependName)
{
    switch (iAuthRes) {
    case DebListModel::CancelAuth:
        m_packageOperateStatus[m_packagesManager->getPackageMd5(iIndex)] = Prepare;           //取消授权后，缺失wine依赖的包的操作状态修改为prepare
        break;
    case DebListModel::AuthConfirm:                         //确认授权后，状态的修改由debinstaller进行处理
        break;
    case DebListModel::AuthDependsSuccess:                  //安装成功后，状态的修改由debinstaller进行处理
        break;
    case DebListModel::AuthDependsErr:                      //安装失败后，状态的修改由debinstaller进行处理
        break;
    default:
        break;
    }
    emit DependResult(iAuthRes, dependName);                //发送信号，由debinstaller处理界面状态。
}

/**
 * @brief DebListModel::isReady 判断当前库是否准备就绪
 * @return  库是否准备就绪
 */
bool DebListModel::isReady() const
{
    return m_packagesManager->isBackendReady();
}

/**
 * @brief DebListModel::preparedPackages    获取当前安装器内部的所有的包
 * @return  获取包的列表
 */
const QList<QString> DebListModel::preparedPackages() const
{
    return m_packagesManager->m_preparedPackages;
}

/**
 * @brief DebListModel::first 获取安装器添加的第一个包
 * @return 第一个包的index
 */
QModelIndex DebListModel::first() const
{
    return index(0);
}

/**
 * @brief DebListModel::rowCount 获取model的行数
 * @param parent
 * @return 当前软件包内部包的数量
 */
int DebListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_packagesManager->m_preparedPackages.size();
}

/**
 * @brief DebListModel::data 获取包的信息
 * @param index 包的index
 * @param role  要获取的包的信息的类别
 * @return
 */
QVariant DebListModel::data(const QModelIndex &index, int role) const
{
    const int r = index.row();
    const DebFile *deb = new DebFile(m_packagesManager->package(r));

    QString packageName = deb->packageName();                       //包名
    QString filePath = deb->filePath();                             //包的路径
    QString version = deb->version();                               //包的版本
    QString architecture = deb->architecture();                     //包可用的架构
    QString shortDescription = deb->shortDescription();             //包的短描述
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
        return m_packagesManager->packageInstallStatus(r);          //获取当前index包的安装状态
    case PackageDependsStatusRole:
        return m_packagesManager->getPackageDependsStatus(r).status;   //获取当前index包的依赖状态
    case PackageInstalledVersionRole:
        return m_packagesManager->packageInstalledVersion(r);       //获取当前index包在系统中安装的版本
    case PackageAvailableDependsListRole:
        return m_packagesManager->packageAvailableDepends(r);       //获取当前index包可用的依赖
    case PackageReverseDependsListRole:
        return m_packagesManager->packageReverseDependsList(packageName, architecture); //获取依赖于当前index包的应用
    case PackageDescriptionRole:
        return Utils::fromSpecialEncoding(shortDescription);        //获取当前index包的短描述
    case PackageFailReasonRole:
        return packageFailedReason(r);                              //获取当前index包的安装失败的原因
    case PackageOperateStatusRole: {
        auto md5 = m_packagesManager->getPackageMd5(r);
        if (m_packageOperateStatus.contains(md5))                     //获取当前包的操作状态
            return m_packageOperateStatus[md5];
        else
            return Prepare;
    }
    case Qt::SizeHintRole:                                          //设置当前index的大小
        return QSize(0, 48);

    default:
        ;
    }

    return QVariant();
}

/**
 * @brief DebListModel::installPackages 开始安装
 * 安装之前状态初始化.
 * 开始安装流程
 */
void DebListModel::installPackages()
{
    qInfo() << "[Performance Testing] Click the install button";
    Q_ASSERT_X(m_workerStatus == WorkerPrepare, Q_FUNC_INFO, "installer status error");
    if (m_workerStatus != WorkerPrepare) return;

    m_workerStatus = WorkerProcessing;                                  //刷新包安装器的工作状态
    m_workerStatus_temp = m_workerStatus;
    m_operatingIndex = 0;                                               //初始化当前操作的index
    m_operatingStatusIndex = 0;
    m_operatingPackageMd5 = m_packageMd5[m_operatingIndex];

    // start first
    initRowStatus();                                                    //初始化包的操作状态
    installNextDeb();                                                   //开始安装
}

/**
 * @brief DebListModel::uninstallPackage 卸载某一个包
 * @param idx   要卸载的包的index
 */
void DebListModel::uninstallPackage(const int idx)
{
    Q_ASSERT(idx == 0);
    Q_ASSERT_X(m_workerStatus == WorkerPrepare, Q_FUNC_INFO, "uninstall status error");

    m_workerStatus = WorkerProcessing;                  //刷新当前包安装器的工作状态
    m_workerStatus_temp = m_workerStatus;               //保存工作状态
    m_operatingIndex = idx;                             //获取卸载的包的indx
    m_operatingPackageMd5 = m_packageMd5[m_operatingIndex];
    // fix bug : 卸载失败时不提示卸载失败。
    m_operatingStatusIndex = idx;                       //刷新操作状态的index

    DebFile *deb = new DebFile(m_packagesManager->package(m_operatingIndex));   //获取到包
    // 添加详细的大小信息
    PERF_PRINT_BEGIN("POINT-04", "pkgsize=" + QString::number(deb->installedSize()) + "b");    //卸载包开始

    const QStringList rdepends = m_packagesManager->packageReverseDependsList(deb->packageName(), deb->architecture());     //检查是否有应用依赖到该包
    Backend *b = m_packagesManager->m_backendFuture.result();
    for (const auto &r : rdepends) {                                        // 卸载所有依赖该包的应用（二者的依赖关系为depends）
        if (b->package(r))
            b->markPackageForRemoval(r);
        else
            qDebug() << "DebListModel:" << "reverse depend" << r << "error ,please check it!";
    }
    b->markPackageForRemoval(deb->packageName() + ':' + deb->architecture());       //卸载当前包

    // uninstall
    qDebug() << "DebListModel:" << "starting to remove package: " << deb->packageName() << rdepends;

    refreshOperatingPackageStatus(Operating);                                       //刷新当前index的操作状态
    Transaction *trans = b->commitChanges();

    // trans 进度change 链接
    connect(trans, &Transaction::progressChanged, this, &DebListModel::transactionProgressChanged);

    //详细状态信息（安装情况）展示链接
    connect(trans, &Transaction::statusDetailsChanged, this, &DebListModel::appendOutputInfo);

    // trans 运行情况（授权是否成功）
    connect(trans, &Transaction::statusChanged, this, &DebListModel::onTransactionStatusChanged);

    // trans运行中出现错误
    connect(trans, &Transaction::errorOccurred, this, &DebListModel::onTransactionErrorOccurred);

    // 卸载结束，处理卸载成功与失败的情况并发送结束信号
    connect(trans, &Transaction::finished, this, &DebListModel::uninstallFinished);

    // 卸载结束之后 删除指针
    connect(trans, &Transaction::finished, trans, &Transaction::deleteLater);

    m_currentTransaction = trans;   //保存trans指针

    trans->run();                   //开始卸载
    delete deb;
}

/**
 * @brief DebListModel::removePackage   从prepare列表中删除某一个包
 * @param idx   要删除的包的index
 * 逻辑有问题，稍后修改
 */
void DebListModel::removePackage(const int idx)
{
    Q_ASSERT_X(m_workerStatus == WorkerPrepare, Q_FUNC_INFO, "installer status error");

    // 获取wine依赖不满足的包的下标
    //这个地方如果同时存在两个wine依赖不满足的包，删除之后可能会将两个包一起删除
    //此处删除 标记
    qDebug() << "DebListModel:" << "remove package: operate Status" << m_packageOperateStatus;

    // 去除操作状态 中的index
    // 删除的时候 package的状态一定是prepare 全部删除之后再次重新添加。
    int packageOperateStatusCount = m_packageOperateStatus.size() - 1;
    m_packageOperateStatus.clear();
    for (int i = 0; i < packageOperateStatusCount; i++) {
        m_packageOperateStatus[m_packagesManager->getPackageMd5(i)] = DebListModel::Prepare;
    }

    qDebug() << "DebListModel:" << "operate Status remove";
    m_packagesManager->removePackage(idx);       //在packageManager中删除标记的下标
}

/**
 * @brief DebListModel::appendPackage 向安装器内部添加包
 * @param package   包的文件路径
 * 修改返回方式为void
 * 修改为在线程中添加
 * 使用信号判断是否添加成功
 * 修改直接将所有的包都传递到后端处理，前端只根据后端的处理结果刷新界面
 */
void DebListModel::appendPackage(QStringList package)
{
    Q_ASSERT_X(m_workerStatus == WorkerPrepare, Q_FUNC_INFO, "installer status error");
    m_packagesManager->appendPackage(package);      //添加包，并返回添加结果
}

/**
 * @brief DebListModel::onTransactionErrorOccurred 处理trans过程中出现的错误
 */
void DebListModel::onTransactionErrorOccurred()
{
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");
    Transaction *trans = static_cast<Transaction *>(sender());

    //fix bug:39834
    //失败时刷新操作状态为failed,并记录失败原因
    refreshOperatingPackageStatus(Failed);
    m_packageOperateStatus[m_operatingPackageMd5] = Failed;

    //记录失败代码与失败原因
    // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
    m_packageFailCode[m_operatingPackageMd5] = trans->error();
    m_packageFailReason[m_operatingPackageMd5] = trans->errorString();
    //fix bug: 点击重新后，授权码输入框弹出时反复取消输入，进度条已显示进度
    //取消安装后，Errorinfo被输出造成进度条进度不为0，现屏蔽取消授权错误。
    //授权错误不再输出到详细信息中
    if (!trans->errorString().contains("proper authorization was not provided"))
        emit appendOutputInfo(trans->errorString());

    const QApt::ErrorCode e = trans->error();               //trans错误的代码
    Q_ASSERT(e);

    qWarning() << "DebListModel:" << "Transaction Error:" << e << workerErrorString(e, trans->errorString());
    qWarning() << "DebListModel:" << "Error Infomation:" << trans->errorDetails() << trans->errorString();

    if (trans->isCancellable()) trans->cancel();

    //特殊处理授权错误
    if (e == AuthError) {
        trans->deleteLater();                                                       //删除 trans指针
        QTimer::singleShot(100 * 1, this, &DebListModel::checkBoxStatus);           //检查授权弹窗的状态 如果弹窗仍然在只是超时，则底层窗口按钮不可用
        qDebug() << "DebListModel:" << "Authorization error";

        // reset env
        emit AuthCancel();                                                          //发送授权被取消的信号
        emit lockForAuth(false);                                                    //取消授权锁定，设置按钮可用
        //EnableReCancelBtn在信号在checkBoxStatus已发送，修改为enableCloseButton
        emit enableCloseButton(true);
        m_workerStatus = WorkerPrepare;                                             // 重置工作状态为准备态
        m_workerStatus_temp = m_workerStatus;

        PERF_PRINT_END("POINT-04");                                                 //安装时间
        PERF_PRINT_END("POINT-05");                                                 //授权框消失时间

        return;
    }

    // DO NOT install next, this action will finished and will be install next automatic.
    trans->setProperty("exitStatus", QApt::ExitFailed);                             //设置trans的退出状态为 失败
    PERF_PRINT_END("POINT-04");                                                     //安装时间
}

/**
 * @brief DebListModel::onTransactionStatusChanged trans状态变化的槽函数
 * @param stat  变化后的Transaction状态
 */
void DebListModel::onTransactionStatusChanged(TransactionStatus stat)
{
    switch (stat) {
    case TransactionStatus::AuthenticationStatus:           //等待授权
        emit lockForAuth(true);                             //设置底层窗口按钮不可用
        break;
    case TransactionStatus::WaitingStatus:                  //当前操作在队列中等待操作
        emit lockForAuth(false);                            //设置底层窗口按钮可用
        break;
    default:
        ;
    }

}

/**
 * @brief DebListModel::getInstallFileSize  获取当前安装器内部准备安装的包的数量
 * @return 准备安装的包的数量
 */
int DebListModel::getInstallFileSize()
{
    return m_packagesManager->m_preparedPackages.size();
}

/**
 * @brief DebListModel::reset 重置状态
 * 工作状态重置为准备态
 * 当前操作的index置为0
 * 当前操作状态的index置为0
 * 清空操作状态列表
 * 清空错误代码和错误原因列表
 * 重置PackageManager
 */
void DebListModel::reset()
{
    m_workerStatus = WorkerPrepare;                     //工作状态重置为准备态
    m_workerStatus_temp = m_workerStatus;
    m_operatingIndex = 0;                               //当前操作的index置为0
    m_operatingPackageMd5 = nullptr;
    m_operatingStatusIndex = 0;                         //当前操作状态的index置为0

    m_packageOperateStatus.clear();                     //清空操作状态列表
    m_packageFailCode.clear();                          //清空错误原因列表
    m_packageFailReason.clear();
    m_packagesManager->reset();                         //重置packageManager
}

/**
 * @brief DebListModel::reset_filestatus 重置包的状态
 * 重置包的操作状态
 * 重置包的错误状态
 */
void DebListModel::reset_filestatus()
{
    m_packageOperateStatus.clear();                     //重置包的操作状态
    m_packageFailReason.clear();                        //重置包的错误状态
    m_packageFailCode.clear();
}

/**
 * @brief DebListModel::bumpInstallIndex 批量安装流程控制
 * 安装流程控制：
 * 安装完成后，判断后续是否还有下一个包等待安装
 * 全部安装成功后重置工作状态，发送结束信号，修改工作进度。
 */
void DebListModel::bumpInstallIndex()
{
    if (m_currentTransaction.isNull()) {
        qInfo() << "previous transaction not finished";
    }

    if (++m_operatingIndex == m_packagesManager->m_preparedPackages.size()) {
        qDebug() << "congratulations, install finished !!!";
        m_workerStatus = WorkerFinished;                                        //设置包安装器的工作状态为Finish
        m_workerStatus_temp = m_workerStatus;
        emit workerFinished();                                                  //发送安装完成信号
        emit workerProgressChanged(100);                                        //修改安装进度
        emit transactionProgressChanged(100);
        PERF_PRINT_END("POINT-04");                                             //安装时间
        return;
    }
    ++ m_operatingStatusIndex;
    m_operatingPackageMd5 = m_packageMd5[m_operatingIndex];
    emit onChangeOperateIndex(m_operatingIndex);                                //修改当前操作的下标
    // install next
    qDebug() << "DebListModel:" << "install next deb package";
    PERF_PRINT_END("POINT-04");                                                 //安装时间
    installNextDeb();                                                           //安装下一个包
}

/**
 * @brief DebListModel::refreshOperatingPackageStatus 刷新当前正在操作的包的状态
 * @param stat 要刷新的状态
 */
void DebListModel::refreshOperatingPackageStatus(const DebListModel::PackageOperationStatus stat)
{
    m_packageOperateStatus[m_operatingPackageMd5] = stat;  //将失败包的索引和状态修改保存,用于更新

    const QModelIndex idx = index(m_operatingStatusIndex);

    emit dataChanged(idx, idx);                             //发送状态已经修改的信号
}

/**
 * @brief DebListModel::packageFailedReason 获取包安装错误原因的详细描述
 * @param idx 安装发生错误的包的下标
 * @return 错误的详细原因
 *
 * 根据 依赖安装情况，架构情况，冲突情况等初步判断包安装失败或不能安装的原因并返回
 */
QString DebListModel::packageFailedReason(const int idx) const
{
    const auto stat = m_packagesManager->getPackageDependsStatus(idx);                         //获取包的依赖状态
    const auto md5 = m_packagesManager->getPackageMd5(idx);                                 //获取包的md5值
    if (m_packagesManager->isArchError(idx)) return tr("Unmatched package architecture");   //判断是否架构冲突
    if (stat.isBreak() || stat.isAuthCancel()) {                                            //依赖状态错误
        if (!stat.package.isEmpty()) {
            if (m_packagesManager->m_errorIndex.contains(md5))     //修改wine依赖的标记方式
                return tr("Failed to install %1").arg(stat.package);                        //wine依赖安装失败
            return tr("Broken dependencies: %1").arg(stat.package);                         //依赖不满足
        }

        const auto conflict = m_packagesManager->packageConflictStat(idx);                  //获取冲突情况
        if (!conflict.is_ok()) return tr("Broken dependencies: %1").arg(conflict.unwrap()); //依赖冲突
    }

    Q_ASSERT(m_packageOperateStatus.contains(md5));
    Q_ASSERT(m_packageOperateStatus[md5] == Failed);
    //判断当前这个包是否错误
    if (!m_packageFailCode.contains(md5))
        qInfo() << "DebListModel:" << "failed to get reason" << m_packageFailCode.size() << idx;
    Q_ASSERT(m_packageFailCode.contains(md5));

    // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
    return workerErrorString(m_packageFailCode[md5], m_packageFailReason[md5]);             //根据错误代码和错误原因返回具体的错误原因
}

/**
 * @brief DebListModel::onTransactionFinished transaction结束的槽函数
 *
 */
void DebListModel::onTransactionFinished()
{
    if (m_workerStatus == WorkerProcessing) {
        qInfo() << "installer status error";
    }
    Transaction *trans = static_cast<Transaction *>(sender());                              // 获取trans指针


    // prevent next signal
    disconnect(trans, &Transaction::finished, this, &DebListModel::onTransactionFinished);  //不再接收trans结束的信号

    // report new progress
    //更新安装进度（批量安装进度控制）
    int progressValue = static_cast<int>(100. * (m_operatingIndex + 1) / m_packagesManager->m_preparedPackages.size());
    emit workerProgressChanged(progressValue);

    qDebug() << "DebListModel:" << "transaciont finished with exit status:" << trans->exitStatus();
    if (trans->exitStatus()) {
        //安装失败
        qWarning() << trans->error() << trans->errorDetails() << trans->errorString();
        //保存错误原因和错误代码
        // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
        m_packageFailCode[m_operatingPackageMd5] = trans->error();
        m_packageFailReason[m_operatingPackageMd5] = trans->errorString();

        //刷新操作状态
        refreshOperatingPackageStatus(Failed);
        emit appendOutputInfo(trans->errorString());
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
    trans->deleteLater();
    m_currentTransaction = nullptr;
    bumpInstallIndex();                 //进入安装进度控制
}

/**
 * @brief DebListModel::onDependsInstallTransactionFinished 依赖安装结束的槽函数
 *
 * 普通依赖安装结束后开始安装拖入的包
 */
void DebListModel::onDependsInstallTransactionFinished()//依赖安装关系满足
{
    if (m_workerStatus == WorkerProcessing) {
        qInfo() << "installer status error";
    }
    Transaction *trans = static_cast<Transaction *>(sender());                                  //获取transaction指针

    const auto ret = trans->exitStatus();

    if (ret) qWarning() << trans->error() << trans->errorDetails() << trans->errorString();     //transaction发生错误

    if (ret) {
        // record error
        // 记录错误原因和错误代码
        // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
        m_packageFailCode[m_operatingPackageMd5] = trans->error();
        m_packageFailReason[m_operatingPackageMd5] = trans->errorString();
        refreshOperatingPackageStatus(Failed);                                  // 刷新操作状态
        emit appendOutputInfo(trans->errorString());
    }

    //    delete trans;
    trans->deleteLater();
    m_currentTransaction = nullptr;

    // check current operate exit status to install or install next
    if (ret)
        bumpInstallIndex();                                                     //依赖安装失败，直接安装下一个包
    else
        installNextDeb();                                                       //依赖安装成功，开始安装这个包
}

/**
 * @brief DebListModel::setEndEnable 授权框消失之后，设置底层按钮可用
 */
void DebListModel::setEndEnable()
{
    emit EnableReCancelBtn(true);
}

/**
 * @brief DebListModel::checkBoxStatus check 授权弹出窗口的弹出状态
 * 根据授权弹出窗口的弹出状态判断当前底层安装按钮是否可用
 */
void DebListModel::checkBoxStatus()
{
    QTime initTime = QTime::currentTime();                                      //获取弹出的时间
    Transaction *trans = nullptr;
    auto *const backend = m_packagesManager->m_backendFuture.result();
    trans = backend->commitChanges();

    QTime stopTime = QTime::currentTime();
    int elapsed = initTime.msecsTo(stopTime);                                   //获取commit授权被取消的时间
    if (elapsed > 20000) {                                                      //如果时间超过20ms则不断判断当前窗口是否超时
        QTimer::singleShot(100 * 1, this, &DebListModel::checkBoxStatus);
        return;
    }

    if (trans != nullptr) {
        if (trans->isCancellable()) {                                           //当前transaction已经被取消
            trans->cancel();
            QTimer::singleShot(100 * 1, this, &DebListModel::setEndEnable);     //设置按钮可用
        } else {
            QTimer::singleShot(100 * 1, this, &DebListModel::checkBoxStatus);   //当前Transaction还在运行中，继续等待并判断
        }
    } else {
        qDebug() << "DebListModel:" << "Transaction is Nullptr";
    }
}

/**
 * @brief DebListModel::installDebs 安装当前包
 * 发送启动信号
 * 检查安装环境
 * 判断依赖是否可用
 * 下载普通依赖
 * 安装
 */
void DebListModel::installDebs()
{
    DebFile deb(m_packagesManager->package(m_operatingIndex)) ;
    PERF_PRINT_BEGIN("POINT-04", "packsize=" + QString::number(deb.installedSize()) + "b");         //安装时间
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");
    Q_ASSERT_X(m_currentTransaction.isNull(), Q_FUNC_INFO, "previous transaction not finished");

    //在判断dpkg启动之前就发送开始安装的信号，并在安装信息中输出 dpkg正在运行的信息。
    emit onStartInstall();
    if (isDpkgRunning()) {
        qDebug() << "DebListModel:" << "dpkg running, waitting...";
        // 缩短检查的时间，每隔1S检查当前dpkg是否正在运行。
        QTimer::singleShot(1000 * 1, this, &DebListModel::installNextDeb);
        emit appendOutputInfo("dpkg running, waitting...");                 //发送提示，告知用户dpkg正在运行
        return;
    }

    // fetch next deb
    auto *const backend = m_packagesManager->m_backendFuture.result();

    Transaction *trans = nullptr;

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
        // 依赖可用 但是需要下载
        Q_ASSERT_X(m_packageOperateStatus[m_operatingPackageMd5], Q_FUNC_INFO,
                   "package operate status error when start install availble dependencies");

        // 获取到所有的依赖包 准备安装
        const QStringList availableDepends = m_packagesManager->packageAvailableDepends(m_operatingIndex);
        //        for (auto const &p : availableDepends) backend->markPackageForInstall(p);
        //获取到可用的依赖包并根据后端返回的结果判断依赖包的安装结果
        for (auto const &p : availableDepends) {
            if (p.contains(" not found")) {                             //依赖安装失败
                refreshOperatingPackageStatus(Failed);                  //刷新当前包的状态
                // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
                m_packageFailCode.insert(m_operatingPackageMd5, DownloadDisallowedError);                       //记录错误代码与错误原因
                m_packageFailReason.insert(m_operatingPackageMd5, p);
                emit appendOutputInfo(m_packagesManager->package(m_operatingIndex) + "\'s depend " + " " + p);  //输出错误原因
                bumpInstallIndex();                                     //开始安装下一个包或结束安装
                return;
            }
            backend->markPackageForInstall(p);                          //开始安装依赖包
        }

        //安装
        qDebug() << "DebListModel:" << "install" << deb.packageName() << "dependencies: " << availableDepends;

        trans = backend->commitChanges();
        //依赖安装结果处理
        connect(trans, &Transaction::finished, this, &DebListModel::onDependsInstallTransactionFinished);
    } else {
        //开始安装当前包
        qDebug() << "DebListModel:" << "starting to install package: " << deb.packageName();

        PERF_PRINT_BEGIN("POINT-05", "packsize=" + QString::number(deb.installedSize()) + "b");
        qInfo() << "[Performance Testing]：install Files";
        trans = backend->installFile(deb);//触发Qapt授权框和安装线程

        // 进度变化和结束过程处理
        connect(trans, &Transaction::progressChanged, this, &DebListModel::transactionProgressChanged);
        connect(trans, &Transaction::finished, this, &DebListModel::onTransactionFinished);
    }

    // NOTE: DO NOT remove this.
    // see: https://bugs.kde.org/show_bug.cgi?id=382272
    trans->setLocale(".UTF-8");

    //记录日志
    connect(trans, &Transaction::statusDetailsChanged, this, &DebListModel::appendOutputInfo);

    //刷新操作状态
    connect(trans, &Transaction::statusDetailsChanged, this, &DebListModel::onTransactionOutput);

    //授权处理
    connect(trans, &Transaction::statusChanged, this, &DebListModel::onTransactionStatusChanged);

    //错误处理
    connect(trans, &Transaction::errorOccurred, this, &DebListModel::onTransactionErrorOccurred);

    m_currentTransaction = trans;

    qInfo() << "[Performance Testing] Pop up authorization box";
    m_currentTransaction->run();
}

/**
 * @brief DebListModel::digitalVerifyFailed 数字签名校验失败的处理槽函数
 */
void DebListModel::digitalVerifyFailed(ErrorCode code)
{
    if (preparedPackages().size() > 1) {                        //批量安装
        refreshOperatingPackageStatus(Failed);                  //刷新操作状态
        // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
        m_packageFailCode.insert(m_operatingPackageMd5, code); //记录错误代码与错误原因
        m_packageFailReason.insert(m_operatingPackageMd5, "");
        bumpInstallIndex();                                     //跳过当前包
    } else if (preparedPackages().size() == 1) {
        exit(0);                                                //单包安装 直接退出
    }
}

/**
 * @brief DebListModel::showNoDigitalErrWindow 无数字签名
 */
void DebListModel::showNoDigitalErrWindow()
{
    //批量安装时，如果不是最后一个包，则不弹窗，只记录详细错误原因。
    //task:https://pms.uniontech.com/zentao/task-view-48443.html
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

    Ddialog->addButton(QString(tr("Cancel")), true, DDialog::ButtonNormal);     //添加取消按钮
    Ddialog->addButton(QString(tr("Proceed")), true, DDialog::ButtonRecommend);  //添加前往按钮
    Ddialog->show();    //显示弹窗

    //取消按钮
    QPushButton *btnCancel = qobject_cast<QPushButton *>(Ddialog->getButton(0));
    connect(btnCancel, &DPushButton::clicked, this, [ = ] {
        //跳过当前包，当前包的安装状态为失败，失败原因是无数字签名
        digitalVerifyFailed(NoDigitalSignature);
    });

    //前往按钮
    QPushButton *btnProceedControlCenter = qobject_cast<QPushButton *>(Ddialog->getButton(1));
    connect(btnProceedControlCenter, &DPushButton::clicked, this, [ = ] {
        //前往控制中心
        showDevelopModeWindow();
        exit(0);
    });

    //关闭图标
    connect(Ddialog, &DDialog::aboutToClose, this, [ = ] {
        //跳过当前包，当前包的安装状态为失败，失败原因是无数字签名
        digitalVerifyFailed(NoDigitalSignature);
    });
}

/**
 * @brief DebListModel::showDigitalErrWindow 无有效的数字签名
 */
void DebListModel::showDigitalErrWindow()
{
    //批量安装时，如果不是最后一个包，则不弹窗，只记录详细错误原因。
    //task:https://pms.uniontech.com/zentao/task-view-48443.html
    if (m_operatingIndex < m_packagesManager->m_preparedPackages.size() - 1) {
        digitalVerifyFailed(DigitalSignatureError);     //刷新安装错误，并记录错误原因
        return;
    }
    DDialog *Ddialog = new DDialog();
    //设置窗口焦点
    //fix bug:https://pms.uniontech.com/zentao/bug-view-44837.html
    Ddialog->setFocusPolicy(Qt::TabFocus);

    //设置弹出窗口为模态窗口
    Ddialog->setModal(true);

    //设置窗口始终置顶
    Ddialog->setWindowFlag(Qt::WindowStaysOnTopHint);

    // 设置弹出窗口显示的信息
    Ddialog->setTitle(tr("Unable to install"));
    Ddialog->setMessage(QString(tr("This package does not have a valid digital signature")));
    Ddialog->setIcon(QIcon::fromTheme("di_popwarning"));
    Ddialog->addButton(QString(tr("OK")), true, DDialog::ButtonNormal);
    Ddialog->show();
    QPushButton *btnOK = qobject_cast<QPushButton *>(Ddialog->getButton(0));

    // fix bug:44837 https://pms.uniontech.com/zentao/bug-view-44837.html
    btnOK->setFocusPolicy(Qt::TabFocus);
    btnOK->setFocus();
    // 点击弹出窗口的关闭图标按钮
    connect(Ddialog, &DDialog::aboutToClose, this, [ = ] {
        //刷新当前包的操作状态，失败原因为数字签名校验失败
        digitalVerifyFailed(DigitalSignatureError);
    });

    //点击弹出窗口的确定按钮
    connect(btnOK, &DPushButton::clicked, this, [ = ] {
        //刷新当前包的操作状态，失败原因为数字签名校验失败
        digitalVerifyFailed(DigitalSignatureError);
    });
}

/**
 * @brief DebListModel::showDevelopModeWindow 打开控制中心通用界面
 */
void DebListModel::showDevelopModeWindow()
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
    return;
}

/**
 * @brief DebListModel::checkSystemVersion 检查当前操作系统的版本，判断是否需要验证签名
 *
 * 个人版专业版 非开模式需要验证签名
 * 服务器版 没有开发者模式，默认不验证签名
 * 社区版默认开发者模式，不验证签名。
 * 此部分修改，无法影响control依赖，服务器版与社区版需要在Control文件中去除deepin-elf-sign-tool依赖
 * 已经有更新的接口，稍后需要更新
 * PS: 2020/10/12 已经更新
 */
void DebListModel::checkSystemVersion()
{
    // add for judge OS Version
    // 修改获取系统版本的方式 此前为  DSysInfo::deepinType()
    switch (Dtk::Core::DSysInfo::uosEditionType()) {            //获取系统的类型
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

/**
 * @brief DebListModel::checkDigitalSignature 验证开发者模式与数字签名
 * @return 是否能够安装当前包
 * 如果当前处于开发者模式或处于非开发者模式但有数字签名 则验证成功
 * 如果处于非开模式且无数字签名。则验证失败
 */
bool DebListModel::checkDigitalSignature()
{
    if (m_isDevelopMode) {
        qInfo() << "The developer mode is currently enabled, and the digital signature is not verified";
        return true;
    }
    int digitalSigntual = Utils::Digital_Verify(m_packagesManager->package(m_operatingIndex)); //非开模式，判断是否有数字签名
    switch (digitalSigntual) {
    case Utils::VerifySuccess:                                                                  //签名验证成功
        qInfo() << "Digital signature verification succeed";
        return true;
    case Utils::DebfileInexistence:                                                             //无签名文件
        qInfo() << "No signature file was found in the application";
        showNoDigitalErrWindow();
        return false;
    case Utils::ExtractDebFail:
        showDigitalErrWindow();
        qInfo() << "An error occurred while verifying the signature";                           //无有效的数字签名
        return false;
    case Utils::DebVerifyFail:
    case Utils::OtherError:
        showDigitalErrWindow();
        qInfo() << "Signature file verification failed";                                        //其他原因造成的签名校验失败
        return false;
    default:                                                                                    //其他未知错误
        qInfo() << "unknown mistake";
        return false;
    }
}

/**
 * @brief DebListModel::installNextDeb
 * 检查签名认证结果
 * 判断是否当前包是否需要配置
 */
void DebListModel::installNextDeb()
{
    if (!checkDigitalSignature()) {                             //非开发者模式且数字签名验证失败
        return;
    } else {
        QString sPackageName = m_packagesManager->m_preparedPackages[m_operatingIndex];
        QStringList strFilePath;
        if (checkTemplate(sPackageName)) {                      //检查当前包是否需要配置
            rmdir();                                            //删除临时路径
            qDebug() << "DebListModel:" << "command install" << sPackageName;
            m_procInstallConfig->start("pkexec", QStringList() << "deepin-deb-installer-dependsInstall" << "InstallConfig" << sPackageName);
        } else {
            qDebug() << "DebListModel:" << "normal install" << sPackageName;
            installDebs();                                      //普通安装
        }
    }
}

/**
 * @brief DebListModel::rmdir 删除临时目录
 */
void DebListModel::rmdir()
{
    QDir filePath(tempPath);
    if (filePath.exists()) {
        if (filePath.removeRecursively()) {
            qDebug() << "DebListModel:" << "remove temporary path success";
        } else {
            qDebug() << "DebListModel:" << "remove temporary path failed";
        }
    }
}

/**
 * @brief DebListModel::checkTemplate 检查template文件是否存在
 * @param debPath 包的路径
 * @return template是否存在
 * 根据template 来判断当前包是否需要配置
 */
bool DebListModel::checkTemplate(QString debPath)
{
    rmdir();
    getDebian(debPath);
    QFile templates(tempPath + "/templates");
    if (templates.exists()) {
        qDebug() << "DebListModel:" << "Check that the template file exists";
        return true;
    }
    return false;
}
/**
 * @brief DebListModel::mkdir 创建临时文件夹
 * @return 是否创建成功。
 */
bool DebListModel::mkdir()
{
    QDir filePath(tempPath);                //获取配置包的临时路径

    if (!filePath.exists()) {               //当前临时路径不存在
        return filePath.mkdir(tempPath);    //删除临时路径，并返回删除结果
    }
    return true;
}

/**
 * @brief DebListModel::getDebian
 * @param debPath 包的路径
 * 通过dpkg获取当前包的DEBIAN文件
 */
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
        qDebug() << "DebListModel:" << "Failed to decompress the main control file" << getDebianProcessErrInfo;
    }
    delete m_pDpkg;
}

/**
 * @brief DebListModel::onTransactionOutput 刷新当前包的操作状态
 */
void DebListModel::onTransactionOutput()
{
    if (m_workerStatus == WorkerProcessing) {
        qInfo() << "installer status error";
    }
    Transaction *trans = static_cast<Transaction *>(sender());
    Q_ASSERT(trans == m_currentTransaction.data());

    refreshOperatingPackageStatus(Operating);                       //刷新当前包的操作状态

    disconnect(trans, &Transaction::statusDetailsChanged, this, &DebListModel::onTransactionOutput);
    PERF_PRINT_END("POINT-05");                                     //授权框时间
}

/**
 * @brief DebListModel::uninstallFinished 卸载包结束
 */
void DebListModel::uninstallFinished()
{
    if (m_workerStatus == WorkerProcessing) {
        qInfo() << "installer status error";
    }

    //增加卸载失败的情况
    //此前的做法是发出commitError的信号，现在全部在Finished中进行处理。不再特殊处理。
    Transaction *trans = static_cast<Transaction *>(sender());
    qDebug() << "DebListModel:" << "uninstall finished with finished code:" << trans->error() << "finished details:" << trans->errorString();
    if (trans->exitStatus()) {
        m_workerStatus = WorkerFinished;                            //刷新包安装器的工作状态
        m_workerStatus_temp = m_workerStatus;
        refreshOperatingPackageStatus(Failed);                      //刷新当前包的操作状态
        m_packageOperateStatus[m_operatingPackageMd5] = Failed;
    } else {
        m_workerStatus = WorkerFinished;                            //刷新包安装器的工作状态
        m_workerStatus_temp = m_workerStatus;
        refreshOperatingPackageStatus(Success);                     //刷新当前包的卸载状态
        m_packageOperateStatus[m_operatingPackageMd5] = Success;
    }
    emit workerFinished();                                          //发送结束信号（只有单包卸载）卸载结束就是整个流程的结束
    trans->deleteLater();
    PERF_PRINT_END("POINT-04");                                     //安装时间
}

/**
 * @brief DebListModel::setCurrentIndex 修改当前操作的index、
 * @param idx
 */
void DebListModel::setCurrentIndex(const QModelIndex &idx)
{
    if (m_currentIdx == idx) return;                //要修改的index与当前index 一致

    const QModelIndex index = m_currentIdx;         //保存当前的index
    m_currentIdx = idx;                             //修改当前的index

    emit dataChanged(index, index);
    emit dataChanged(m_currentIdx, m_currentIdx);   //发送index修改信号
}

/**
 * @brief DebListModel::initPrepareStatus 刷新当前所有包的状态为Prepare
 */
void DebListModel::initPrepareStatus()
{
    qDebug() << "DebListModel: " << "Reresh all package operate status to Prepare";
    for (int i = 0; i < m_packagesManager->m_preparedPackages.size(); i++) {
        auto md5 = m_packagesManager->getPackageMd5(i);
        m_packageOperateStatus.insert(md5, Prepare);                          //刷新当前所有包的状态为Prepare
    }
}

/**
 * @brief DebListModel::initRowStatus 开始安装后，刷新所有包的状态为等待安装
 */
void DebListModel::initRowStatus()
{
    for (int i = 0; i < m_packagesManager->m_preparedPackages.size(); i++) {
        m_operatingStatusIndex = i;
        refreshOperatingPackageStatus(Waiting);
    }
    m_operatingStatusIndex = 0;
}

/**
 * @brief DebListModel::upWrongStatusRow 将安装失败的包上滚
 * 刷新包的操作状态
 * 刷新包的MD5
 * 刷新包安装失败的原因
 * 刷新包安装的状态
 * 刷新包的依赖状态
 *
 * 此部分代码可以优化，部分状态无需刷新
 */
void DebListModel::upWrongStatusRow()
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
    emit onChangeOperateIndex(-1);
}

/**
 * @brief DebListModel::ConfigInstallFinish
 * @param flag 安装配置包的返回结果
 * 处理命令的返回结果
 */
void DebListModel::ConfigInstallFinish(int flag)
{
    int progressValue = static_cast<int>(100. * (m_operatingIndex + 1) / m_packagesManager->m_preparedPackages.size()); //批量安装时对进度进行处理
    emit workerProgressChanged(progressValue);
    qDebug() << "DebListModel:" << "config install result:" << flag;
    if (flag == 0) {        //安装成功
        if (m_packagesManager->m_packageMd5DependsStatus[m_packagesManager->m_packageMd5[m_operatingIndex]].status == DependsOk) {
            refreshOperatingPackageStatus(Success);                 //刷新安装状态
        }
        bumpInstallIndex();                                         //开始安装下一个
    } else {
        if (m_packagesManager->m_preparedPackages.size() == 1) {    //单包安装
            refreshOperatingPackageStatus(Prepare);                 //刷新当前包的操作状态为准备态
            m_workerStatus = WorkerPrepare;
            emit AuthCancel();                                      //授权取消
        } else {
            //批量安装
            refreshOperatingPackageStatus(Failed);                  //刷新当前包的状态为失败

            // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
            m_packageFailCode.insert(m_operatingPackageMd5, flag);       //保存失败原因
            m_packageFailReason.insert(m_operatingPackageMd5, "Authentication failed");
            bumpInstallIndex();                                     //开始安装下一个
        }
    }
    configWindow->hide();                        //隐藏配置窗口
    configWindow->clearTexts();                  //清楚配置信息
    m_procInstallConfig->terminate();                               //结束配置
    m_procInstallConfig->close();
}

/**
 * @brief DebListModel::ConfigReadOutput
 * 根据命令返回的输出数据，向界面添加数据展示
 */
void DebListModel::ConfigReadOutput()
{
    QString tmp = m_procInstallConfig->readAllStandardOutput().data();                  //获取配置读取到的信息

    //检查命令返回的结果，如果是 没有发现命令。直接报错，安装失败
    checkInstallStatus(tmp);

    tmp.remove(QChar('"'), Qt::CaseInsensitive);
    tmp.remove(QChar('\n'), Qt::CaseInsensitive);

    if (tmp.contains("StartInstallAptConfig")) {                                        //获取到当前正在安装配置
        emit onStartInstall();
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
    emit appendOutputInfo(appendInfoStr);                                               //将信息同步显示到安装信息中
    if (tmp.contains("Not authorized")) {
        configWindow->close();                                       //没有授权，关闭配置窗口
    } else {
        configWindow->appendTextEdit(tmp);                           //授权成功，继续配置
    }
}

/**
 * @brief DebListModel::ConfigInputWrite
 * @param str 输入的数据
 * 向命令传递输入的数据
 */
void DebListModel::ConfigInputWrite(QString str)
{
    m_procInstallConfig->write(str.toUtf8());                                          //将用户输入的配置项写入到配置安装进程中。
    m_procInstallConfig->write("\n");                                                  //写入换行，配置生效
}

/**
 * @brief DebListModel::checkInstallStatus  根据命令返回的消息判断安装状态
 * @param str 命令返回的安装信息
 * 如果命令返回的信息是Cannot run program deepin-deb-installer-dependsInstall: No such file or directory
 * 意味着当前/usr/bin下没有deepin-deb-installer-dependsInstall命令，此版本有问题，需要重新安装deepin-deb-installer-dependsInstall命令
 */
void DebListModel::checkInstallStatus(QString str)
{
    // 判断当前的信息是否是错误提示信息
    if (str.contains("Cannot run program deepin-deb-installer-dependsInstall: No such file or directory")) {
        emit appendOutputInfo(str);                                 //输出安装错误的原因
        m_workerStatus = WorkerFinished;                            //刷新包安装器的工作状态
        m_workerStatus_temp = m_workerStatus;
        refreshOperatingPackageStatus(Failed);                      //刷新当前包的操作状态

        // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
        m_packageOperateStatus[m_operatingPackageMd5] = Failed;
        m_packageFailCode.insert(m_operatingPackageMd5, 0);       //保存失败原因
        m_packageFailReason.insert(m_operatingPackageMd5, "");
        bumpInstallIndex();
        return;
    }
}

/**
 * @brief DebListModel::getPackageMd5 获取当前正在处理的包的md5值
 * @return 包的md5值
 */
void DebListModel::getPackageMd5(QList<QByteArray> md5)
{
    m_packageMd5.clear();
    m_packageMd5 = md5;
    emit appendFinished();
}


DebListModel::~DebListModel()
{
    delete m_packagesManager;
    delete configWindow;
    delete m_procInstallConfig;
}
