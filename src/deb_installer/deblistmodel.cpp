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
#include "packagesmanager.h"
#include "AptConfigMessage.h"
#include "utils.h"

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
    const QString output = proc.readAllStandardOutput();

    // 查看进程信息中是否存在dpkg 存在说明已经正在安装其他包
    for (const auto &item : output.split('\n'))
        if (item == "dpkg")
            return true;

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
    // 安装配置包时，没有得到授权
    if (errorCode == ConfigAuthCancel) {
        return QApplication::translate("DebListModel",
                                       "Authentication failed");
    }
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

    // 配置安装结束
    connect(m_procInstallConfig, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this, &DebListModel::ConfigInstallFinish);

    // 配置安装的过程数据
    connect(m_procInstallConfig, &QProcess::readyReadStandardOutput, this, &DebListModel::ConfigReadOutput);

    // 向安装进程中写入配置信息（一般是配置的序号）
    connect(AptConfigMessage::getInstance(), &AptConfigMessage::AptConfigInputStr, this, &DebListModel::ConfigInputWrite);

    //处理wine依赖安装的过程
    connect(m_packagesManager, &PackagesManager::DependResult, this, &DebListModel::DealDependResult);

    //安装wine依赖的时候不允许程序退出
    connect(m_packagesManager, &PackagesManager::enableCloseButton, this, &DebListModel::enableCloseButton);
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
        m_packageOperateStatus[iIndex] = Prepare;           //取消授权后，缺失wine依赖的包的操作状态修改为prepare
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
        return m_packagesManager->packageDependsStatus(r).status;   //获取当前index包的依赖状态
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
    case PackageOperateStatusRole:
        if (m_packageOperateStatus.contains(r))                     //获取当前包的操作状态
            return m_packageOperateStatus[r];
        else
            return Prepare;
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

    Q_ASSERT_X(m_workerStatus == WorkerPrepare, Q_FUNC_INFO, "installer status error");
    if (m_workerStatus != WorkerPrepare) return;

    m_workerStatus = WorkerProcessing;                                  //刷新包安装器的工作状态
    m_workerStatus_temp = m_workerStatus;
    m_operatingIndex = 0;                                               //初始化当前操作的index
    m_operatingStatusIndex = 0;
    m_InitRowStatus = false;                                            //当前未初始化每个包的操作状态
    // start first

    qDebug() << "size:" << m_packagesManager->m_preparedPackages.size();
    initRowStatus();                                                    //初始化包的操作状态
    installNextDeb();                                                   //开始安装
}

/**
 * @brief DebListModel::refreshAllDependsStatus 刷新获取所有依赖的状态
 */
void DebListModel::refreshAllDependsStatus()
{
    // 遍历获取所有依赖的状态
    for (int i = 0; i < preparedPackages().size(); i++) {
        m_packagesManager->packageDependsStatus(i);
    }
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
    // fix bug : 卸载失败时不提示卸载失败。
    m_operatingStatusIndex = idx;                       //刷新操作状态的index

    DebFile *deb = new DebFile(m_packagesManager->package(m_operatingIndex));   //获取到包

    const QStringList rdepends = m_packagesManager->packageReverseDependsList(deb->packageName(), deb->architecture());     //检查是否有应用依赖到该包
    Backend *b = m_packagesManager->m_backendFuture.result();
    for (const auto &r : rdepends) {                                        // 卸载所有依赖该包的应用（二者的依赖关系为depends）
        if (b->package(r))
            b->markPackageForRemoval(r);
        else
            qDebug() << "rDepend" << r << "package error ,please check it!";
    }
    b->markPackageForRemoval(deb->packageName() + ':' + deb->architecture());       //卸载当前包

    // uninstall
    qDebug() << Q_FUNC_INFO << "starting to remove package: " << deb->packageName() << rdepends;

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
 * @brief DebListModel::initDependsStatus  初始化index包依赖的状态
 * @param index 当前包的index
 * 此处逻辑有问题 稍后修改 0927
 */
void DebListModel::initDependsStatus(int index)
{
    const int packageCount = this->preparedPackages().size();           //获取当前包的数量
    if (index >= packageCount)                                          //边界检查
        return;
    for (int num = index; num < packageCount; num++)                    //初始化所有包的依赖
        this->index(num).data(DebListModel::PackageDependsStatusRole);
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
    const int packageCount = this->preparedPackages().size();           //获取prepare列表中包的数量
    QList<int> listdependInstallMark;                                   //保存wine依赖的下表
    for (int num = 0; num < packageCount; num++) {
        int dependsStatus = this->index(num).data(DebListModel::PackageDependsStatusRole).toInt();      //判断依赖状态
        if (dependsStatus != DependsOk) {                                                               //如果是wine依赖不满足，则标记
            QString failStr = this->index(num).data(DebListModel::PackageFailReasonRole).toString();
            if (failStr.contains("deepin-wine"))
                listdependInstallMark.append(num);                                                      //将wine依赖不满足的包的下标放到标记列表中
        }
    }
    qDebug() << "operate Status" << m_packageOperateStatus;

    // 去除操作状态 中的index
    // 由于map 中没有序号的区别，因此，需要对idx之后的需要-1来调整 operateIndex
    // 删除的时候 package的状态一定是prepare 其实可以全部删除之后再次重新添加。
    if (m_packageOperateStatus.size() > 1) {
        QMapIterator<int, int> MapIteratorOperateStatus(m_packageOperateStatus);            //QMap指针
        QMap<int, int> tmpOperateStatus;                                                    //临时列表
        while (MapIteratorOperateStatus.hasNext()) {
            MapIteratorOperateStatus.next();
            // 删除operateStatus中的index
            if (idx > MapIteratorOperateStatus.key())                                               // idx之前的下标不动
                tmpOperateStatus[MapIteratorOperateStatus.key()] = MapIteratorOperateStatus.value();
            else if (idx != MapIteratorOperateStatus.key()) {
                tmpOperateStatus[MapIteratorOperateStatus.key() - 1] = MapIteratorOperateStatus.value();    //之后的下标全部-1
            }
        }
        m_packageOperateStatus = tmpOperateStatus;
    }
    qDebug() << "operate Status after remove" << m_packageOperateStatus;

    m_packagesManager->removePackage(idx, listdependInstallMark);       //在packageManager中删除标记的下标
}

/**
 * @brief DebListModel::appendPackage 向安装器内部添加包
 * @param package   包的文件路径
 * @return 是否安装成功
 */
bool DebListModel::appendPackage(QString package)
{
    Q_ASSERT_X(m_workerStatus == WorkerPrepare, Q_FUNC_INFO, "installer status error");
    return m_packagesManager->appendPackage(package);      //添加包，并返回添加结果
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
    m_packageOperateStatus[m_operatingStatusIndex] = Failed;

    //记录失败代码与失败原因
    m_packageFailCode[m_operatingIndex] = trans->error();
    m_packageFailReason[m_operatingIndex] = trans->errorString();
    //fix bug: 点击重新后，授权码输入框弹出时反复取消输入，进度条已显示进度
    //取消安装后，Errorinfo被输出造成进度条进度不为0，现屏蔽取消授权错误。
    //授权错误不再输出到详细信息中
    if (!trans->errorString().contains("proper authorization was not provided"))
        emit appendOutputInfo(trans->errorString());

    const QApt::ErrorCode e = trans->error();               //trans错误的代码
    Q_ASSERT(e);

    qWarning() << Q_FUNC_INFO << e << workerErrorString(e, trans->errorString());
    qWarning() << trans->errorDetails() << trans->errorString();

    if (trans->isCancellable()) trans->cancel();

    //特殊处理授权错误
    if (e == AuthError) {
        trans->deleteLater();                                                       //删除 trans指针
        QTimer::singleShot(100 * 1, this, &DebListModel::checkBoxStatus);           //检查授权弹窗的状态 如果弹窗仍然在只是超时，则底层窗口按钮不可用
        qDebug() << "reset env to prepare";

        // reset env
        emit AuthCancel();                                                          //发送授权被取消的信号
        emit lockForAuth(false);                                                    //取消授权锁定，设置按钮可用
        //EnableReCancelBtn在信号在checkBoxStatus已发送，修改为enableCloseButton
        emit enableCloseButton(true);
        m_workerStatus = WorkerPrepare;                                             // 重置工作状态为准备态
        m_workerStatus_temp = m_workerStatus;
        return;
    }

    // DO NOT install next, this action will finished and will be install next automatic.
    trans->setProperty("exitStatus", QApt::ExitFailed);                             //设置trans的退出状态为 失败
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
    Q_ASSERT_X(m_currentTransaction.isNull(), Q_FUNC_INFO, "previous transaction not finished");

    // install finished
    qDebug() << "m_packageFailCode.size:" << m_packageFailCode.size();
    qDebug() << m_packageFailCode;

    qDebug() << "m_packageOperateStatus:" << m_packageOperateStatus;

    if (++m_operatingIndex == m_packagesManager->m_preparedPackages.size()) {
        qDebug() << "congratulations, install finished !!!";
        m_workerStatus = WorkerFinished;                                        //设置包安装器的工作状态为Finish
        m_workerStatus_temp = m_workerStatus;
        emit workerFinished();                                                  //发送安装完成信号
        emit workerProgressChanged(100);                                        //修改安装进度
        emit transactionProgressChanged(100);

        qDebug() << "m_packageDependsStatus,size" << m_packagesManager->m_packageDependsStatus.size();
        for (int i = 0; i < m_packagesManager->m_packageDependsStatus.size(); i++) {
            qDebug() << "m_packageDependsStatus[" << i << "] = " << m_packagesManager->m_packageDependsStatus[i].status;
        }
        return;
    }
    qDebug() << "m_packageDependsStatus,size" << m_packagesManager->m_packageDependsStatus.size();
    for (int i = 0; i < m_packagesManager->m_packageDependsStatus.size(); i++) {
        qDebug() << "m_packageDependsStatus[" << i << "] = " << m_packagesManager->m_packageDependsStatus[i].status;
    }
    qDebug() << "m_packagesManager->m_preparedPackages.size()" << m_packagesManager->m_preparedPackages.size();
    qDebug() << "m_operatingIndex" << m_operatingIndex;
    qDebug() << "m_packageFailCode.size:" << m_packageFailCode.size();
    qDebug() << m_packageFailCode;

    ++ m_operatingStatusIndex;
    emit onChangeOperateIndex(m_operatingIndex);                                //修改当前操作的下标
    // install next
    installNextDeb();                                                           //安装下一个包
}

/**
 * @brief DebListModel::refreshOperatingPackageStatus 刷新当前正在操作的包的状态
 * @param stat 要刷新的状态
 */
void DebListModel::refreshOperatingPackageStatus(const DebListModel::PackageOperationStatus stat)
{
    m_packageOperateStatus[m_operatingStatusIndex] = stat;  //将失败包的索引和状态修改保存,用于更新

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
    const auto stat = m_packagesManager->packageDependsStatus(idx);                         //获取包的依赖状态
    if (m_packagesManager->isArchError(idx)) return tr("Unmatched package architecture");   //判断是否架构冲突
    if (stat.isBreak() || stat.isAuthCancel()) {                                            //依赖状态错误
        if (!stat.package.isEmpty()) {
            if (m_packagesManager->m_errorIndex.contains(idx))
                return tr("Failed to install %1").arg(stat.package);                        //wine依赖安装失败
            return tr("Broken dependencies: %1").arg(stat.package);                         //依赖不满足
        }

        const auto conflict = m_packagesManager->packageConflictStat(idx);                  //获取冲突情况
        if (!conflict.is_ok()) return tr("Broken dependencies: %1").arg(conflict.unwrap()); //依赖冲突
        Q_UNREACHABLE();
    }
    Q_ASSERT(m_packageOperateStatus.contains(idx));
    Q_ASSERT(m_packageOperateStatus[idx] == Failed);
    if (!m_packageFailCode.contains(idx))
        qDebug() << "ggy" << m_packageFailCode.size() << idx;
    Q_ASSERT(m_packageFailCode.contains(idx));

    return workerErrorString(m_packageFailCode[idx], m_packageFailReason[idx]);             //根据错误代码和错误原因返回具体的错误原因
}

/**
 * @brief DebListModel::onTransactionFinished transaction结束的槽函数
 *
 */
void DebListModel::onTransactionFinished()
{
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");
    Transaction *trans = static_cast<Transaction *>(sender());                              // 获取trans指针

    // prevent next signal
    disconnect(trans, &Transaction::finished, this, &DebListModel::onTransactionFinished);  //不再接收trans结束的信号

    // report new progress
    //更新安装进度（批量安装进度控制）
    int progressValue = static_cast<int>(100. * (m_operatingIndex + 1) / m_packagesManager->m_preparedPackages.size());
    emit workerProgressChanged(progressValue);

    qDebug() << "tans.exitStatus()" << trans->exitStatus();
    if (trans->exitStatus()) {
        //安装失败
        qWarning() << trans->error() << trans->errorDetails() << trans->errorString();
        //保存错误原因和错误代码
        m_packageFailCode[m_operatingStatusIndex] = trans->error();
        m_packageFailReason[m_operatingStatusIndex] = trans->errorString();

        //刷新操作状态
        refreshOperatingPackageStatus(Failed);
        emit appendOutputInfo(trans->errorString());
    } else if (m_packageOperateStatus.contains(m_operatingStatusIndex) &&
               m_packageOperateStatus[m_operatingStatusIndex] != Failed) {
        //安装成功
        refreshOperatingPackageStatus(Success);

        //准备安装下一个包，修改下一个包的状态为正在安装状态
        if (m_operatingStatusIndex < m_packagesManager->m_preparedPackages.size() - 1) {
            m_packageOperateStatus[m_operatingStatusIndex + 1] = Waiting;
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
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");
    Transaction *trans = static_cast<Transaction *>(sender());                                  //获取transaction指针

    const auto ret = trans->exitStatus();

    if (ret) qWarning() << trans->error() << trans->errorDetails() << trans->errorString();     //transaction发生错误

    if (ret) {
        // record error
        // 记录错误原因和错误代码
        m_packageFailCode[m_operatingStatusIndex] = trans->error();
        m_packageFailReason[m_operatingStatusIndex] = trans->errorString();
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
        qDebug() << "Transaction is Nullptr";
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
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");
    Q_ASSERT_X(m_currentTransaction.isNull(), Q_FUNC_INFO, "previous transaction not finished");

    //在判断dpkg启动之前就发送开始安装的信号，并在安装信息中输出 dpkg正在运行的信息。
    emit onStartInstall();
    if (isDpkgRunning()) {
        qDebug() << "dpkg running, waitting...";
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
    const auto dependsStat = m_packagesManager->packageDependsStatus(m_operatingStatusIndex);
    if (dependsStat.isBreak() || dependsStat.isAuthCancel()) {          //依赖不满足或者下载wine依赖时授权被取消
        refreshOperatingPackageStatus(Failed);                          //刷新错误状态
        m_packageFailCode.insert(m_operatingStatusIndex, -1);           //保存错误原因
        bumpInstallIndex();                                             //开始下一步的安装流程
        return;
    } else if (dependsStat.isAvailable()) {
        // 依赖可用 但是需要下载
        Q_ASSERT_X(m_packageOperateStatus[m_operatingStatusIndex], Q_FUNC_INFO,
                   "package operate status error when start install availble dependencies");

        // 获取到所有的依赖包 准备安装
        const QStringList availableDepends = m_packagesManager->packageAvailableDepends(m_operatingIndex);
        //        for (auto const &p : availableDepends) backend->markPackageForInstall(p);
        //获取到可用的依赖包并根据后端返回的结果判断依赖包的安装结果
        for (auto const &p : availableDepends) {
            if (p.contains(" not found")) {                             //依赖安装失败
                refreshOperatingPackageStatus(Failed);                  //刷新当前包的状态
                m_packageFailCode.insert(m_operatingStatusIndex, DownloadDisallowedError);                       //记录错误代码与错误原因
                m_packageFailReason.insert(m_operatingStatusIndex, p);
                emit appendOutputInfo(m_packagesManager->package(m_operatingIndex) + "\'s depend " + " " + p);  //输出错误原因
                bumpInstallIndex();                                     //开始安装下一个包或结束安装
                return;
            }
            backend->markPackageForInstall(p);                          //开始安装依赖包
        }

        //安装
        qDebug() << Q_FUNC_INFO << "install" << deb.packageName() << "dependencies: " << availableDepends;

        trans = backend->commitChanges();
        //依赖安装结果处理
        connect(trans, &Transaction::finished, this, &DebListModel::onDependsInstallTransactionFinished);
    } else {
        //开始安装当前包
        qDebug() << Q_FUNC_INFO << "starting to install package: " << deb.packageName();

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
    m_currentTransaction->run();
}

/**
 * @brief DebListModel::showNoDigitalErrWindow 没有有效的数字签名时弹出对应的错误窗口
 */
void DebListModel::showNoDigitalErrWindow()
{
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
    // 点击弹出窗口的关闭按钮
    connect(Ddialog, &DDialog::aboutToClose, this, [ = ] {
        if (preparedPackages().size() > 1)                          //批量安装
        {
            refreshOperatingPackageStatus(VerifyFailed);            //刷新操作状态
            bumpInstallIndex();                                     //跳过当前包
            return;
        } else if (preparedPackages().size() == 1)
        {
            exit(0);                                                //单包安装 直接退出
        }
    });

    //点击弹出窗口的确定按钮
    connect(btnOK, &DPushButton::clicked, this, [ = ] {
        qDebug() << "result:" << btnOK->isChecked();
        if (preparedPackages().size() > 1)                          //批量安装
        {
            refreshOperatingPackageStatus(VerifyFailed);            //刷新操作状态
            bumpInstallIndex();                                     //跳过当前包
            return;
        } else if (preparedPackages().size() == 1)                  //单包安装
        {
            exit(0);                                                //直接退出
        }
    });
}

/**
 * @brief DebListModel::checkSystemVersion 检查当前操作系统的版本，判断是否需要验证签名
 * @return 是否需要验证签名
 *
 * 个人版专业版 非开模式需要验证签名
 * 服务器版 没有开发者模式，默认不验证签名
 * 社区版默认开发者模式，不验证签名。
 * 此部分修改，无法影响control依赖，服务器版与社区版需要在Control文件中去除deepin-elf-sign-tool依赖
 * 已经有更新的接口，稍后需要更新
 */
bool DebListModel::checkSystemVersion()
{
    // add for judge OS Version
    bool isVerifyDigital = false;                           //默认不需要验证数字签名
    switch (Dtk::Core::DSysInfo::deepinType()) {            //获取系统的类型
    case Dtk::Core::DSysInfo::DeepinDesktop:                //社区版 不验证签名
        isVerifyDigital = false;
        break;
    case Dtk::Core::DSysInfo::DeepinPersonal:               //个人版
    case Dtk::Core::DSysInfo::DeepinProfessional:           //专业版
        isVerifyDigital = true;
        break;
    case Dtk::Core::DSysInfo::DeepinServer:                 //服务器版
        isVerifyDigital = false;
        break;
    default:
        isVerifyDigital = true;
    }

    qDebug() << "DeepinType:" << Dtk::Core::DSysInfo::deepinType();
    qDebug() << "Whether to verify the digital signature：" << isVerifyDigital;
    return isVerifyDigital;
}

/**
 * @brief DebListModel::checkDigitalSignature 验证开发者模式与数字签名
 * @return 是否能够安装当前包
 * 如果当前处于开发者模式或处于非开发者模式但有数字签名 则验证成功
 * 如果处于非开模式且无数字签名。则验证失败
 */
bool DebListModel::checkDigitalSignature()
{
    QDBusInterface Installer("com.deepin.deepinid", "/com/deepin/deepinid", "com.deepin.deepinid");
    bool deviceMode = Installer.property("DeviceUnlocked").toBool();                            // 判断当前是否处于开发者模式
    qDebug() << "QDBusResult" << deviceMode;
    if (deviceMode)                                                                             //处于开发者模式，直接返回验证成功
        return true;
    int digitalSigntual = Utils::Digital_Verify(m_packagesManager->package(m_operatingIndex)); //非开模式，判断是否有数字签名
    switch (digitalSigntual) {
    case Utils::VerifySuccess:                                                                  //签名验证成功
        return true;
    case Utils::DebfileInexistence:
    case Utils::ExtractDebFail:
    case Utils::DebVerifyFail:
    case Utils::OtherError:
        return false;
    default:
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
    if (checkSystemVersion() && !checkDigitalSignature()) {     //非开发者模式且数字签名验证失败
        showNoDigitalErrWindow();                               //演出错误窗口
    } else {
        QString sPackageName = m_packagesManager->m_preparedPackages[m_operatingIndex];
        QStringList strFilePath;
        qDebug() << sPackageName;
        if (checkTemplate(sPackageName)) {                      //检查当前包是否需要配置
            rmdir();                                            //删除临时路径

            if (!m_procInstallConfig->isOpen()) {               //启动配置包的安装 此处逻辑可以优化
                qDebug() << "pkexec install" << sPackageName;
                m_procInstallConfig->start("pkexec", QStringList() << "deepin-deb-installer-dependsInstall" << "InstallConfig" << sPackageName);
            } else {
                qDebug() << "pkexec install again" << sPackageName;
                m_procInstallConfig->start("pkexec", QStringList() << "deepin-deb-installer-dependsInstall" << "InstallConfig" << sPackageName);
            }
        } else {
            qDebug() << "normal install" << sPackageName;
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
            qDebug() << "remove success";
        } else {
            qDebug() << "remove failed";
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
    qDebug() << tempPath + "/templates";
    if (templates.exists()) {
        qDebug() << "exists";
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
    QProcess *m_pDpkg = new QProcess;

    if (!mkdir()) {                                                             //创建临时路径
        qWarning() << "check error mkdir" << tempPath << "failed";              //创建失败
        return;
    }
    qDebug() << "dpkg" << "-e" << debPath << tempPath;
    m_pDpkg->start("dpkg", QStringList() << "-e" << debPath << tempPath);       //获取DEBIAN文件，查看当前包是否需要配置
    m_pDpkg->waitForFinished();
    qDebug() << "dpkg StandardOutput" << m_pDpkg->readAllStandardOutput();
    qDebug() << "dpkg StandardError" << m_pDpkg->readAllStandardError();
}

/**
 * @brief DebListModel::onTransactionOutput 刷新当前包的操作状态
 */
void DebListModel::onTransactionOutput()
{
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");
    Transaction *trans = static_cast<Transaction *>(sender());
    Q_ASSERT(trans == m_currentTransaction.data());
    qDebug() << "local:" << m_currentTransaction->locale();

    refreshOperatingPackageStatus(Operating);                       //刷新当前包的操作状态

    disconnect(trans, &Transaction::statusDetailsChanged, this, &DebListModel::onTransactionOutput);
}

/**
 * @brief DebListModel::uninstallFinished 卸载包结束
 */
void DebListModel::uninstallFinished()
{
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");

    qDebug() << Q_FUNC_INFO;

    //增加卸载失败的情况
    //此前的做法是发出commitError的信号，现在全部在Finished中进行处理。不再特殊处理。
    Transaction *trans = static_cast<Transaction *>(sender());
    qDebug() << Q_FUNC_INFO << "trans.error()" << trans->error() << "trans.errorString" << trans->errorString();
    if (trans->exitStatus()) {
        m_workerStatus = WorkerFinished;                            //刷新包安装器的工作状态
        m_workerStatus_temp = m_workerStatus;
        refreshOperatingPackageStatus(Failed);                      //刷新当前包的操作状态
        m_packageOperateStatus[m_operatingIndex] = Failed;
    } else {
        m_workerStatus = WorkerFinished;                            //刷新包安装器的工作状态
        m_workerStatus_temp = m_workerStatus;
        refreshOperatingPackageStatus(Success);                     //刷新当前包的卸载状态
        m_packageOperateStatus[m_operatingIndex] = Success;
    }
    emit workerFinished();                                          //发送结束信号（只有单包卸载）卸载结束就是整个流程的结束
    trans->deleteLater();
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
    qDebug() << "m_packageOperateStatus" << m_packageOperateStatus;
    for (int i = 0; i < m_packagesManager->m_preparedPackages.size(); i++) {
        m_packageOperateStatus.insert(i, Prepare);                          //刷新当前所有包的状态为Prepare
    }
    qDebug() << "after m_packageOperateStatus" << m_packageOperateStatus;

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

    QList<int> listWrongIndex;
    int iIndex = 0;

    //find wrong index
    // 找到操作错误的包的下标
    QMapIterator<int, int> iteratorpackageOperateStatus(m_packageOperateStatus);
    QList<int> listpackageOperateStatus;
    while (iteratorpackageOperateStatus.hasNext()) {
        iteratorpackageOperateStatus.next();
        if (iteratorpackageOperateStatus.value() == Failed || iteratorpackageOperateStatus.value() == VerifyFailed) {   //安装失败或签名验证失败
            listWrongIndex.insert(iIndex, iteratorpackageOperateStatus.key());                                          //保存下标
            listpackageOperateStatus.insert(iIndex++, iteratorpackageOperateStatus.value());                            //保存状态
        } else if (iteratorpackageOperateStatus.value() == Success)                                                     //安装成功的只保存状态
            listpackageOperateStatus.append(iteratorpackageOperateStatus.value());
        else
            return;
    }

    if (listWrongIndex.size() == 0)       //全部安装成功 直接退出
        return;

    QList<int> t_errorIndex;                                                                //临时变量，保存安装wine依赖错误的下标
    //change  m_preparedPackages, m_packageOperateStatus sort.
    QList<QString> listTempDebFile;
    QList<QByteArray> listTempPreparedMd5;
    iIndex = 0;
    for (int i = 0; i < m_packagesManager->m_preparedPackages.size(); i++) {
        m_packageOperateStatus[i] = listpackageOperateStatus[i];
        if (listWrongIndex.contains(i)) {
            t_errorIndex.push_back(iIndex);                                                 //保存错误的下标
            //保存安装失败的文件路径
            //注意 此处用的是insert 到iIndex位置，也就是说 安装失败的包会被放到前面
            listTempDebFile.insert(iIndex, m_packagesManager->m_preparedPackages[i]);
            //保存安装失败的MD5的值
            //注意 此处用的是insert 到iIndex位置，也就是说 安装失败的包会被放到前面
            //此处iIndex++ 则是将index +1 开始对下一个包进行排序
            listTempPreparedMd5.insert(iIndex++, m_packagesManager->m_preparedMd5[i]);
        } else {
            // 所有安装成功的包会被append到后面
            listTempDebFile.append(m_packagesManager->m_preparedPackages[i]);               //保存安装成功的文件路径
            listTempPreparedMd5.append(m_packagesManager->m_preparedMd5[i]);                //保存安装成功的md5的值
        }
    }

    //保存排序后的prepaed 和md5
    m_packagesManager->m_preparedPackages = listTempDebFile;
    m_packagesManager->m_preparedMd5 = listTempPreparedMd5;

    //Reupdates the index that failed to install Deepin-Wine
    if (m_packagesManager->m_errorIndex.size() > 0) {
        m_packagesManager->m_errorIndex.clear();
        m_packagesManager->m_errorIndex = t_errorIndex;
    }


    //change  m_packageFailCode sort.
    QMap<int, int> mappackageFailReason;
    QMapIterator<int, int> IteratorpackageFailReason(m_packageFailCode);
    while (IteratorpackageFailReason.hasNext()) {
        IteratorpackageFailReason.next();
        // 获取安装错误失败代码的下标和错误值
        int iIndexTemp = listWrongIndex.indexOf(IteratorpackageFailReason.key());
        mappackageFailReason[iIndexTemp] = IteratorpackageFailReason.value();
    }
    m_packageFailCode.clear();
    m_packageFailCode = mappackageFailReason;

    //change  m_packageInstallStatus sort.
    QMapIterator<int, int> MapIteratorpackageInstallStatus(m_packagesManager->m_packageInstallStatus);
    QList<int> listpackageInstallStatus;
    iIndex = 0;
    while (MapIteratorpackageInstallStatus.hasNext()) {
        MapIteratorpackageInstallStatus.next();
        //对安装状态进行排序
        //注意 此处用的是insert 到iIndex位置，也就是说 安装失败的包会被放到前面
        if (listWrongIndex.contains(MapIteratorpackageInstallStatus.key()))

            listpackageInstallStatus.insert(iIndex++, MapIteratorpackageInstallStatus.value());

        //所有安装成功的包会被append到后面
        else
            listpackageInstallStatus.append(MapIteratorpackageInstallStatus.value());
    }

    //保存所有的安装状态
    for (int i = 0; i < listpackageInstallStatus.size(); i++)
        m_packagesManager->m_packageInstallStatus[i] = listpackageInstallStatus[i];

    //change  m_packageDependsStatus sort.
    //对依赖状态进行排序
    QMapIterator<int, PackagesManagerDependsStatus::PackageDependsStatus> MapIteratorpackageDependsStatus(m_packagesManager->m_packageDependsStatus);
    QList<PackagesManagerDependsStatus::PackageDependsStatus> listpackageDependsStatus;
    iIndex = 0;
    while (MapIteratorpackageDependsStatus.hasNext()) {
        MapIteratorpackageDependsStatus.next();
        // 对安装失败和安装成功的包分别排序
        if (listWrongIndex.contains(MapIteratorpackageDependsStatus.key()))
            listpackageDependsStatus.insert(iIndex++, MapIteratorpackageDependsStatus.value());
        else
            listpackageDependsStatus.append(MapIteratorpackageDependsStatus.value());
    }
    //保存所有包的安装状态
    for (int i = 0; i < listpackageDependsStatus.size(); i++)
        m_packagesManager->m_packageDependsStatus[i] = listpackageDependsStatus[i];

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
    qDebug() << "config install result:" << flag;
    if (flag == 0) {        //安装成功
        if (m_packagesManager->m_packageDependsStatus[m_operatingIndex].status == DependsOk) {
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
            m_packageFailCode.insert(m_operatingIndex, flag);       //保存失败原因
            m_packageFailReason.insert(m_operatingIndex, "Authentication failed");
            bumpInstallIndex();                                     //开始安装下一个
        }
    }
    AptConfigMessage::getInstance()->hide();                        //隐藏配置窗口
    AptConfigMessage::getInstance()->clearTexts();                  //清楚配置信息
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
    tmp.remove(QChar('"'), Qt::CaseInsensitive);
    tmp.remove(QChar('\n'), Qt::CaseInsensitive);

    if (tmp.contains("StartInstallAptConfig")) {                                        //获取到当前正在安装配置
        emit onStartInstall();
        refreshOperatingPackageStatus(Operating);                                       //刷新当前的操作状态
        AptConfigMessage::getInstance()->show();                                        //显示配置窗口
        QString startFlagStr = "StartInstallAptConfig";
        int num = tmp.indexOf(startFlagStr) + startFlagStr.size();
        int iCutoutNum = tmp.size() - num;
        if (iCutoutNum > 0)
            AptConfigMessage::getInstance()->appendTextEdit(tmp.mid(num, iCutoutNum));  //显示配置信息
        return;
    }

    QString appendInfoStr = tmp;
    appendInfoStr.remove(QChar('\"'), Qt::CaseInsensitive);
    appendInfoStr.remove(QChar('"'), Qt::CaseInsensitive);
    appendInfoStr.replace("\\n", "\n");
    appendInfoStr.replace("\n\n", "\n");
    emit appendOutputInfo(appendInfoStr);                                               //将信息同步显示到安装信息中
    if (tmp.contains("Not authorized")) {
        AptConfigMessage::getInstance()->close();                                       //没有授权，关闭配置窗口
    } else {
        AptConfigMessage::getInstance()->appendTextEdit(tmp);                           //授权成功，继续配置
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
