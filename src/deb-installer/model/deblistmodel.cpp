// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deblistmodel.h"
#include "manager/packagesmanager.h"
#include "utils/ddlog.h"
#include "manager/PackageDependsStatus.h"
#include "packageanalyzer.h"
#include "view/pages/AptConfigMessage.h"
#include "view/pages/settingdialog.h"
#include "utils/utils.h"
#include "utils/hierarchicalverify.h"
#include "singleInstallerApplication.h"
#include "view/widgets/error_notify_dialog_helper.h"
#include "compatible/compatible_backend.h"
#include "compatible/compatible_process_controller.h"
#include "immutable/immutable_backend.h"
#include "immutable/immutable_process_controller.h"
#include "utils/qtcompat.h"

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
    : AbstractPackageListModel(parent)
    , m_packagesManager(new PackagesManager(this))
{
    qCDebug(appLog) << "DebListModel initializing...";
    m_supportPackageType = Pkg::Deb;

    // 配置包安装的进程
    m_procInstallConfig = new Konsole::Pty;
    configWindow = new AptConfigMessage;

    // 链接信号与槽
    initConnections();
    // 检查系统版本与是否开启了开发者模式
    m_isDevelopMode = Utils::isDevelopMode();
    qCInfo(appLog) << "DebListModel initialized, develop mode:" << m_isDevelopMode;
}

bool DebListModel::isDpkgRunning()
{
    qCDebug(appLog) << "Checking if dpkg is running.";
    QProcess proc;

    // 获取当前的进程信息
    proc.start("ps",
               QStringList() << "-e"
                             << "-o"
                             << "comm");
    proc.waitForFinished();

    // 获取进程信息的数据
    const QString processOutput = proc.readAllStandardOutput();

    // 查看进程信息中是否存在dpkg 存在说明已经正在安装其他包
    if (processOutput.contains("dpkg")) {
        qCDebug(appLog) << "dpkg is running.";
        return true;  // 更换判断的方式
    }

    qCDebug(appLog) << "dpkg is not running.";
    return false;
}

const QStringList DebListModel::netErrors()
{
    qCDebug(appLog) << "Getting network error strings.";
    QStringList errorDetails;
    errorDetails << "Address family for hostname not supported";
    errorDetails << "Temporary failure resolving";
    errorDetails << "Network is unreachable";
    errorDetails << "Cannot initiate the connection to";
    return errorDetails;
}

const QString DebListModel::workerErrorString(const int errorCode, const QString &errorInfo)
{
    qCDebug(appLog) << "Getting worker error string for code:" << errorCode << "info:" << errorInfo;
    switch (errorCode) {
        case FetchError:
        case DownloadDisallowedError:
            qCDebug(appLog) << "Network error detected.";
            return QApplication::translate("DebListModel",
                                           "Installation failed, please check your network connection");  // 网络错误
        case NotFoundError:
             qCDebug(appLog) << "Package not found error.";
            return QApplication::translate("DebListModel", "Installation failed, please check for updates in Control Center");
        case DiskSpaceError:
            qCDebug(appLog) << "Disk space error.";
            return QApplication::translate("DebListModel", "Installation failed, insufficient disk space");  // 存储空间不足
        // lockError 也会报空间不足的错误
        case LockError:
            qCDebug(appLog) << "Lock error.";
            if (errorInfo.contains("No space left on device")) {
                qCDebug(appLog) << "Disk space error within LockError.";
                return QApplication::translate("DebListModel", "Installation failed, insufficient disk space");
            }
            break;
        // fix bug:39834 网络断开时，偶现安装deb包失败时提示语不显示
        case CommitError:
            qCDebug(appLog) << "Commit error.";
            // commit Error 中会报网络错误
            for (auto error : netErrors()) {
                if (errorInfo.contains(error) && errorInfo.contains("http")) {
                    qCDebug(appLog) << "Network error within CommitError.";
                    return QApplication::translate("DebListModel", "Installation failed, please check your network connection");
                }
            }
            // commitError 有时会出现空间不足的错误
            if (errorInfo.contains("No space left on device")) {
                 qCDebug(appLog) << "Disk space error within CommitError.";
                return QApplication::translate("DebListModel", "Installation failed, insufficient disk space");
            }
            break;
        // 无数字签名的错误
        case Pkg::NoDigitalSignature:
            qCDebug(appLog) << "No digital signature error.";
            return QApplication::translate("DebListModel", "No digital signature");

        // 无有效的数字签名
        case Pkg::DigitalSignatureError:
            qCDebug(appLog) << "Invalid digital signature error.";
            return QApplication::translate("DebListModel", "Invalid digital signature");

        // 安装配置包时，没有得到授权
        case Pkg::ConfigAuthCancel:
            qCDebug(appLog) << "Config auth cancel error.";
            return QApplication::translate("DebListModel", "Authentication failed");
        case Pkg::ApplocationProhibit:
            qCDebug(appLog) << "Application prohibit error.";
            return QApplication::translate("DebListModel",
                                           "The administrator has set policies to prevent installation of this package");
        default:
            qCDebug(appLog) << "Unknown error, returning generic failure message.";
            break;
    }
    // 其余错误，暂不提示具体的错误原因
    return QApplication::translate("DebListModel", "Installation Failed");
}
void DebListModel::initAppendConnection()
{
    qCDebug(appLog) << "Initializing append connections.";
    connect(m_packagesManager, &PackagesManager::signalAppendFailMessage, this, &DebListModel::signalAppendFailMessage);

    // 告诉前端当前处在添加过程中
    connect(m_packagesManager, &PackagesManager::signalAppendStart, this, &DebListModel::signalAppendStart);

    // 提示前端当前已经添加完成
    connect(m_packagesManager, &PackagesManager::signalAppendFinished, this, &DebListModel::getPackageMd5);

    // 当前由于文件路径被修改删除md5
    connect(m_packagesManager, &PackagesManager::signalPackageMd5Changed, this, &DebListModel::getPackageMd5);
}

/**
 * @brief DebListModel::initInstallConnecions 链接安装过程的信号与槽
 */
void DebListModel::initInstallConnections()
{
    qCDebug(appLog) << "Initializing install connections.";
    // 安装成功后，根据安装结果排序
    connect(this, &DebListModel::signalWorkerFinished, this, &DebListModel::slotUpWrongStatusRow);

    // 配置安装结束
    connect(m_procInstallConfig,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            &DebListModel::slotConfigInstallFinish);

    // 配置安装的过程数据
    connect(m_procInstallConfig, &Konsole::Pty::receivedData, this, &DebListModel::slotConfigReadOutput);

    // 向安装进程中写入配置信息（一般是配置的序号）
    connect(configWindow, &AptConfigMessage::AptConfigInputStr, this, &DebListModel::slotConfigInputWrite);

    // 处理wine依赖安装的过程
    connect(m_packagesManager, &PackagesManager::signalDependResult, this, &DebListModel::slotDealDependResult);

    // 安装wine依赖的时候不允许程序退出
    connect(m_packagesManager, &PackagesManager::signalEnableCloseButton, this, &DebListModel::signalEnableCloseButton);
}

/**
 * @brief DebListModel::initRefreshPageConnecions 链接刷新界面的信号与槽
 */
void DebListModel::initRefreshPageConnecions()
{
    qCDebug(appLog) << "Initializing refresh page connections.";
    // Refresh the current install page based on the packge count, single / multiple or choose file page.
    connect(m_packagesManager, &PackagesManager::signalPackageCountChanged, this, &DebListModel::signalPackageCountChanged);
}

/**
 * @brief DebListModel::initConnections 链接所有的信号与槽
 */
void DebListModel::initConnections()
{
    qCDebug(appLog) << "Initializing all connections.";
    // 链接添加时的信号与槽
    initAppendConnection();

    // 链接页面刷新的信号与槽
    initRefreshPageConnecions();

    // 链接安装过程的信号与槽
    initInstallConnections();
}

void DebListModel::slotDealDependResult(int authType, int dependIndex, const QString &dependName)
{
    qCDebug(appLog) << "Dealing with depend result. Auth type:" << authType << "Depend index:" << dependIndex << "Depend name:" << dependName;
    m_brokenDepend = dependName;
    switch (authType) {
        case DebListModel::CancelAuth:
            qCDebug(appLog) << "Auth canceled.";
            m_packageOperateStatus[m_packagesManager->getPackageMd5(dependIndex)] =
                Pkg::PackageOperationStatus::Prepare;  // 取消授权后，缺失wine依赖的包的操作状态修改为prepare
            break;
        case DebListModel::AuthConfirm:  // 确认授权后，状态的修改由debinstaller进行处理
            qCDebug(appLog) << "Auth confirmed.";
            break;
        case DebListModel::AuthDependsSuccess:  // 安装成功后，状态的修改由debinstaller进行处理
            qCDebug(appLog) << "Auth depends success.";
            m_packageOperateStatus[m_packagesManager->getPackageMd5(dependIndex)] = Pkg::PackageOperationStatus::Prepare;
            m_workerStatus = WorkerPrepare;
            break;
        case DebListModel::AuthDependsErr:  // 安装失败后，状态的修改由debinstaller进行处理
            qCDebug(appLog) << "Auth depends error.";
            break;
        case DebListModel::VerifyDependsErr:  // 依赖包分级管控验证签名失败，弹出分级设置提示框
            qCDebug(appLog) << "Verify depends error.";
            ErrorNotifyDialogHelper::showHierarchicalVerifyWindow();
            break;
        default:
            qCDebug(appLog) << "Unknown auth type.";
            break;
    }
    emit signalDependResult(authType, dependName);  // 发送信号，由debinstaller处理界面状态。
}

bool DebListModel::isReady() const
{
    bool ready = m_packagesManager->isBackendReady();
    qCDebug(appLog) << "Checking if model is ready:" << ready;
    return ready;
}

const QList<QString> DebListModel::preparedPackages() const
{
    qCDebug(appLog) << "Getting prepared packages.";
    return m_packagesManager->m_preparedPackages;
}

QModelIndex DebListModel::first() const
{
    qCDebug(appLog) << "Getting first index.";
    return index(0);
}

int DebListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    int count = m_packagesManager->m_preparedPackages.size();
    qCDebug(appLog) << "Getting row count:" << count;
    return count;
}

QVariant DebListModel::data(const QModelIndex &index, int role) const
{
    const int currentRow = index.row();
    // 判断当前下标是否越界
    if (currentRow < 0 || currentRow >= m_packagesManager->m_preparedPackages.size()) {
        qCDebug(appLog) << "Data request for invalid index:" << currentRow;
        return QVariant();
    }
    // 当前给出的路径文件已不可访问.直接删除该文件
    if (!recheckPackagePath(m_packagesManager->package(currentRow))) {
        qCDebug(appLog) << "Package path for index" << currentRow << "is no longer valid, removing package.";
        m_packagesManager->removePackage(currentRow);
        return QVariant();
    }

    const DebFile deb(m_packagesManager->package(currentRow));
    if (!deb.isValid()) {
        qCDebug(appLog) << "Deb file for index" << currentRow << "is invalid.";
        return QVariant();
    }

    QString packageName = deb.packageName();            // 包名
    QString filePath = deb.filePath();                  // 包的路径
    QString version = deb.version();                    // 包的版本
    QString architecture = deb.architecture();          // 包可用的架构
    QString shortDescription = deb.shortDescription();  // 包的短描述
    QString longDescription = deb.longDescription();    // 包的长描述 //删除该指针，以免内存泄露

    switch (role) {
        case WorkerIsPrepareRole:
            return isWorkerPrepare();  // 获取当前工作状态是否准备九局
        case ItemIsCurrentRole:
            return m_currentIdx == index;  // 获取当前的index
        case PackageNameRole:
            return packageName;  // 获取当前index包的包名
        case PackagePathRole:
            return filePath;  // 获取当前index包的路径
        case PackageVersionRole:
            return version;  // 获取当前index包的版本
        case PackageVersionStatusRole:
            return m_packagesManager->packageInstallStatus(currentRow);  // 获取当前index包的安装状态
        case PackageDependsStatusRole:
            return m_packagesManager->getPackageDependsStatus(currentRow).status;  // 获取当前index包的依赖状态
        case PackageInstalledVersionRole:
            return m_packagesManager->packageInstalledVersion(currentRow);  // 获取当前index包在系统中安装的版本
        case PackageAvailableDependsListRole:
            return m_packagesManager->packageAvailableDepends(currentRow);  // 获取当前index包可用的依赖
        case PackageReverseDependsListRole:
            return m_packagesManager->packageReverseDependsList(packageName, architecture);  // 获取依赖于当前index包的应用
        case PackageShortDescriptionRole:
            return Utils::fromSpecialEncoding(shortDescription);  // 获取当前index包的短描述
        case PackageLongDescriptionRole:
            return Utils::fromSpecialEncoding(longDescription);  // 获取当前index包的长描述
        case PackageFailReasonRole:
            return packageFailedReason(currentRow);  // 获取当前index包的安装失败的原因
        case PackageOperateStatusRole: {
            auto md5 = m_packagesManager->getPackageMd5(currentRow);
            if (m_packageOperateStatus.contains(md5))  // 获取当前包的操作状态
                return m_packageOperateStatus[md5];
            else
                return Pkg::PackageOperationStatus::Prepare;
        }
        case PackageTypeRole:
            return Pkg::Deb;
        case PackageDependsDetailRole:
            return QVariant::fromValue(m_packagesManager->getPackageDependsDetail(currentRow));
        case CompatibleRootfsRole:
            if (auto pkgPtr = packagePtr(currentRow)) {
                return pkgPtr->compatible()->rootfs;
            }
            break;
        case CompatibleTargetRootfsRole:
            if (auto pkgPtr = packagePtr(currentRow)) {
                return pkgPtr->compatible()->targetRootfs;
            }
            break;
        case PackageRemoveDependsRole: {
            const QByteArray md5 = m_packagesManager->getPackageMd5(currentRow);
            return m_packagesManager->removePackages(md5);
        }

        case PackageSharedPointerRole: {
            return QVariant::fromValue(packagePtr(currentRow));
        }

        case Qt::SizeHintRole:  // 设置当前index的大小
            return QSize(0, 48);
        case Qt::ToolTipRole:
            return itemToolTips(currentRow);
        default:
            break;
    }

    return QVariant();
}

bool DebListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    // qCDebug(appLog) << "Setting data for index" << index.row() << "role" << role;
    const int currentRow = index.row();
    // 判断当前下标是否越界
    if (currentRow < 0 || currentRow >= m_packagesManager->m_preparedPackages.size()) {
        qCDebug(appLog) << "Index out of bounds, returning false.";
        return false;
    }

    switch (role) {
        case CompatibleTargetRootfsRole: {
            if (auto pkgPtr = packagePtr(currentRow)) {
                // qCDebug(appLog) << "Setting compatible target rootfs to" << value.toString();
                pkgPtr->compatible()->targetRootfs = value.toString();
                return true;
            }
            break;
        }
        default:
            // qCDebug(appLog) << "Unhandled role in setData.";
            break;
    }

    // qCDebug(appLog) << "setData returning false.";
    return false;
}

bool DebListModel::isDevelopMode()
{
    qCDebug(appLog) << "Getting develop mode status:" << m_isDevelopMode;
    return m_isDevelopMode;
}

bool DebListModel::slotInstallPackages()
{
    qCDebug(appLog) << "Entering slotInstallPackages.";
    if (m_workerStatus != WorkerPrepare) {
        qCWarning(appLog) << "Cannot start installation - worker status is" << m_workerStatus << "expected Prepare";
        return false;
    }

    qCDebug(appLog) << "Starting installation for" << m_packagesManager->m_preparedPackages.size() << "packages";
    m_workerStatus = WorkerProcessing;  // 刷新包安装器的工作状态
    m_operatingIndex = 0;               // 初始化当前操作的index
    m_operatingStatusIndex = 0;
    m_operatingPackageMd5 = m_packageMd5[m_operatingIndex];
    m_hierarchicalVerifyError = false;

    // start first
    initRowStatus();  // 初始化包的操作状态
    qCDebug(appLog) << "Initialized package operating status";

    // 检查当前应用是否在黑名单中
    // 非开发者模式且数字签名验证失败
    if (checkBlackListApplication() || !checkDigitalSignature()) {
        qCWarning(appLog) << "Installation blocked - blacklisted application or digital signature verification failed";
        return false;
    }

    qCDebug(appLog) << "Proceeding with installation of" << m_packagesManager->m_preparedPackages.size() << "packages";
    installNextDeb();  // 开始安装

    qCDebug(appLog) << "slotInstallPackages returning true.";
    return true;
}

bool DebListModel::slotUninstallPackage(int index)
{
    qCDebug(appLog) << "Starting uninstall for package at index" << index;
    m_workerStatus = WorkerProcessing;  // 刷新当前包安装器的工作状态
    m_operatingIndex = index;           // 获取卸载的包的indx
    m_operatingPackageMd5 = m_packageMd5[m_operatingIndex];
    // fix bug : 卸载失败时不提示卸载失败。
    m_operatingStatusIndex = index;  // 刷新操作状态的index
    m_hierarchicalVerifyError = false;
    auto dependStatus = m_packagesManager->getPackageDependsStatus(m_operatingStatusIndex);

    // for comaptible mode
    if (Pkg::CompatibleIntalled == dependStatus.status && supportCompatible() && 0 == index) {
        qCDebug(appLog) << "Uninstalling in compatible mode.";
        auto ptr = packagePtr(index);
        // check pacakge installed in compatible rootfs
        if (ptr && ptr->compatible()->installed()) {
            qCDebug(appLog) << "Package is installed in compatible rootfs.";
            if (uninstallCompatiblePackage()) {
                qCDebug(appLog) << "Compatible package uninstall successful.";
                refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Operating);
                return true;
            } else {
                qCDebug(appLog) << "Compatible package uninstall failed.";
                refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Failed);
                return false;
            }
        }

        // otherwise uninstall from current system
    }

    if (ImmBackend::instance()->immutableEnabled()) {
        qCDebug(appLog) << "Uninstalling in immutable mode.";
        auto ptr = packagePtr(index);
        if (ptr) {
            qCDebug(appLog) << "Package pointer is valid.";
            if (uninstallImmutablePackage()) {
                qCDebug(appLog) << "Immutable package uninstall successful.";
                refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Operating);
                return true;
            } else {
                qCDebug(appLog) << "Immutable package uninstall failed.";
                refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Failed);
                return false;
            }
        }

        qWarning() << qPrintable("Trigger uninstall non-installed pacakge");
        return false;
    }

    DebFile debFile(m_packagesManager->package(m_operatingIndex));  // 获取到包
    if (!debFile.isValid()) {
        qCWarning(appLog) << "Invalid deb file at index" << m_operatingIndex;
        return false;
    }
    const QStringList rdepends =
        m_packagesManager->packageReverseDependsList(debFile.packageName(), debFile.architecture());  // 检查是否有应用依赖到该包
    qCInfo(appLog) << QString("Will remove reverse depends before remove %1 , Lists:").arg(debFile.packageName()) << rdepends;

    Backend *backend = PackageAnalyzer::instance().backendPtr();

    qCInfo(appLog) << "Uninstalling reverse dependencies:" << rdepends;
    for (const auto &r : rdepends) {  // 卸载所有依赖该包的应用（二者的依赖关系为depends）
        if (backend->package(r)) {
            qCDebug(appLog) << "Purging reverse dependency:" << r;
            backend->package(r)->setPurge();
        } else {
            qCWarning(appLog) << "Failed to find reverse dependency package:" << r;
        }
    }
    // 卸载当前包 更换卸载包的方式，remove卸载不卸载完全会在影响下次安装的依赖判断。
    const QString packageId = debFile.packageName() + ':' + debFile.architecture();
    qCDebug(appLog) << "Looking up package to uninstall:" << packageId;
    QApt::Package *uninstalledPackage = backend->package(packageId);

    // 未通过当前包的包名以及架构名称获取package对象，刷新操作状态为卸载失败
    if (!uninstalledPackage) {
        qCWarning(appLog) << "Failed to find package to uninstall:" << packageId;
        refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Failed);
        return false;
    }
    uninstalledPackage->setPurge();

    refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Operating);  // 刷新当前index的操作状态
    Transaction *transsaction = backend->commitChanges();

    // trans 进度change 链接
    connect(transsaction, &Transaction::progressChanged, this, &DebListModel::signalCurrentPacakgeProgressChanged);

    // 详细状态信息（安装情况）展示链接
    connect(transsaction, &Transaction::statusDetailsChanged, this, &DebListModel::signalAppendOutputInfo);

    // trans 运行情况（授权是否成功）
    connect(transsaction, &Transaction::statusChanged, this, &DebListModel::slotTransactionStatusChanged);

    // trans运行中出现错误
    connect(transsaction, &Transaction::errorOccurred, this, &DebListModel::slotTransactionErrorOccurred);

    // 卸载结束，处理卸载成功与失败的情况并发送结束信号
    connect(transsaction, &Transaction::finished, this, &DebListModel::slotUninstallFinished);

    // 卸载结束之后 删除指针
    connect(transsaction, &Transaction::finished, transsaction, &Transaction::deleteLater);

    m_currentTransaction = transsaction;  // 保存trans指针

#ifdef ENABLE_QAPT_SETENV // Qt5环境且qapt >= 3.0.5.1-1-deepin1，支持setEnvVariable
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QVariantMap map;

    m_currentTransaction = transsaction;   //保存trans指针
    // 获取当前真实用户信息
    QString currentUser = env.value("USER");
    // 如果SUDO_USER存在，说明当前是通过sudo启动的
    QString realUser = env.value("SUDO_USER");
    if (realUser.isEmpty())
        realUser = currentUser;
    map.insert("SUDO_USER", realUser);
    transsaction->setEnvVariable(map);
#endif

    qCDebug(appLog) << "Starting uninstall transaction for package:" << packageId;
    transsaction->run();  // 开始卸载

    qCDebug(appLog) << "slotUninstallPackage returning true.";
    return true;
}

void DebListModel::removePackage(const int idx)
{
    qCDebug(appLog) << "Removing package at index" << idx;
    if (WorkerPrepare != m_workerStatus) {
        qWarning() << "installer status error";
    }
    // 去除操作状态 中的index
    int packageOperateStatusCount = m_packageOperateStatus.size() - 1;
    m_packageOperateStatus.clear();
    qCDebug(appLog) << "Clearing package operation status. Count:" << packageOperateStatusCount;
    for (int i = 0; i < packageOperateStatusCount; i++) {
        m_packageOperateStatus[m_packagesManager->getPackageMd5(i)] = Pkg::PackageOperationStatus::Prepare;
    }

    if (0 <= idx && idx < m_packagesManager->m_packageMd5.size()) {
        qCDebug(appLog) << "Removing package from pointer map.";
        m_packagePtrMap.remove(m_packagesManager->m_packageMd5[idx]);
    }

    m_packagesManager->removePackage(idx);  // 在packageManager中删除标记的下标
    qCDebug(appLog) << "Package removed from manager.";
}

Pkg::PackageInstallStatus DebListModel::checkInstallStatus(const QString &package_path)
{
    qCDebug(appLog) << "Checking install status for package path:" << package_path;
    return m_packagesManager->checkInstallStatus(package_path);
}

Pkg::DependsStatus DebListModel::checkDependsStatus(const QString &package_path)
{
    qCDebug(appLog) << "Checking depends status for package path:" << package_path;
    return static_cast<Pkg::DependsStatus>(m_packagesManager->checkDependsStatus(package_path).status);
}

int DebListModel::checkDigitalSignature(const QString &package_path)
{
    qCDebug(appLog) << "Checking digital signature for package path:" << package_path;
    // 分级管控可用时，交由分级管控进行签名验证
    if (HierarchicalVerify::instance()->isValid()) {
        qCDebug(appLog) << "Hierarchical verify is valid, returning success.";
        return Utils::VerifySuccess;
    }

    const auto stat = m_packagesManager->checkDependsStatus(package_path);  // 获取包的依赖状态
    if (stat.isBreak() || stat.isAuthCancel()) {
        qCDebug(appLog) << "Depends status is break or auth cancel, returning success.";
        return Utils::VerifySuccess;
    }
    SettingDialog dialog;
    m_isDigitalVerify = dialog.isDigitalVerified();
    int digitalSigntual = Utils::Digital_Verify(package_path);  // 判断是否有数字签名
    qCDebug(appLog) << "Digital signature verification result:" << digitalSigntual;
    if (m_isDevelopMode && !m_isDigitalVerify) {                // 开发者模式且未设置验签功能
        qCDebug(appLog) << "Developer mode and digital verification is not set, returning success.";
        return Utils::VerifySuccess;
    } else if (m_isDevelopMode && m_isDigitalVerify) {  // 开发者模式且设置验签功能
        qCDebug(appLog) << "Developer mode and digital verification is set, returning signature result.";
        return digitalSigntual;
    } else {  // 非开发者模式
        qCDebug(appLog) << "Not in developer mode, returning signature result.";
        return digitalSigntual;
    }
}

QStringList DebListModel::getPackageInfo(const QString &package_path)
{
    qCDebug(appLog) << "Getting package info for path:" << package_path;
    return m_packagesManager->getPackageInfo(package_path);
}

QString DebListModel::lastProcessError()
{
    qCDebug(appLog) << "Getting last process error.";
    if (m_currentTransaction) {
        qCDebug(appLog) << "Returning error string from current transaction:" << m_currentTransaction->errorString();
        return m_currentTransaction->errorString();
    }
    qCDebug(appLog) << "No current transaction, returning 'failed'.";
    return "failed";
}

bool DebListModel::containsSignatureFailed() const
{
    // qCDebug(appLog) << "Checking if signature failed: " << m_hierarchicalVerifyError;
    return m_hierarchicalVerifyError;
}

QString DebListModel::checkPackageValid(const QString &package_path)
{
    // qCDebug(appLog) << "Checking package validity for path:" << package_path;
    return m_packagesManager->checkPackageValid(QStringList(package_path));
}

void DebListModel::slotAppendPackage(const QStringList &package)
{
    qCDebug(appLog) << "Appending packages:";
    if (WorkerPrepare != m_workerStatus) {
        qWarning() << "installer status error";
    }
    m_packagesManager->appendPackage(package);  // 添加包，并返回添加结果
}

void DebListModel::slotTransactionStatusChanged(TransactionStatus transactionStatus)
{
    qCDebug(appLog) << "Transaction status changed:" << transactionStatus;
    switch (transactionStatus) {
        case TransactionStatus::AuthenticationStatus:  // 等待授权
            qCDebug(appLog) << "Transaction status: AuthenticationStatus, emitting signalLockForAuth(true).";
            emit signalLockForAuth(true);              // 设置底层窗口按钮不可用
            break;
        case TransactionStatus::WaitingStatus:  // 当前操作在队列中等待操作
            qCDebug(appLog) << "Transaction status: WaitingStatus, emitting signalLockForAuth(false).";
            emit signalLockForAuth(false);      // 设置底层窗口按钮可用
            break;
        default:
            qCDebug(appLog) << "Transaction status: unhandled status.";
            break;
    }
}

void DebListModel::reset()
{
    qCDebug(appLog) << "Resetting DebListModel.";
    m_workerStatus = WorkerPrepare;  // 工作状态重置为准备态
    m_operatingIndex = 0;            // 当前操作的index置为0
    m_operatingPackageMd5 = nullptr;
    m_operatingStatusIndex = 0;  // 当前操作状态的index置为0

    m_packagePtrMap.clear();

    m_packageOperateStatus.clear();  // 清空操作状态列表
    m_packageFailCode.clear();       // 清空错误原因列表
    m_packageFailReason.clear();
    m_packagesManager->reset();  // 重置packageManager

    m_hierarchicalVerifyError = false;  // 复位分级管控安装状态
    m_cacheUpdated = false;  // 重置缓存更新标记
}

int DebListModel::getInstallFileSize()
{
    qCDebug(appLog) << "Getting install file size.";
    return m_packagesManager->m_preparedPackages.size();
}

void DebListModel::resetFileStatus()
{
    qCDebug(appLog) << "Resetting file status.";
    m_packageOperateStatus.clear();  // 重置包的操作状态
    m_cacheUpdated = false;  // 重置缓存更新标记
    m_packageFailReason.clear();     // 重置包的错误状态
    m_packageFailCode.clear();
}

void DebListModel::resetInstallStatus()
{
    qCDebug(appLog) << "Resetting install status.";
    m_packageOperateStatus.clear();  // 重置包的操作状态
    m_packageFailReason.clear();     // 重置包的错误状态
    m_packageFailCode.clear();

    initPrepareStatus();
}

void DebListModel::bumpInstallIndex()
{
    qCDebug(appLog) << "Bumping install index.";
    if (m_currentTransaction.isNull()) {
        qWarning() << "previous transaction not finished";
    }
    if (++m_operatingIndex >= m_packagesManager->m_preparedPackages.size()) {
        qCDebug(appLog) << "All packages installed, finishing up.";
        m_workerStatus = WorkerFinished;       // 设置包安装器的工作状态为Finish
        emit signalWorkerFinished();           // 发送安装完成信号
        emit signalWholeProgressChanged(100);  // 修改安装进度
        emit signalCurrentPacakgeProgressChanged(100);
        return;
    }
    ++m_operatingStatusIndex;
    m_operatingPackageMd5 = m_packageMd5[m_operatingIndex];
    emit signalCurrentProcessPackageIndex(m_operatingIndex);  // 修改当前操作的下标
    // install next
    qCInfo(appLog) << "DebListModel:"
            << "install next deb package";

    // 检查当前应用是否在黑名单中
    // 非开发者模式且数字签名验证失败
    if (checkBlackListApplication() || !checkDigitalSignature()) {
        qCDebug(appLog) << "Installation blocked by blacklist or digital signature check.";
        return;
    }
    installNextDeb();  // 安装下一个包
}

void DebListModel::slotTransactionErrorOccurred()
{
    qCDebug(appLog) << "Entering slotTransactionErrorOccurred.";
    if (WorkerProcessing != m_workerStatus) {
        qWarning() << "installer status error" << m_workerStatus;
    }
    Transaction *transaction = qobject_cast<Transaction *>(sender());
    if (!transaction) {
        qCWarning(appLog) << "Transaction object is null, returning.";
        return;
    }
    // 失败时刷新操作状态为failed,并记录失败原因
    refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Failed);
    m_packageOperateStatus[m_operatingPackageMd5] = Pkg::PackageOperationStatus::Failed;

    m_packageFailCode[m_operatingPackageMd5] = transaction->error();
    m_packageFailReason[m_operatingPackageMd5] = transaction->errorString();

    if (!transaction->errorString().contains("proper authorization was not provided")) {
        qCDebug(appLog) << "Emitting signalAppendOutputInfo with error:" << transaction->errorString();
        emit signalAppendOutputInfo(transaction->errorString());
    }

    const QApt::ErrorCode errorCode = transaction->error();  // trans错误的代码

    qWarning() << "DebListModel:"
               << "Transaction Error:" << errorCode << workerErrorString(errorCode, transaction->errorString());
    qWarning() << "DebListModel:"
               << "Error Infomation:" << transaction->errorDetails() << transaction->errorString();

    if (transaction->isCancellable()) {
        qCDebug(appLog) << "Cancelling transaction.";
        transaction->cancel();
    }

    // 特殊处理授权错误
    if (AuthError == errorCode) {
        qCDebug(appLog) << "Handling authorization error.";
        transaction->deleteLater();  // 删除 trans指针
        QTimer::singleShot(
            100 * 1, this, &DebListModel::checkBoxStatus);  // 检查授权弹窗的状态 如果弹窗仍然在只是超时，则底层窗口按钮不可用
        qWarning() << "DebListModel:"
                   << "Authorization error";

        // 复位，注意先取消锁定状态再设置界面，否则焦点设置丢失
        emit signalLockForAuth(false);  // 取消授权锁定，设置按钮可用
        emit signalAuthCancel();        // 发送授权被取消的信号
        emit signalEnableCloseButton(true);
        m_workerStatus = WorkerPrepare;  // 重置工作状态为准备态
        return;
    }

    // DO NOT install next, this action will finished and will be install next automatic.
    qCDebug(appLog) << "Setting transaction exit status to failed.";
    transaction->setProperty("exitStatus", QApt::ExitFailed);  // 设置trans的退出状态为 失败
}

void DebListModel::refreshOperatingPackageStatus(Pkg::PackageOperationStatus operationStatus)
{
    qCDebug(appLog) << "Refreshing operating package status to" << operationStatus << "for index" << m_operatingStatusIndex;
    m_packageOperateStatus[m_operatingPackageMd5] = operationStatus;  // 将失败包的索引和状态修改保存,用于更新

    const QModelIndex modelIndex = index(m_operatingStatusIndex);

    emit dataChanged(modelIndex, modelIndex);  // 发送状态已经修改的信号
}

QString DebListModel::packageFailedReason(const int idx) const
{
    qCDebug(appLog) << "Entering packageFailedReason for index" << idx;
    const auto dependStatus = m_packagesManager->getPackageDependsStatus(idx);  // 获取包的依赖状态
    const auto md5 = m_packagesManager->getPackageMd5(idx);                     // 获取包的md5值
    if (m_packagesManager->isArchError(idx)) {
        qCDebug(appLog) << "Package has architecture error";
        return tr("Unmatched package architecture");  // 判断是否架构冲突
    }

    // need refactor, move to Deb::DebPackage
    int status = dependStatus.status;
    if (Pkg::CompatibleNotInstalled == status && CompBackend::instance()->supportAppCheck()) {
        qCDebug(appLog) << "Package is not installed but compatibility check is supported";
        if (auto ptr = packagePtr(idx)) {
            if (auto compPtr = ptr->compatible()) {
                // back to depends break if no support rootfs on compatible mode.
                if (compPtr->checked && compPtr->supportRootfs.isEmpty()) {
                    qCDebug(appLog) << "No support rootfs on compatible mode, setting status to DependsBreak";
                    status = Pkg::DependsBreak;
                } else if (compPtr->checked && !compPtr->targetRootfs.isEmpty()) {
                    status = Pkg::CompatibleInstallFailed;
                }
            }
        }
    }

    qCDebug(appLog) << "Checking package status" << status;
    switch (status) {
        case Pkg::CompatibleIntalled:
            qCDebug(appLog) << "Package is already installed in a compatible system";
            if (auto ptr = packagePtr(idx)) {
                QString system = ptr->compatible()->rootfs;
                system = CompBackend::instance()->osName(system);
                if (system.isEmpty()) {
                    qCDebug(appLog) << "System name is empty, using 'current system'";
                    system = tr("current system");
                }

                return tr("%2 has been installed in %1, please uninstall this package before installing it")
                    .arg(system)
                    .arg(ptr->compatible()->name);
            }
            break;
        case Pkg::CompatibleNotInstalled:
            qCDebug(appLog) << "Package has broken dependencies, suggesting compatibility mode";
            return tr("Broken dependencies, try installing the app in compatibility mode");

        case Pkg::CompatibleInstallFailed:
            return tr("Compatibility mode installation failed");

        case Pkg::Prohibit:
            qCDebug(appLog) << "Package installation is prohibited by administrator policy";
            return tr("The administrator has set policies to prevent installation of this package");

        case Pkg::DependsBreak:
            qCDebug(appLog) << "Package has broken dependencies";
            Q_FALLTHROUGH();
        case Pkg::DependsAuthCancel: {  // 依赖状态错误
            qCDebug(appLog) << "Package dependency auth canceled or broken";
            if (!dependStatus.package.isEmpty() || !m_brokenDepend.isEmpty()) {
                qCDebug(appLog) << "Has dependStatus package or brokenDepend";
                if (m_packagesManager->m_errorIndex.contains(md5)) {  // 修改wine依赖的标记方式
                    qCDebug(appLog) << "Found error index for md5" << md5;
                    auto ret = static_cast<DebListModel::DependsAuthStatus>(m_packagesManager->m_errorIndex.value(md5));
                    switch (ret) {
                        case DebListModel::VerifyDependsErr:
                            qCDebug(appLog) << "Dependency verification error";
                            return m_brokenDepend + tr("Invalid digital signature");
                        default:
                            qCDebug(appLog) << "Failed to install dependency" << m_brokenDepend;
                            return tr("Failed to install %1").arg(m_brokenDepend);  // wine依赖安装失败
                    }
                }
                qCDebug(appLog) << "Broken dependencies:" << dependStatus.package;
                return tr("Broken dependencies: %1").arg(dependStatus.package);  // 依赖不满足
            }

            const auto conflictStatus = m_packagesManager->packageConflictStat(idx);  // 获取冲突情况
            if (!conflictStatus.is_ok()) {
                qCDebug(appLog) << "Found package conflict:" << conflictStatus.unwrap();
                return tr("Broken dependencies: %1").arg(conflictStatus.unwrap());  // 依赖冲突
            }

            break;
        }
        default:
            qCDebug(appLog) << "Default case in switch, no specific reason";
            break;
    }

    // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
    qCDebug(appLog) << "Returning workerErrorString";
    return workerErrorString(m_packageFailCode[md5], m_packageFailReason[md5]);  // 根据错误代码和错误原因返回具体的错误原因
}

void DebListModel::slotTransactionFinished()
{
    qCDebug(appLog) << "Entering slotTransactionFinished";
    if (m_workerStatus == WorkerProcessing) {
        qWarning() << "installer status still processing";
    }
    // 获取trans指针
    Transaction *transaction = qobject_cast<Transaction *>(sender());
    if (!transaction) {
        qCDebug(appLog) << "Transaction object is null, returning";
        return;
    }
    // prevent next signal
    qCDebug(appLog) << "Disconnecting transaction finished signal";
    disconnect(transaction, &Transaction::finished, this, &DebListModel::slotTransactionFinished);  // 不再接收trans结束的信号

    // report new progress
    // 更新安装进度（批量安装进度控制）
    int progressValue = static_cast<int>(100. * (m_operatingIndex + 1) / m_packagesManager->m_preparedPackages.size());
    qCDebug(appLog) << "Updating whole progress to" << progressValue;
    emit signalWholeProgressChanged(progressValue);

    qCInfo(appLog) << "DebListModel:"
            << "transaciont finished with exit status:" << transaction->exitStatus();
    if (transaction->exitStatus()) {
        qCDebug(appLog) << "Transaction finished with an error";
        // 安装失败
        qWarning() << transaction->error() << transaction->errorDetails() << transaction->errorString();
        // 检测错误信息是否包含分级管控错误码，若存在，则当前错误为分级管控验证签名不通过
        QString errorInfo = transaction->errorDetails();
        if (errorInfo.isEmpty()) {
            qCDebug(appLog) << "Error details is empty, using error string";
            errorInfo = transaction->errorString();
        }
        QString sPackageName = m_packagesManager->m_preparedPackages[m_operatingIndex];
        bool verifyError = HierarchicalVerify::instance()->checkTransactionError(sPackageName, errorInfo);
        qCDebug(appLog) << "Hierarchical verification result:" << verifyError;

        // 检测安装失败时，弹出对话框提示
        if (verifyError) {
            qCDebug(appLog) << "Hierarchical verification failed, setting flag";
            // 安装结束后再弹出提示对话框
            m_hierarchicalVerifyError = true;
        }

        // 保存错误原因和错误代码
        // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
        qCDebug(appLog) << "Saving package fail code and reason";
        m_packageFailCode[m_operatingPackageMd5] =
            verifyError ? static_cast<int>(Pkg::DigitalSignatureError) : static_cast<int>(transaction->error());
        m_packageFailReason[m_operatingPackageMd5] = transaction->errorString();

        // 刷新操作状态
        qCDebug(appLog) << "Refreshing package status to Failed";
        refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Failed);
        emit signalAppendOutputInfo(transaction->errorString());
    } else if (m_packageOperateStatus.contains(m_operatingPackageMd5) &&
               m_packageOperateStatus[m_operatingPackageMd5] != Pkg::PackageOperationStatus::Failed) {
        // 安装成功
        qCDebug(appLog) << "Transaction finished successfully";
        refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Success);

        // 准备安装下一个包，修改下一个包的状态为正在安装状态
        if (m_operatingStatusIndex < m_packagesManager->m_preparedPackages.size() - 1) {
            qCDebug(appLog) << "Preparing for next package installation";
            auto md5 = m_packagesManager->getPackageMd5(m_operatingIndex + 1);
            m_packageOperateStatus[md5] = Pkg::PackageOperationStatus::Waiting;
        }
    }
    //    delete trans;
    if (!m_currentTransaction.isNull()) {
        qCDebug(appLog) << "Deleting current transaction";
        m_currentTransaction->deleteLater();
        m_currentTransaction = nullptr;
    }
    transaction = nullptr;
    qCDebug(appLog) << "Bumping install index";
    bumpInstallIndex();  // 进入安装进度控制
}

void DebListModel::slotDependsInstallTransactionFinished()  // 依赖安装关系满足
{
    qCDebug(appLog) << "Entering slotDependsInstallTransactionFinished";
    if (m_workerStatus == WorkerProcessing) {
        qWarning() << "installer status still processing";
    }
    Transaction *transaction = qobject_cast<Transaction *>(sender());
    if (!transaction) {
        qCDebug(appLog) << "Transaction object is null, returning";
        return;
    }

    const auto transExitStatus = transaction->exitStatus();
    qCDebug(appLog) << "Transaction exit status:" << transExitStatus;

    if (transExitStatus) {
        qCDebug(appLog) << "Dependency installation transaction failed";
        // record error
        // 记录错误原因和错误代码
        // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
        qWarning() << transaction->error() << transaction->errorDetails() << transaction->errorString();  // 向终端打印错误
        m_packageFailCode[m_operatingPackageMd5] = transaction->error();
        m_packageFailReason[m_operatingPackageMd5] = transaction->errorString();
        refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Failed);  // 刷新操作状态
        emit signalAppendOutputInfo(transaction->errorString());
    }

    //    delete trans;
    if (!m_currentTransaction.isNull()) {
        qCDebug(appLog) << "Deleting current transaction";
        m_currentTransaction->deleteLater();
        m_currentTransaction = nullptr;
    }
    transaction = nullptr;

    // check current operate exit status to install or install next
    if (transExitStatus) {
        qCDebug(appLog) << "Bumping install index due to failed dependency install";
        bumpInstallIndex();  // 依赖安装失败，直接安装下一个包
    } else {
        qCDebug(appLog) << "Dependency installation successful, installing next deb";
        // 安装依赖前已对此包进行黑名单及验签校验，无需二次验证
        installNextDeb();  // 依赖安装成功，开始安装这个包
    }
}

void DebListModel::setEndEnable()
{
    qCDebug(appLog) << "Entering setEndEnable";
    emit signalEnableReCancelBtn(true);
}

void DebListModel::checkBoxStatus()
{
    qCDebug(appLog) << "Entering checkBoxStatus";
    QTime startTime = QTime::currentTime();  // 获取弹出的时间
    Transaction *transation = nullptr;
    auto *const backend = PackageAnalyzer::instance().backendPtr();
    qCDebug(appLog) << "Committing changes to backend";
    transation = backend->commitChanges();

    QTime stopTime = QTime::currentTime();
    int elapsed = startTime.msecsTo(stopTime);  // 获取commit授权被取消的时间
    if (elapsed > 20000) {                      // 如果时间超过20ms则不断判断当前窗口是否超时
        qCDebug(appLog) << "Commit timeout, re-checking status";
        QTimer::singleShot(100 * 1, this, &DebListModel::checkBoxStatus);
        return;
    }

    if (transation) {
        if (transation->isCancellable()) {  // 当前transaction已经被取消
            qCDebug(appLog) << "Transaction is cancellable, cancelling it";
            transation->cancel();
            QTimer::singleShot(100 * 1, this, &DebListModel::setEndEnable);  // 设置按钮可用
        } else {
            qCDebug(appLog) << "Transaction is not cancellable, re-checking status";
            QTimer::singleShot(100 * 1, this, &DebListModel::checkBoxStatus);  // 当前Transaction还在运行中，继续等待并判断
        }
    } else {
        qWarning() << "DebListModel:"
                   << "Transaction is Nullptr";
    }
}

void DebListModel::installDebs()
{
    Q_ASSERT_X(m_workerStatus == WorkerProcessing, Q_FUNC_INFO, "installer status error");
    Q_ASSERT_X(m_currentTransaction.isNull(), Q_FUNC_INFO, "previous transaction not finished");

    // Ensure cache is updated before installation
    if (ensureCacheUpdated()) {
        qCDebug(appLog) << "Cache update in progress, waiting for completion";
        return;  // Cache update is in progress, wait for callback
    }

    // Proceed with actual installation
    performInstallation();
}

bool DebListModel::ensureCacheUpdated()
{
    if (m_cacheUpdated) {
        qCDebug(appLog) << "Cache already updated";
        return false;  // Cache is already updated, no need to wait
    }

    qCDebug(appLog) << "Cache not updated, starting update process";
    auto *const backend = PackageAnalyzer::instance().backendPtr();
    if (!backend) {
        qCWarning(appLog) << "Backend pointer is null, skipping cache update";
        m_cacheUpdated = true;
        return false;
    }

    emit signalWorkerStart();
    emit signalAppendOutputInfo("Updating package cache...");

    Transaction *transaction = backend->updateCache();
    if (!transaction) {
        qCWarning(appLog) << "Failed to create updateCache transaction, continuing without update";
        m_cacheUpdated = true;
        return false;
    }

    transaction->setLocale(".UTF-8");
    connect(transaction, &Transaction::statusDetailsChanged, this, &DebListModel::signalAppendOutputInfo);
    connect(transaction, &Transaction::finished, this, &DebListModel::slotUpdateCacheFinished);
    connect(transaction, &Transaction::errorOccurred, this, &DebListModel::slotTransactionErrorOccurred);

    m_currentTransaction = transaction;
    transaction->run();

    qCDebug(appLog) << "Cache update transaction started";
    return true;  // Cache update is in progress, need to wait
}

void DebListModel::performInstallation()
{
    qCDebug(appLog) << "Performing installation for operating index" << m_operatingIndex;
    DebFile deb(m_packagesManager->package(m_operatingIndex));
    if (!deb.isValid()) {
        qCDebug(appLog) << "Deb file is invalid, returning";
        return;
    }
    qCInfo(appLog) << QString("Prepare to install %1, ver: %2, arch: %3").arg(deb.packageName()).arg(deb.version()).arg(deb.architecture());

    // Get backend pointer
    auto *const backend = PackageAnalyzer::instance().backendPtr();
    if (!backend) {
        qCWarning(appLog) << "Backend pointer is null, cannot perform installation";
        return;
    }

    emit signalWorkerStart();

    Transaction *transaction = nullptr;

    // reset package depends status on installNextDeb(), check available dependencies
    const auto dependsStat = m_packagesManager->getPackageDependsStatus(m_operatingStatusIndex);

    // for compatbile install
    if (dependsStat.canInstallCompatible() && supportCompatible()) {
        qCDebug(appLog) << "Package can be installed in compatible mode";
        if (installCompatiblePackage()) {
            qCDebug(appLog) << "Compatible package installation started, setting status to Operating";
            refreshOperatingPackageStatus(Pkg::Operating);
        } else {
            qCDebug(appLog) << "Compatible package installation failed, setting status to Failed";
            refreshOperatingPackageStatus(Pkg::Failed);
        }
        return;
    }

    // for immutable system, if immutable is enabled, the normal installation process will not be entered
    if (dependsStat.canInstall() && ImmBackend::instance()->immutableEnabled()) {
        qCDebug(appLog) << "Package can be installed in immutable mode";
        if (installImmutablePackage()) {
            qCDebug(appLog) << "Immutable package installation started, setting status to Operating";
            refreshOperatingPackageStatus(Pkg::Operating);
        } else {
            qCDebug(appLog) << "Immutable package installation failed, setting status to Failed";
            refreshOperatingPackageStatus(Pkg::Failed);
        }
        return;
    }

    if (!dependsStat.canInstall()) {
        qCDebug(appLog) << "Dependencies cannot be satisfied";
        // 依赖不满足或者下载wine依赖时授权被取消
        refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Failed);  // 刷新错误状态

        // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
        m_packageFailCode.insert(m_operatingPackageMd5, -1);  // 保存错误原因
        // 记录详细错误原因
        m_packageFailReason.insert(m_operatingPackageMd5, packageFailedReason(m_operatingStatusIndex));

        qCDebug(appLog) << "Bumping install index due to unsatisfied dependencies";
        bumpInstallIndex();  // 开始下一步的安装流程
        return;
    } else if (dependsStat.isAvailable()) {
        qCDebug(appLog) << "Dependencies are available";
        if (isDpkgRunning()) {
            qCInfo(appLog) << "DebListModel:"
                    << "dpkg running, waitting...";
            // 缩短检查的时间，每隔1S检查当前dpkg是否正在运行。
            qCDebug(appLog) << "Dpkg is running, waiting before next install check";
            QTimer::singleShot(1000 * 1, this, &DebListModel::installNextDeb);
            return;
        }
        // 依赖可用 但是需要下载
        Q_ASSERT_X(m_packageOperateStatus[m_operatingPackageMd5],
                   Q_FUNC_INFO,
                   "package operate status error when start install availble dependencies");

        // 获取到所有的依赖包 准备安装
        const QStringList availableDepends = m_packagesManager->packageAvailableDepends(m_operatingIndex);
        qCInfo(appLog) << QString("Prepare install package: %1 , install depends: ").arg(deb.packageName()) << availableDepends;

        // 获取到可用的依赖包并根据后端返回的结果判断依赖包的安装结果
        for (auto const &p : availableDepends) {
            if (p.contains(" not found")) {                                          // 依赖安装失败
                qCDebug(appLog) << "Dependency not found:" << p;
                refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Failed);  // 刷新当前包的状态
                // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
                m_packageFailCode.insert(m_operatingPackageMd5, DownloadDisallowedError);  // 记录错误代码与错误原因
                m_packageFailReason.insert(m_operatingPackageMd5, p);
                emit signalAppendOutputInfo(m_packagesManager->package(m_operatingIndex) + "\'s depend " + " " +
                                            p);  // 输出错误原因
                bumpInstallIndex();              // 开始安装下一个包或结束安装

                qWarning() << QString("Packge %1 install failed, not found depend package: %2").arg(deb.packageName()).arg(p);
                return;
            }
            qCDebug(appLog) << "Marking dependency for installation:" << p;
            backend->markPackageForInstall(p);  // 开始安装依赖包
        }
        // 打印待安装的软件包信息
        printDependsChanges();

        qCDebug(appLog) << "Committing changes for dependency installation";
        transaction = backend->commitChanges();
        if (!transaction) {
            qCDebug(appLog) << "Transaction is null after committing changes, returning";
            return;
        }
        // 依赖安装结果处理
        connect(transaction, &Transaction::finished, this, &DebListModel::slotDependsInstallTransactionFinished);
    } else {
        qCDebug(appLog) << "Dependencies are not available";
        if (isDpkgRunning()) {
            qCInfo(appLog) << "DebListModel:"
                    << "dpkg running, waitting...";
            // 缩短检查的时间，每隔1S检查当前dpkg是否正在运行。
            qCDebug(appLog) << "Dpkg is running, waiting before next install check";
            QTimer::singleShot(1000 * 1, this, &DebListModel::installNextDeb);
            return;
        }
        qCDebug(appLog) << "Installing file directly";
        transaction = backend->installFile(deb);  // 触发Qapt授权框和安装线程
        if (!transaction) {
            qCDebug(appLog) << "Transaction is null after installing file, returning";
            return;
        }
        // 进度变化和结束过程处理
        connect(transaction, &Transaction::progressChanged, this, &DebListModel::signalCurrentPacakgeProgressChanged);
        connect(transaction, &Transaction::finished, this, &DebListModel::slotTransactionFinished);
    }

    // NOTE: DO NOT remove this.
    transaction->setLocale(".UTF-8");

    // 记录日志
    connect(transaction, &Transaction::statusDetailsChanged, this, &DebListModel::signalAppendOutputInfo);

    // 刷新操作状态
    connect(transaction, &Transaction::statusDetailsChanged, this, &DebListModel::slotTransactionOutput);

    // 授权处理
    connect(transaction, &Transaction::statusChanged, this, &DebListModel::slotTransactionStatusChanged);

    // 错误处理
    connect(transaction, &Transaction::errorOccurred, this, &DebListModel::slotTransactionErrorOccurred);

    m_currentTransaction = transaction;

#ifdef ENABLE_QAPT_SETENV   // Qt5环境且qapt >= 3.0.5.1-1-deepin1，支持setEnvVariable
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QVariantMap map;

    // 获取当前真实用户信息
    QString currentUser = env.value("USER");
    // 如果SUDO_USER存在，说明当前是通过sudo启动的
    QString realUser = env.value("SUDO_USER");
    if (realUser.isEmpty())
        realUser = currentUser;
    map.insert("SUDO_USER", realUser);
    m_currentTransaction->setEnvVariable(map);
#endif
    qCDebug(appLog) << "Running current transaction" ;
    m_currentTransaction->run();
}

void DebListModel::digitalVerifyFailed(Pkg::ErrorCode errorCode)
{
    qCDebug(appLog) << "Entering digitalVerifyFailed with error code" << errorCode;
    if (preparedPackages().size() > 1) {                                     // 批量安装
        qCDebug(appLog) << "Multiple packages prepared, handling failure for current package";
        refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Failed);  // 刷新操作状态
        // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
        m_packageFailCode.insert(m_operatingPackageMd5, errorCode);  // 记录错误代码与错误原因
        m_packageFailReason.insert(m_operatingPackageMd5, "");
        bumpInstallIndex();  // 跳过当前包
    } else if (preparedPackages().size() == 1) {
        qCDebug(appLog) << "Single package prepared";
        if (!m_isDevelopMode) {
            qCDebug(appLog) << "Not in developer mode, exiting";
            exit(0);
        } else {  // 开发者模式下，点击取消按钮，返回错误界面
            qCDebug(appLog) << "In developer mode, setting status to failed";
            refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Failed);  // 刷新操作状态
            // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
            m_packageFailCode.insert(m_operatingPackageMd5, errorCode);  // 记录错误代码与错误原因
            m_packageFailReason.insert(m_operatingPackageMd5, "");
            qCDebug(appLog) << "Emitting signalWorkerFinished";
            emit signalWorkerFinished();
        }
    }
}

void DebListModel::showNoDigitalErrWindowInDdimProcess(void (DebListModel::*failedFunction)())
{
    qCDebug(appLog) << "Entering showNoDigitalErrWindowInDdimProcess";
    DDialog *Ddialog = new DDialog();  // 弹出窗口
    Ddialog->setModal(true);
    Ddialog->setWindowFlag(Qt::WindowStaysOnTopHint);  // 窗口一直置顶
    Ddialog->setTitle(tr("Unable to install"));
    Ddialog->setIcon(QIcon::fromTheme("di_popwarning"));
    Ddialog->addButton(QString(tr("OK", "button")), true, DDialog::ButtonNormal);  // 添加确认按钮

    auto fullPath = m_packagesManager->package(m_operatingIndex);
    QFileInfo info(fullPath);
    Ddialog->setMessage(QString(tr("Failed to install %1: no valid digital signature").arg(info.fileName())) + QString("!"));

    // 消息框reject后的操作，包括点击取消按钮、关闭图标、按ESC退出
    std::function<void(void)> rejectOperate = [this, Ddialog, failedFunction]() {
        qCDebug(appLog) << "Dialog rejected";
        if (failedFunction) {
            qCDebug(appLog) << "Calling failedFunction";
            (this->*failedFunction)();
        }
        Ddialog->deleteLater();
    };

    // 取消按钮
    QPushButton *btnOk = qobject_cast<QPushButton *>(Ddialog->getButton(0));
    connect(btnOk, &DPushButton::clicked, Ddialog, &DDialog::reject);

    // 关闭图标
    connect(Ddialog, &DDialog::aboutToClose, Ddialog, &DDialog::reject);

    // ESC退出
    connect(Ddialog, &DDialog::rejected, rejectOperate);

    Ddialog->exec();  // 显示弹窗
}

void DebListModel::showNoDigitalErrWindow()
{
    qCDebug(appLog) << "Entering showNoDigitalErrWindow";
    if (SingleInstallerApplication::mode == SingleInstallerApplication::DdimChannel) {
        qCDebug(appLog) << "Ddim channel detected, showing specific error window";
        showNoDigitalErrWindowInDdimProcess(&DebListModel::slotNoDigitalSignature);
        return;
    }

    // 批量安装时，如果不是最后一个包，则不弹窗，只记录详细错误原因。
    if (m_operatingIndex < m_packagesManager->m_preparedPackages.size() - 1) {
        qCDebug(appLog) << "Not the last package in batch install, failing silently";
        digitalVerifyFailed(Pkg::NoDigitalSignature);  // 刷新安装错误，并记录错误原因
        return;
    }
    qCDebug(appLog) << "Showing no digital signature dialog";
    DDialog *Ddialog = new DDialog();  // 弹出窗口
    Ddialog->setModal(true);
    Ddialog->setWindowFlag(Qt::WindowStaysOnTopHint);  // 窗口一直置顶
    Ddialog->setTitle(tr("Unable to install - no digital signature"));
    Ddialog->setMessage(QString(tr("Please go to Control Center to enable developer mode and try again. Proceed?")));
    Ddialog->setIcon(QIcon::fromTheme("di_popwarning"));

    Ddialog->addButton(QString(tr("Cancel", "button")), true, DDialog::ButtonNormal);      // 添加取消按钮
    Ddialog->addButton(QString(tr("Proceed", "button")), true, DDialog::ButtonRecommend);  // 添加前往按钮
    Ddialog->show();                                                                       // 显示弹窗

    // 消息框reject后的操作，包括点击取消按钮、关闭图标、按ESC退出
    std::function<void(void)> rejectOperate = [this, Ddialog]() {
        qCDebug(appLog) << "Dialog rejected, calling slotNoDigitalSignature";
        this->slotNoDigitalSignature();
        Ddialog->deleteLater();
    };

    // 取消按钮
    QPushButton *btnCancel = qobject_cast<QPushButton *>(Ddialog->getButton(0));
    connect(btnCancel, &DPushButton::clicked, rejectOperate);

    // 关闭图标
    connect(Ddialog, &DDialog::aboutToClose, rejectOperate);

    // ESC退出
    connect(Ddialog, &DDialog::rejected, rejectOperate);

    // 前往按钮1
    QPushButton *btnProceedControlCenter = qobject_cast<QPushButton *>(Ddialog->getButton(1));
    connect(btnProceedControlCenter, &DPushButton::clicked, this, &DebListModel::slotShowDevelopModeWindow);
    connect(btnProceedControlCenter, &DPushButton::clicked, this, &QApplication::exit);
    connect(btnProceedControlCenter, &DPushButton::clicked, Ddialog, &DDialog::deleteLater);
}

void DebListModel::showDigitalErrWindow(bool recordError)
{
    qCDebug(appLog) << "Entering showDigitalErrWindow, recordError:" << recordError;
    if (SingleInstallerApplication::mode == SingleInstallerApplication::DdimChannel) {
        qCDebug(appLog) << "Ddim channel detected";
        // 不记录错误时仅提示，不涉及状态切换及更新记录
        showNoDigitalErrWindowInDdimProcess(recordError ? &DebListModel::slotDigitalSignatureError : nullptr);
        return;
    }

    // 批量安装时，如果不是最后一个包，则不弹窗，只记录详细错误原因。
    if (m_operatingIndex < m_packagesManager->m_preparedPackages.size() - 1) {
        qCDebug(appLog) << "Not the last package in batch install";
        if (recordError) {
            qCDebug(appLog) << "Recording digital signature error and failing";
            digitalVerifyFailed(Pkg::DigitalSignatureError);  // 刷新安装错误，并记录错误原因
        }
        return;
    }
    qCDebug(appLog) << "Showing digital error dialog";
    DDialog *Ddialog = new DDialog();
    // 设置窗口焦点
    Ddialog->setFocusPolicy(Qt::TabFocus);

    // 设置弹出窗口为模态窗口
    Ddialog->setModal(true);

    // 设置窗口始终置顶
    Ddialog->setWindowFlag(Qt::WindowStaysOnTopHint);

    // 设置弹出窗口显示的信息
    Ddialog->setTitle(tr("Unable to install"));
    Ddialog->setMessage(QString(tr("This package does not have a valid digital signature")) + QString("!"));
    Ddialog->setIcon(QIcon::fromTheme("di_popwarning"));
    Ddialog->addButton(QString(tr("OK", "button")), true, DDialog::ButtonNormal);
    Ddialog->show();
    QPushButton *btnOK = qobject_cast<QPushButton *>(Ddialog->getButton(0));
    btnOK->setFocusPolicy(Qt::TabFocus);
    btnOK->setFocus();

    // 窗口退出操作，包括所有可以退出此窗口的操作
    std::function<void(void)> exitOperate = [this, Ddialog, recordError]() {
        qCDebug(appLog) << "Dialog exit operation triggered";
        if (recordError) {
            qCDebug(appLog) << "Calling slotDigitalSignatureError";
            this->slotDigitalSignatureError();
        }
        Ddialog->deleteLater();
    };

    // 点击弹出窗口的关闭图标按钮
    connect(Ddialog, &DDialog::aboutToClose, exitOperate);

    // 点击弹出窗口的确定按钮
    connect(btnOK, &DPushButton::clicked, exitOperate);

    // ESC退出
    connect(Ddialog, &DDialog::rejected, exitOperate);
}

void DebListModel::showDevelopDigitalErrWindow(Pkg::ErrorCode code)
{
    qCDebug(appLog) << "Entering showDevelopDigitalErrWindow with error code" << code;
    Dialog *Ddialog = new Dialog();
    // 设置窗口焦点
    // fix bug:https://pms.uniontech.com/zentao/bug-view-44837.html
    Ddialog->setFocusPolicy(Qt::TabFocus);

    // 设置弹出窗口为模态窗口
    Ddialog->setModal(true);

    // 设置窗口始终置顶
    Ddialog->setWindowFlag(Qt::WindowStaysOnTopHint);

    // 设置弹出窗口显示的信息
    Ddialog->setMessage(QString(tr("This package does not have a valid digital signature. Continue with the installation?")));
    Ddialog->setIcon(QIcon::fromTheme("di_popwarning"));
    Ddialog->addButton(QString(tr("Cancel")), true, DDialog::ButtonNormal);
    Ddialog->addButton(QString(tr("Continue", "button")), true, DDialog::ButtonRecommend);  // 添加前往按钮

    Ddialog->show();
    QPushButton *cancelBtn = qobject_cast<QPushButton *>(Ddialog->getButton(0));

    cancelBtn->setFocusPolicy(Qt::TabFocus);
    cancelBtn->setFocus();

    // 点击弹出窗口的关闭图标按钮
    connect(Ddialog, &DDialog::aboutToClose, this, [=] {
        qCDebug(appLog) << "Close button clicked on develop digital error window, calling digitalVerifyFailed";
        // 刷新当前包的操作状态，失败原因为数字签名校验失败
        digitalVerifyFailed(code);
    });
    connect(Ddialog, &DDialog::aboutToClose, Ddialog, &DDialog::deleteLater);

    // 点击弹出窗口的确定按钮
    connect(cancelBtn, &DPushButton::clicked, this, [=] {
        qCDebug(appLog) << "Cancel button clicked on develop digital error window, calling digitalVerifyFailed";
        digitalVerifyFailed(code);
    });
    connect(cancelBtn, &DPushButton::clicked, Ddialog, &DDialog::deleteLater);

    QPushButton *continueBtn = qobject_cast<QPushButton *>(Ddialog->getButton(1));
    connect(continueBtn, &DPushButton::clicked, this, [&] {
        qCDebug(appLog) << "Continue button clicked on develop digital error window, calling installNextDeb";
        installNextDeb();
    });  // 点击继续，进入安装流程
    connect(continueBtn, &DPushButton::clicked, Ddialog, &DDialog::deleteLater);
    connect(Ddialog, &Dialog::signalClosed, this, [=] { digitalVerifyFailed(code); });
    connect(Ddialog, &Dialog::signalClosed, Ddialog, &DDialog::deleteLater);
}

void DebListModel::slotDigitalSignatureError()
{
    qCDebug(appLog) << "Entering slotDigitalSignatureError";
    digitalVerifyFailed(Pkg::DigitalSignatureError);
}

void DebListModel::slotNoDigitalSignature()
{
    qCDebug(appLog) << "Entering slotNoDigitalSignature";
    digitalVerifyFailed(Pkg::NoDigitalSignature);
}

void DebListModel::slotShowDevelopModeWindow()
{
    qCDebug(appLog) << "Entering slotShowDevelopModeWindow";
    // 弹出设置 通用窗口

    // 1.读取系统版本号
    qCDebug(appLog) << "Reading system version";
    QProcess *unlock = new QProcess(this);
    unlock->start("lsb_release", {"-r"});
    unlock->waitForFinished();
    auto output = unlock->readAllStandardOutput();
    auto str = QString::fromUtf8(output);
    REG_EXP re("\t.+\n");
    QString osVerStr;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (re.indexIn(str) > -1) {
        auto result = re.cap(0);
#else
    QRegularExpressionMatch match = re.match(str);
    if (match.hasMatch()) {
        auto result = match.captured(0);
#endif
        osVerStr = result.remove(0, 1).remove(result.size() - 1, 1);
        qCInfo(appLog) << "lsb_release -r:" << output;
        qCInfo(appLog) << "OS version:" << osVerStr;
    }

    // 2.打开控制中心
    qCDebug(appLog) << "Opening control center for OS version:" << osVerStr;
    if (osVerStr == "20") {  // V20模式
        qCDebug(appLog) << "OS version is 20, using com.deepin.dde.ControlCenter";
        QDBusInterface interface("com.deepin.dde.ControlCenter", "/com/deepin/dde/ControlCenter", "com.deepin.dde.ControlCenter");
        if (interface.isValid()) {
            qCDebug(appLog) << "Calling ShowPage on D-Bus interface";
            interface.call("ShowPage", "commoninfo", "Developer Mode");
        }

        QDBusError error = interface.lastError();
        if (error.isValid()) {
            qWarning() << QString("DBus ControlCenter.ShowPage failed, Type: %1 MSG: %2").arg(error.type()).arg(error.message());
        }

    } else if (osVerStr == "23") {  // V23模式
        qCDebug(appLog) << "OS version is 23, using org.deepin.dde.ControlCenter1";
        if (unlock->exitCode() != QProcess::NormalExit) {
            QDBusInterface interface(
                "org.deepin.dde.ControlCenter1", "/org/deepin/dde/ControlCenter1", "org.deepin.dde.ControlCenter1");
            if (interface.isValid()) {
                qCDebug(appLog) << "Calling ShowPage on D-Bus interface for v23";
                interface.call("ShowPage", "commoninfo", "Developer Mode");
            }

            QDBusError error = interface.lastError();
            if (error.isValid()) {
                qWarning()
                    << QString("DBus ControlCenter.ShowPage failed, Type: %1 MSG: %2").arg(error.type()).arg(error.message());
            }
        }

    } else {
        qWarning() << qPrintable("Unknown OS version, connot open dde-control-center");
    }

    unlock->deleteLater();
}

bool DebListModel::checkDigitalSignature()
{
    qCDebug(appLog) << "Entering checkDigitalSignature";
    // 分级管控可用时，交由分级管控进行签名验证
    if (HierarchicalVerify::instance()->isValid()) {
        qCDebug(appLog) << "Hierarchical verification is valid, skipping signature check";
        return true;
    }

    const auto stat = m_packagesManager->getPackageDependsStatus(m_operatingIndex);  // 获取包的依赖状态
    if (stat.isBreak() || stat.isAuthCancel()) {
        qCDebug(appLog) << "Dependency is broken or auth cancelled, skipping signature check";
        return true;
    }
    SettingDialog dialog;
    m_isDigitalVerify = dialog.isDigitalVerified();
    int digitalSigntual = Utils::Digital_Verify(m_packagesManager->package(m_operatingIndex));  // 判断是否有数字签名
    qCInfo(appLog) << "m_isDevelopMode:" << m_isDevelopMode << " /m_isDigitalVerify:" << m_isDigitalVerify
            << " /digitalSigntual:" << digitalSigntual;
    if (m_isDevelopMode && !m_isDigitalVerify) {  // 开发者模式且未设置验签功能
        qCDebug(appLog) << "Developer mode and digital verification is disabled, returning true";
        return true;
    } else if (m_isDevelopMode && m_isDigitalVerify) {  // 开发者模式且设置验签功能
        qCDebug(appLog) << "Developer mode and digital verification is enabled";
        if (digitalSigntual == Utils::VerifySuccess) {
            qCDebug(appLog) << "Signature verification successful in developer mode";
            return true;
        } else {
            qCDebug(appLog) << "Signature verification failed in developer mode";
            Pkg::ErrorCode code;
            if (digitalSigntual == Utils::DebfileInexistence) {
                qCDebug(appLog) << "Deb file inexistence, setting code to NoDigitalSignature";
                code = Pkg::NoDigitalSignature;
            } else {
                qCDebug(appLog) << "Digital signature error, setting code to DigitalSignatureError";
                code = Pkg::DigitalSignatureError;
            }
            showDevelopDigitalErrWindow(code);  // 弹出提示框
            return false;
        }
    } else {  // 非开发者模式
        qCDebug(appLog) << "Not in developer mode, checking signature status";
        bool verifiedResult = false;
        switch (digitalSigntual) {
            case Utils::VerifySuccess:  // 签名验证成功
                qCDebug(appLog) << "Signature verification successful";
                verifiedResult = true;
                break;
            case Utils::DebfileInexistence:  // 无签名文件
                qCDebug(appLog) << "Deb file inexistence, showing no digital signature window";
                showNoDigitalErrWindow();
                verifiedResult = false;
                break;
            case Utils::ExtractDebFail:  // 无有效的数字签名
                qCDebug(appLog) << "Failed to extract deb, showing digital error window";
                showDigitalErrWindow();
                verifiedResult = false;
                break;
            case Utils::DebVerifyFail:
            case Utils::OtherError:  // 其他原因造成的签名校验失败
                qCDebug(appLog) << "Deb verification failed or other error, showing digital error window";
                showDigitalErrWindow();
                verifiedResult = false;
                break;
            default:  // 其他未知错误
                qCInfo(appLog) << "unknown mistake";
                qCDebug(appLog) << "Unknown signature verification error";
                verifiedResult = false;
                break;
        }
        return verifiedResult;
    }
}

void DebListModel::installNextDeb()
{
    qCDebug(appLog) << "Entering installNextDeb for operating index" << m_operatingStatusIndex;
    // If package is first package or install to compatible mode, not need reset status.
    bool isFirstPackageAndCached = (0 == m_operatingStatusIndex) && m_packagesManager->cachedPackageDependStatus(m_operatingStatusIndex);
    if (!isFirstPackageAndCached) {
        qCDebug(appLog) << "Not first package or not cached, resetting package depends status";
        // The apt backend cache may changed, refresh package status.
        m_packagesManager->resetPackageDependsStatus(m_operatingStatusIndex);
    }
    PackageDependsStatus dependStatus = m_packagesManager->getPackageDependsStatus(m_operatingStatusIndex);
    qCDebug(appLog) << "Current dependency status:" << dependStatus.status;

    if (dependStatus.canInstallCompatible() && supportCompatible()) {
        qCDebug(appLog) << "Can install in compatible mode, calling installDebs";
        installDebs();
    } else if (ImmBackend::instance()->immutableEnabled()) {
        qCDebug(appLog) << "Immutable backend enabled, calling installDebs";
        installDebs();
    } else if (dependStatus.isAvailable()) {  // 存在没有安装的依赖包，则进入普通安装流程执行依赖安装
        qCDebug(appLog) << "Dependencies are available, calling installDebs";
        installDebs();
    } else if (dependStatus.status >= Pkg::DependsStatus::DependsBreak) {  // 安装前置条件不满足，无法处理
        qCDebug(appLog) << "Dependencies broken, failing and bumping index";
        refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Failed);
        bumpInstallIndex();
        return;
    } else {  // 如果当前包的依赖全部安装完毕，则进入配置判断流程
        qCDebug(appLog) << "Dependencies satisfied, checking for debconf";
        QString sPackageName = m_packagesManager->m_preparedPackages[m_operatingIndex];
        if (Utils::checkPackageContainsDebConf(sPackageName)) {  // 检查当前包是否需要配置
            qCDebug(appLog) << "Package contains debconf, starting install config process";
            m_procInstallConfig->start("pkexec",
                                       QStringList() << "pkexec"
                                                     << "deepin-deb-installer-dependsInstall"
                                                     << "--install_config" << sPackageName,
                                       {},
                                       0,
                                       false);  // 配置安装流程
        } else {
            qCDebug(appLog) << "No debconf, proceeding with normal installation by calling installDebs";
            installDebs();  // 普通安装流程
        }
    }
}

void DebListModel::slotTransactionOutput()
{
    qCDebug(appLog) << "Entering slotTransactionOutput";
    if (m_workerStatus == WorkerProcessing) {
        qCInfo(appLog) << "installer status error";
    }
    Transaction *trans = qobject_cast<Transaction *>(sender());
    if (!trans) {
        qCDebug(appLog) << "Transaction object is null, returning";
        return;
    }

    qCDebug(appLog) << "Refreshing package status to Operating";
    refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Operating);  // 刷新当前包的操作状态

    disconnect(trans, &Transaction::statusDetailsChanged, this, &DebListModel::slotTransactionOutput);
}

void DebListModel::slotUninstallFinished()
{
    qCDebug(appLog) << "Entering slotUninstallFinished";
    if (m_workerStatus == WorkerProcessing) {
        qCInfo(appLog) << "installer status error";
    }

    // 增加卸载失败的情况
    // 此前的做法是发出commitError的信号，现在全部在Finished中进行处理。不再特殊处理。
    Transaction *trans = qobject_cast<Transaction *>(sender());
    if (!trans) {
        qCDebug(appLog) << "Transaction object is null, returning";
        return;
    }

    if (trans->exitStatus()) {
        qCDebug(appLog) << "Uninstall finished with an error";
        m_workerStatus = WorkerFinished;                                     // 刷新包安装器的工作状态
        refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Failed);  // 刷新当前包的操作状态
        m_packageOperateStatus[m_operatingPackageMd5] = Pkg::PackageOperationStatus::Failed;
        qWarning() << "DebListModel:"
                   << "uninstall finished with finished code:" << trans->error() << "finished details:" << trans->errorString();
    } else {
        qCDebug(appLog) << "Uninstall finished successfully";
        m_workerStatus = WorkerFinished;                                      // 刷新包安装器的工作状态
        refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Success);  // 刷新当前包的卸载状态
        m_packageOperateStatus[m_operatingPackageMd5] = Pkg::PackageOperationStatus::Success;
    }
    qCDebug(appLog) << "Emitting signalWorkerFinished for uninstall";
    emit signalWorkerFinished();  // 发送结束信号（只有单包卸载）卸载结束就是整个流程的结束
    trans->deleteLater();
}

void DebListModel::slotSetCurrentIndex(const QModelIndex &modelIndex)
{
    qCDebug(appLog) << "Entering slotSetCurrentIndex";
    if (m_currentIdx == modelIndex) {
        qCDebug(appLog) << "New index is same as current, returning";
        return;  // 要修改的index与当前index 一致
    }

    const QModelIndex index = m_currentIdx;  // 保存当前的index
    m_currentIdx = modelIndex;               // 修改当前的index
    qCDebug(appLog) << "Current index changed to" << m_currentIdx.row();

    emit dataChanged(index, index);
    emit dataChanged(m_currentIdx, m_currentIdx);  // 发送index修改信号
}

void DebListModel::initPrepareStatus()
{
    qCDebug(appLog) << "Entering initPrepareStatus";
    for (int i = 0; i < m_packagesManager->m_preparedPackages.size(); i++) {
        auto md5 = m_packagesManager->getPackageMd5(i);
        qCDebug(appLog) << "Setting package" << md5 << "to Prepare status";
        m_packageOperateStatus.insert(md5, Pkg::PackageOperationStatus::Prepare);  // 刷新当前所有包的状态为Prepare
    }
}

void DebListModel::initRowStatus()
{
    qCDebug(appLog) << "Entering initRowStatus";
    // 更换状态存储方式后修改更新状态的方式
    for (auto md5 : m_packageMd5) {
        qCDebug(appLog) << "Setting package" << md5 << "to Waiting status";
        m_packageOperateStatus[md5] = Pkg::PackageOperationStatus::Waiting;
    }
}

void DebListModel::slotUpWrongStatusRow()
{
    qCDebug(appLog) << "Entering slotUpWrongStatusRow";
    if (m_packagesManager->m_preparedPackages.size() == 1) {
        qCDebug(appLog) << "Only one package, no need to update wrong status row, returning";
        return;
    }

    QList<QByteArray> installErrorPackages;    // 安装错误的包的list
    QList<QByteArray> installSuccessPackages;  // 安装成功的包的list

    // 根据包的操作状态，分别找到所有安装成功的包与安装失败的包
    qCDebug(appLog) << "Separating packages by installation status";
    QMapIterator<QByteArray, int> iteratorpackageOperateStatus(m_packageOperateStatus);
    while (iteratorpackageOperateStatus.hasNext()) {
        iteratorpackageOperateStatus.next();
        // 保存安装成功的包
        if (iteratorpackageOperateStatus.value() == Pkg::PackageOperationStatus::Failed ||
            iteratorpackageOperateStatus.value() == Pkg::PackageOperationStatus::VerifyFailed) {  // 安装失败或签名验证失败
            installErrorPackages.append(iteratorpackageOperateStatus.key());                      // 保存下标
        }
        // 保存安装失败的包
        if (iteratorpackageOperateStatus.value() == Pkg::PackageOperationStatus::Success) {
            installSuccessPackages.append(iteratorpackageOperateStatus.key());
        }
    }
    qCDebug(appLog) << "Failed packages count:" << installErrorPackages.size() << ", Success packages count:" << installSuccessPackages.size();
    if (installErrorPackages.size() == 0) { // 全部安装成功 直接退出
        qCDebug(appLog) << "No error packages, returning";
        return;
    }

    // 先将包与md5 绑定
    // 后续要对根据MD5对包的路径进行排序，保证包名和md5的下标统一
    qCDebug(appLog) << "Reordering packages based on status";
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

    // update view
    qCDebug(appLog) << "Emitting dataChanged to update view";
    const QModelIndex idxStart = index(0);
    const QModelIndex idxEnd = index(m_packageOperateStatus.size() - 1);
    emit dataChanged(idxStart, idxEnd);

    // update scroll
    emit signalCurrentProcessPackageIndex(-1);
}

void DebListModel::slotConfigInstallFinish(int installResult)
{
    qCDebug(appLog) << "Config install finish, installResult: " << installResult;
    if (m_packagesManager->m_preparedPackages.size() == 0) {
        qCDebug(appLog) << "No prepared packages, returning";
        return;
    }
    int progressValue = static_cast<int>(100. * (m_operatingIndex + 1) /
                                         m_packagesManager->m_preparedPackages.size());  // 批量安装时对进度进行处理
    emit signalWholeProgressChanged(progressValue);
    if (0 == installResult) {  // 安装成功
        qCDebug(appLog) << "Installation successful";
        if (m_packagesManager->m_packageMd5DependsStatus[m_packagesManager->m_packageMd5[m_operatingIndex]].status ==
            Pkg::DependsStatus::DependsOk) {
            qCDebug(appLog) << "Depends ok, refreshing status to Success";
            refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Success);  // 刷新安装状态
            m_procInstallConfig->terminate();                                     // 结束配置
            m_procInstallConfig->close();
        }
        bumpInstallIndex();  // 开始安装下一个
    } else {
        qCDebug(appLog) << "Installation failed";
        if (1 == m_packagesManager->m_preparedPackages.size()) {                  // 单包安装
            qCDebug(appLog) << "Single package installation, refreshing status to Prepare";
            refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Prepare);  // 刷新当前包的操作状态为准备态
            m_workerStatus = WorkerPrepare;
            emit signalAuthCancel();  // 授权取消
        } else {
            // 批量安装
            qCDebug(appLog) << "Batch installation, refreshing status to Failed";
            refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Failed);  // 刷新当前包的状态为失败

            // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
            m_packageFailCode.insert(m_operatingPackageMd5, installResult);  // 保存失败原因
            m_packageFailReason.insert(m_operatingPackageMd5, "Authentication failed");
            bumpInstallIndex();  // 开始安装下一个
        }
    }
    qCDebug(appLog) << "Hiding and clearing config window";
    configWindow->hide();        // 隐藏配置窗口
    configWindow->clearTexts();  // 清楚配置信息
    //    m_procInstallConfig->terminate();                               //结束配置
    //    m_procInstallConfig->close();
}

void DebListModel::slotConfigReadOutput(const char *buffer, int length, bool isCommandExec)
{
    qCDebug(appLog) << "Config read output, length:" << length;
    QString tmp = QByteArray(buffer, length);  // 获取配置读取到的信息

    tmp.remove(QChar('"'), Qt::CaseInsensitive);
    tmp.remove(QChar('\n'), Qt::CaseInsensitive);

    // 取消授权弹窗，则不显示配置安装界面
    if (!tmp.contains("Error executing command as another user: Request dismissed")) {
        qCDebug(appLog) << "No error, showing config window";
        // 获取到当前正在安装配置
        emit signalWorkerStart();
        refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Operating);  // 刷新当前的操作状态
        configWindow->show();                                                   // 显示配置窗口

        int iCutoutNum = tmp.size();
        if (iCutoutNum > 0) {
            qCDebug(appLog) << "Appending output info to config window";
            emit signalAppendOutputInfo(tmp);  // 原本安装信息界面信息也要添加，以备安装完成后查看安装信息
            configWindow->appendTextEdit(tmp);  // 配置包安装信息界面显示配置信息
        }
    }
}

void DebListModel::slotConfigInputWrite(const QString &str)
{
    qCDebug(appLog) << "Writing config input: " << str;
    if (supportCompatible() && m_compProcessor && m_compProcessor->isRunning()) {
        qCDebug(appLog) << "Writing to compatible processor";
        m_compProcessor->writeConfigData(str);
        return;
    }

    if (m_immProcessor && m_immProcessor->isRunning()) {
        qCDebug(appLog) << "Writing to immutable processor";
        m_immProcessor->writeConfigData(str);
        return;
    }

    qCDebug(appLog) << "Writing to pty";
    m_procInstallConfig->pty()->write(str.toUtf8());  // 将用户输入的配置项写入到配置安装进程中。
    m_procInstallConfig->pty()->write("\n");          // 写入换行，配置生效
}

void DebListModel::slotCheckInstallStatus(const QString &installInfo)
{
    qCDebug(appLog) << "Checking install status, installInfo:" << installInfo;
    // 判断当前的信息是否是错误提示信息
    if (installInfo.contains("Error executing command as another user: Request dismissed")) {
        qCDebug(appLog) << "Error executing command as another user: Request dismissed";
        emit signalAppendOutputInfo(installInfo);  // 输出安装错误的原因
        m_workerStatus = WorkerFinished;           // 刷新包安装器的工作状态

        refreshOperatingPackageStatus(Pkg::PackageOperationStatus::Failed);  // 刷新当前包的操作状态

        // 修改map存储的数据格式，将错误原因与错误代码与包绑定，而非与下标绑定
        m_packageOperateStatus[m_operatingPackageMd5] = Pkg::PackageOperationStatus::Failed;
        m_packageFailCode.insert(m_operatingPackageMd5, 0);  // 保存失败原因
        m_packageFailReason.insert(m_operatingPackageMd5, "");
        bumpInstallIndex();
        return;
    }
}

bool DebListModel::recheckPackagePath(const QString &packagePath) const
{
    // qCDebug(appLog) << "Checking package path:" << packagePath;
    QFile packagePathFile(packagePath);
    do {
        if (packagePathFile.symLinkTarget().isEmpty()) {
            if (packagePathFile.exists()) {
                return true;
            }
        } else {
            QFile realPath(packagePathFile.symLinkTarget());
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
    // qCDebug(appLog) << "Getting package MD5s";
    m_packageMd5.clear();
    m_packageMd5 = packagesMD5;
    emit signalAppendFinished();
}

void DebListModel::slotShowProhibitWindow()
{
    qCDebug(appLog) << "Showing prohibit window";
    digitalVerifyFailed(Pkg::ApplocationProhibit);
}
void DebListModel::showProhibitWindow()
{
    qCDebug(appLog) << "Showing prohibit window";
    // 批量安装时，如果不是最后一个包，则不弹窗，只记录详细错误原因。
    if (m_operatingIndex < m_packagesManager->m_preparedPackages.size() - 1) {
        digitalVerifyFailed(Pkg::ApplocationProhibit);  // 刷新安装错误，并记录错误原因
        return;
    }
    DDialog *Ddialog = new DDialog();
    // 设置窗口焦点
    Ddialog->setFocusPolicy(Qt::TabFocus);

    // 设置弹出窗口为模态窗口
    Ddialog->setModal(true);

    // 设置窗口始终置顶
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

    // 窗口退出操作，包括所有可以退出此窗口的操作
    std::function<void(void)> exitOperate = [this, Ddialog]() {
        this->slotShowProhibitWindow();
        Ddialog->deleteLater();
    };

    // 点击弹出窗口的关闭图标按钮
    connect(Ddialog, &DDialog::aboutToClose, exitOperate);

    // 点击弹出窗口的确定按钮
    connect(btnOK, &DPushButton::clicked, exitOperate);

    // ESC
    connect(Ddialog, &DDialog::rejected, exitOperate);
}

bool DebListModel::checkBlackListApplication()
{
    qCDebug(appLog) << "Checking blacklist application";
    PackageDependsStatus dependsStat = m_packagesManager->getPackageDependsStatus(m_operatingIndex);
    if (dependsStat.isProhibit()) {
        qCDebug(appLog) << "Package is in blacklist, showing prohibit window";
        showProhibitWindow();
        return true;
    }
    qCDebug(appLog) << "Package is not in blacklist";
    return false;
}

DebListModel::~DebListModel()
{
    qCDebug(appLog) << "DebListModel destructor";
    delete m_packagesManager;
    delete configWindow;
    delete m_procInstallConfig;
}

/**
   @brief 打印待安装的软件包信息，将根据安装、升级、卸载等分类分别打印对应变更的软件包
 */
void DebListModel::printDependsChanges()
{
    qCDebug(appLog) << "Printing depends changes";
    auto *const backend = PackageAnalyzer::instance().backendPtr();
    if (!backend) {
        qCDebug(appLog) << "Backend is not available";
        return;
    }

    auto changeList = backend->markedPackages();
    if (changeList.isEmpty()) {
        qCDebug(appLog) << "Change list is empty";
        return;
    }

    static QMap<int, QString> tagTable = {{Package::IsManuallyHeld, "Package::IsManuallyHeld"},
                                          {Package::NewInstall, "Package::NewInstall"},
                                          {Package::ToReInstall, "Package::ToReInstall"},
                                          {Package::ToUpgrade, "Package::ToUpgrade"},
                                          {Package::ToDowngrade, "Package::ToDowngrade"},
                                          {Package::ToRemove, "Package::ToRemove"}};
    QMap<int, QStringList> changeInfo;

    for (const Package *package : changeList) {
        int flags = package->state();
        int status = flags & (Package::IsManuallyHeld | Package::NewInstall | Package::ToReInstall | Package::ToUpgrade |
                              Package::ToDowngrade | Package::ToRemove);
        changeInfo[status] << QString("%1, %2, %3").arg(package->name()).arg(package->version()).arg(package->architecture());
    }

    qCInfo(appLog) << "Install depends details:";
    for (auto info = changeInfo.begin(); info != changeInfo.end(); info++) {
        qCInfo(appLog) << tagTable[info.key()] << info.value();
    }
}

QString DebListModel::itemToolTips(int index) const
{
    qCDebug(appLog) << "Getting item tool tips for index:" << index;
    const auto dependStatus = m_packagesManager->getPackageDependsStatus(index);
    if (!dependStatus.canInstall()) {
        static const int kTipsWidth = 440;
        return Utils::formatWrapText(packageFailedReason(index), kTipsWidth);
    }

    return {};
}

void DebListModel::ensureCompatibleProcessor()
{
    qCDebug(appLog) << "Ensuring compatible processor";
    if (!m_compProcessor) {
        m_compProcessor.reset(new Compatible::CompatibleProcessController);

        connect(
            m_compProcessor.data(), &Compatible::CompatibleProcessController::processOutput, this, [this](const QString &output) {
                Q_EMIT signalAppendOutputInfo(output);

                if (configWindow->isVisible()) {
                    configWindow->appendTextEdit(output);
                } else if (m_compProcessor->needTemplates()) {
                    configWindow->appendTextEdit(output);
                    configWindow->show();
                }
            });

        connect(m_compProcessor.data(), &Compatible::CompatibleProcessController::progressChanged, this, [this](float progress) {
            const int progressValue =
                static_cast<int>((100. / m_packagesManager->m_preparedPackages.size()) * (m_operatingIndex + progress / 100.));
            Q_EMIT signalWholeProgressChanged(progressValue);

            Q_EMIT signalCurrentPacakgeProgressChanged(static_cast<int>(progress));
        });

        connect(m_compProcessor.data(), &Compatible::CompatibleProcessController::processFinished, this, [this](bool success) {
            if (configWindow->isVisible()) {
                configWindow->hide();
                configWindow->clearTexts();
            }

            if (success) {
                refreshOperatingPackageStatus(Pkg::Success);
            } else {
                auto pkgPtr = m_compProcessor->currentPackage();
                if (pkgPtr) {
                    m_packageFailCode.insert(m_operatingPackageMd5, pkgPtr->errorCode());
                    m_packageFailReason.insert(m_operatingPackageMd5, pkgPtr->errorString());

                    if (Pkg::DigitalSignatureError == pkgPtr->errorCode()) {
                        m_hierarchicalVerifyError = true;
                    }

                    if (Pkg::ConfigAuthCancel == pkgPtr->errorCode()) {
                        // notify UI reset, cancel current flow
                        m_workerStatus = WorkerPrepare;
                        Q_EMIT signalAuthCancel();
                        refreshOperatingPackageStatus(Pkg::Failed);
                        return;
                    }
                }

                refreshOperatingPackageStatus(Pkg::Failed);
            }

            bumpInstallIndex();
        });
    }
}

bool DebListModel::installCompatiblePackage()
{
    qCDebug(appLog) << "Installing compatible package";
    ensureCompatibleProcessor();

    m_currentPackage = packagePtr(m_operatingIndex);

    return m_compProcessor->install(m_currentPackage);
}

bool DebListModel::uninstallCompatiblePackage()
{
    qCDebug(appLog) << "Uninstalling compatible package";
    ensureCompatibleProcessor();

    m_currentPackage = packagePtr(m_operatingIndex);

    return m_compProcessor->uninstall(m_currentPackage);
}

void DebListModel::ensureImmutableProcessor()
{
    qCDebug(appLog) << "Ensuring immutable processor";
    if (!m_immProcessor) {
        m_immProcessor.reset(new Immutable::ImmutableProcessController);

        connect(
            m_immProcessor.data(), &Immutable::ImmutableProcessController::processOutput, this, [this](const QString &output) {
                Q_EMIT signalAppendOutputInfo(output);

                if (configWindow->isVisible()) {
                    configWindow->appendTextEdit(output);
                } else if (m_immProcessor->needTemplates()) {
                    configWindow->appendTextEdit(output);
                    configWindow->show();
                }
            });

        connect(m_immProcessor.data(), &Immutable::ImmutableProcessController::progressChanged, this, [this](float progress) {
            const int progressValue =
                static_cast<int>((100. / m_packagesManager->m_preparedPackages.size()) * (m_operatingIndex + progress / 100.));
            Q_EMIT signalWholeProgressChanged(progressValue);

            Q_EMIT signalCurrentPacakgeProgressChanged(static_cast<int>(progress));
        });

        connect(m_immProcessor.data(), &Immutable::ImmutableProcessController::processFinished, this, [this](bool success) {
            if (configWindow->isVisible()) {
                configWindow->hide();
                configWindow->clearTexts();
            }

            if (success) {
                refreshOperatingPackageStatus(Pkg::Success);
            } else {
                auto pkgPtr = m_immProcessor->currentPackage();
                if (pkgPtr) {
                    m_packageFailCode.insert(m_operatingPackageMd5, pkgPtr->errorCode());
                    m_packageFailReason.insert(m_operatingPackageMd5, pkgPtr->errorString());

                    if (Pkg::DigitalSignatureError == pkgPtr->errorCode()) {
                        m_hierarchicalVerifyError = true;
                    }

                    if (Pkg::ConfigAuthCancel == pkgPtr->errorCode()) {
                        // notify UI reset, cancel current flow
                        m_workerStatus = WorkerPrepare;
                        Q_EMIT signalAuthCancel();
                        refreshOperatingPackageStatus(Pkg::Failed);
                        return;
                    }
                }

                refreshOperatingPackageStatus(Pkg::Failed);
            }

            bumpInstallIndex();
        });
    }
}

bool DebListModel::installImmutablePackage()
{
    qCDebug(appLog) << "Installing immutable package";
    ensureImmutableProcessor();

    m_currentPackage = packagePtr(m_operatingIndex);

    return m_immProcessor->install(m_currentPackage);
}

bool DebListModel::uninstallImmutablePackage()
{
    qCDebug(appLog) << "Uninstalling immutable package";
    ensureImmutableProcessor();

    m_currentPackage = packagePtr(m_operatingIndex);

    return m_immProcessor->uninstall(m_currentPackage);
}

Deb::DebPackage::Ptr DebListModel::packagePtr(int index) const
{
    // qCDebug(appLog) << "Getting package pointer for index:" << index;
    Deb::DebPackage::Ptr pkgPtr;

    if (0 <= index && index < m_packagesManager->m_packageMd5.size()) {
        const QByteArray md5 = m_packagesManager->m_packageMd5[index];
        if (!m_packagePtrMap.contains(md5)) {
            const QString packagePath = m_packagesManager->m_preparedPackages[index];
            pkgPtr = Deb::DebPackage::Ptr::create(packagePath);

            // temporary code: wait for use DebPackage replace scattered package data
            const_cast<DebListModel *>(this)->m_packagePtrMap.insert(md5, pkgPtr);
        } else {
            pkgPtr = m_packagePtrMap.value(md5);
        }
    }

    return pkgPtr;
}

Dialog::Dialog() {}

void Dialog::keyPressEvent(QKeyEvent *event)
{
    // qCDebug(appLog) << "Dialog key press event";
    if (event->key() == Qt::Key_Escape) {
        qCDebug(appLog) << "Escape key pressed, emitting signalClosed";
        emit signalClosed();
    }
}

void DebListModel::slotUpdateCacheFinished()
{
    qCDebug(appLog) << "Cache update transaction finished";

    Transaction *transaction = qobject_cast<Transaction *>(sender());
    if (!transaction) {
        qCWarning(appLog) << "Update cache transaction is null";
        return;
    }

    disconnect(transaction, &Transaction::finished, this, &DebListModel::slotUpdateCacheFinished);

    if (transaction->exitStatus()) {
        qCWarning(appLog) << "Cache update failed:" << transaction->errorString();
        emit signalAppendOutputInfo(QString("Failed to update cache: %1").arg(transaction->errorString()));
    } else {
        qCDebug(appLog) << "Cache update successful";
        emit signalAppendOutputInfo("Cache updated successfully");
    }

    m_cacheUpdated = true;

    if (!m_currentTransaction.isNull()) {
        m_currentTransaction->deleteLater();
        m_currentTransaction = nullptr;
    }

    // Continue with actual installation after cache update
    qCDebug(appLog) << "Cache update completed, continuing with installation";
    performInstallation();
}
