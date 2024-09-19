// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uab_package_list_model.h"

#include <QAbstractListModel>

#include "uab_backend.h"
#include "utils/utils.h"

#include <QDebug>

namespace Uab {

static const float kCompleteProgress = 100.0;
static const int kPkgInitedIndex = -1;

UabPackageListModel::UabPackageListModel(QObject *parent)
    : AbstractPackageListModel{parent}
    , m_processor(new UabProcessController(this))
{
    m_supportPackageType = Pkg::Uab;

    connect(m_processor, &UabProcessController::processOutput, this, &UabPackageListModel::signalAppendOutputInfo);
    connect(m_processor, &UabProcessController::progressChanged, this, &UabPackageListModel::slotBackendProgressChanged);
    connect(m_processor, &UabProcessController::processFinished, this, &UabPackageListModel::slotBackendProcessFinished);

    // call init uab backend while uab package list model create
    connect(Uab::UabBackend::instance(), &Uab::UabBackend::backendInitFinsihed, this, [this]() {
        if (!m_delayAppendPackages.isEmpty()) {
            slotAppendPackage(m_delayAppendPackages);
        }
        m_delayAppendPackages.clear();
    });
    Uab::UabBackend::instance()->initBackend();
}

QVariant UabPackageListModel::data(const QModelIndex &index, int role) const
{
    if (!QAbstractListModel::checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return {};
    }

    const UabPackage::Ptr &uabPtr = m_uabPkgList.at(index.row());
    if (!uabPtr) {
        return {};
    }

    switch (role) {
        case PackageNameRole:
            return uabPtr->info()->appName;
        case PackagePathRole:
            return uabPtr->info()->filePath;
        case PackageVersionRole:
            return uabPtr->info()->version;
        case PackageInstalledVersionRole:
            return uabPtr->installedVersion();
        case PackageShortDescriptionRole:
            Q_FALLTHROUGH();
        case PackageLongDescriptionRole:
            return uabPtr->info()->shortDescription;
        case PackageVersionStatusRole:
            return uabPtr->installStatus();
        case PackageDependsStatusRole:
            return uabPtr->dependsStatus();
        case PackageFailReasonRole:
            return uabPtr->failedReason();
        case PackageOperateStatusRole:
            return uabPtr->operationStatus();
        default:
            break;
    }

    return {};
}

int UabPackageListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_uabPkgList.size();
}

void UabPackageListModel::slotAppendPackage(const QStringList &packageList)
{
    if (!isWorkerPrepare()) {
        return;
    }

    // delay append when uab backend not ready. sa backendInitFinsihed()
    if (!Uab::UabBackend::instance()->backendInited()) {
        m_delayAppendPackages.append(packageList);
        return;
    }

    const int oldRowCount = rowCount();
    for (const QString &path : packageList) {
        auto uabPtr = preCheckPackage(path);

        if (uabPtr && uabPtr->isValid()) {
            m_uabPkgList.append(uabPtr);
        }
    }

    if (oldRowCount != rowCount()) {
        Q_EMIT signalPackageCountChanged(rowCount());
    }
}

void UabPackageListModel::removePackage(int index)
{
    if (!isWorkerPrepare()) {
        return;
    }

    if (0 <= index && index < rowCount()) {
        m_uabPkgList.removeAt(index);

        Q_EMIT signalPackageCountChanged(rowCount());
    }
}

QString UabPackageListModel::checkPackageValid(const QString &packagePath)
{
    if (Pkg::PkgReadable != Utils::checkPackageReadable(packagePath)) {
        return "You can only install local ueb packages";
    }

    auto uabPtr = Uab::UabPackage::fromFilePath(packagePath);
    if (!uabPtr->isValid()) {
        return "The uab package may be broken";
    }

    if (packageExists(uabPtr)) {
        return "The uab package already added";
    }

    return "";
}

void UabPackageListModel::slotInstallPackages()
{
    if (!isWorkerPrepare() || rowCount() <= 0) {
        return;
    }

    // reset, mark package waiting install
    resetInstallStatus();
    for (auto uabPtr : m_uabPkgList) {
        uabPtr->m_operationStatus = Pkg::Waiting;
        Q_EMIT dataChanged(index(0), index(m_uabPkgList.size() - 1), {PackageOperateStatusRole});
    }

    setWorkerStatus(WorkerProcessing);
    installNextUab();
}

void UabPackageListModel::slotUninstallPackage(const int i)
{
    bool callRet = false;

    do {
        if (!isWorkerPrepare() || rowCount() <= 0) {
            break;
        }

        if (!checkIndex(index(i))) {
            break;
        }

        // only single page show uninstall flow
        m_operatingIndex = i;
        setWorkerStatus(WorkerUnInstall);

        auto uabPtr = m_uabPkgList.value(m_operatingIndex);
        if (!uabPtr || !uabPtr->isValid()) {
            break;
        }
        auto removeInfoPtr = Uab::UabBackend::instance()->findPackage(uabPtr->info()->id);
        if (!removeInfoPtr) {
            break;
        }

        setCurrentOperation(Pkg::Waiting);

        m_processor->reset();
        m_processor->markUninstall(removeInfoPtr);
        callRet = m_processor->commitChanges();
    } while (false);

    if (!callRet) {
        setCurrentOperation(Pkg::Failed);
        setWorkerStatus(WorkerFinished);
    }
}

void UabPackageListModel::reset()
{
    setWorkerStatus(WorkerPrepare);
    m_operatingIndex = kPkgInitedIndex;
    m_uabPkgList.clear();
}

void UabPackageListModel::resetInstallStatus()
{
    setWorkerStatus(WorkerPrepare);
    m_operatingIndex = kPkgInitedIndex;

    // reset deb file
    for (auto &ptr : m_uabPkgList) {
        ptr->reset();
    }
    Q_EMIT dataChanged(index(0), index(m_uabPkgList.size() - 1), {PackageOperateStatusRole});
}

void UabPackageListModel::installNextUab()
{
    m_operatingIndex++;
    Q_ASSERT_X(m_operatingIndex >= 0, "install uab", "operating index invalid");
    if (m_operatingIndex < 0) {
        setWorkerStatus(WorkerFinished);
        return;
    }

    // check finish
    if (m_operatingIndex >= rowCount()) {
        Q_EMIT signalCurrentPacakgeProgressChanged(kCompleteProgress);
        Q_EMIT signalWholeProgressChanged(kCompleteProgress);

        setWorkerStatus(WorkerFinished);
        return;
    }

    // notify list view scroll to current package
    Q_EMIT signalCurrentProcessPackageIndex(m_operatingIndex);

    auto uabPtr = m_uabPkgList.value(m_operatingIndex);
    if (!uabPtr || Pkg::DependsOk != uabPtr->m_dependsStatus) {
        setCurrentOperation(Pkg::Failed);
        installNextUab();
        return;
    }

    setCurrentOperation(Pkg::Operating);

    m_processor->reset();
    switch (uabPtr->installStatus()) {
        case Pkg::NotInstalled:
            m_processor->markInstall(uabPtr->info());
            break;
        case Pkg::InstalledSameVersion: {
            auto oldInfoPtr = Uab::UabBackend::instance()->findPackage(uabPtr->info()->id);
            m_processor->markUninstall(oldInfoPtr);
            m_processor->markInstall(uabPtr->info());
        } break;
        default: {
            auto oldInfoPtr = Uab::UabBackend::instance()->findPackage(uabPtr->info()->id);
            m_processor->markInstall(uabPtr->info());
            m_processor->markUninstall(oldInfoPtr);
        } break;
    }

    if (!m_processor->commitChanges()) {
        setCurrentOperation(Pkg::Failed);
        installNextUab();
        return;
    }
}

void UabPackageListModel::slotBackendProgressChanged(float progress)
{
    Q_ASSERT_X(rowCount() > 0, "check count", "row count invalid");

    float base = kCompleteProgress / rowCount();
    float wholeProgress = (m_operatingIndex + (progress / kCompleteProgress)) * base;

    Q_EMIT signalCurrentPacakgeProgressChanged(progress);
    Q_EMIT signalWholeProgressChanged(wholeProgress);
}

void UabPackageListModel::slotBackendProcessFinished(bool success)
{
    Q_ASSERT_X(rowCount() > 0, "check count", "row count invalid");

    // update installed status
    setCurrentOperation(success ? Pkg::Success : Pkg::Failed);

    // update progress
    float base = kCompleteProgress / rowCount();
    Q_EMIT signalCurrentPacakgeProgressChanged(kCompleteProgress);
    Q_EMIT signalWholeProgressChanged(base * (m_operatingIndex + 1));

    switch (m_workerStatus) {
        case WorkerProcessing:
            installNextUab();
            break;
        case WorkerUnInstall:
            setWorkerStatus(WorkerFinished);
            break;
        default:
            break;
    }
}

void UabPackageListModel::setCurrentOperation(Pkg::PackageOperationStatus s)
{
    if (!checkIndexValid(m_operatingIndex)) {
        return;
    }

    auto &uabPtr = m_uabPkgList[m_operatingIndex];
    uabPtr->m_operationStatus = s;

    Q_EMIT dataChanged(index(m_operatingIndex), index(m_operatingIndex), {PackageOperateStatusRole});
}

bool UabPackageListModel::checkIndexValid(int index) const
{
    return 0 <= index && index < rowCount();
}

UabPackage::Ptr UabPackageListModel::preCheckPackage(const QString &packagePath)
{
    auto readablilty = Utils::checkPackageReadable(packagePath);
    switch (readablilty) {
        case Pkg::PkgNotInLocal:
            emit signalAppendFailMessage(Pkg::PackageNotLocal);
            return {};
        case Pkg::PkgNoPermission:
            emit signalAppendFailMessage(Pkg::PackageNotInstallable);
            return {};
        default:
            break;
    }

    auto uabPtr = Uab::UabPackage::fromFilePath(packagePath);
    if (!uabPtr || !uabPtr->isValid()) {
        Q_EMIT signalAppendFailMessage(Pkg::PackageInvalid);
        return {};
    }

    if (packageExists(uabPtr)) {
        Q_EMIT signalAppendFailMessage(Pkg::PackageAlreadyExists);
        return {};
    }

    return uabPtr;
}

bool UabPackageListModel::packageExists(const UabPackage::Ptr &uabPtr)
{
    if (!uabPtr) {
        return false;
    }

    // check already added
    auto findItr = std::find_if(m_uabPkgList.begin(), m_uabPkgList.end(), [&uabPtr](const UabPackage::Ptr &cmpUabPtr) {
        if (uabPtr->info()->filePath == cmpUabPtr->info()->filePath) {
            return true;
        }

        if (uabPtr->info()->appName == cmpUabPtr->info()->appName) {
            return true;
        }

        return false;
    });

    return findItr != m_uabPkgList.end();
}

}  // namespace Uab
