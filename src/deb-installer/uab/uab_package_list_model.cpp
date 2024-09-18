// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uab_package_list_model.h"

#include <QAbstractListModel>

#include "uab_backend.h"
#include "utils/utils.h"

#include <QDebug>

namespace Uab {

static const float kCompleteProgress = 100.0f;
static const int kPkgInitedIndex = -1;

UabPackageListModel::UabPackageListModel(QObject *parent)
    : AbstractPackageListModel{parent}
    , m_processor(new UabProcessController(this))
{
    connect(m_processor, &UabProcessController::processOutput, this, &UabPackageListModel::signalAppendOutputInfo);
    connect(m_processor, &UabProcessController::progressChanged, this, &UabPackageListModel::slotBackendProgressChanged);
    connect(m_processor, &UabProcessController::processFinished, this, &UabPackageListModel::slotBackendProcessFinished);
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

    const int oldRowCount = rowCount();
    for (const QString &path : packageList) {
        auto uabPtr = Uab::UabPackage::fromFilePath(path);
        if (uabPtr && uabPtr->isValid()) {
            m_uabPkgList.append(uabPtr);
        }
    }

    if (oldRowCount != rowCount()) {
        Q_EMIT signalPackageCountChanged(rowCount());
    }
}

void UabPackageListModel::removePackage(const int index)
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
    if (!Utils::checkPackageReadable(packagePath)) {
        return "You can only install local ueb packages";
    }

    auto uabPtr = Uab::UabPackage::fromFilePath(packagePath);
    if (!uabPtr->isValid()) {
        return "The uab package may be broken";
    }

    // check alreay added
    auto findItr = std::find_if(m_uabPkgList.begin(), m_uabPkgList.end(), [&uabPtr](const UabPackage::Ptr &cmpUabPtr) {
        if (uabPtr->info()->filePath == cmpUabPtr->info()->filePath) {
            return true;
        }

        if (uabPtr->info()->appName == cmpUabPtr->info()->appName) {
            return true;
        }

        return false;
    });

    if (findItr != m_uabPkgList.end()) {
        return "The uab package Already Added";
    }

    return "";
}

void UabPackageListModel::slotInstallPackages()
{
    if (!isWorkerPrepare() || rowCount() <= 0) {
        setWorkerStatus(WorkerPrepare);
        Q_EMIT signalWorkerFinished();
        return;
    }

    // reset
    resetInstallStatus();

    setWorkerStatus(WorkerProcessing);
    Q_EMIT signalWorkerStart();

    installNextUab();
}

void UabPackageListModel::slotUninstallPackage(const int i)
{
    bool callRet = false;

    do {
        if (WorkerUnInstall != m_workerStatus || rowCount() <= 0) {
            break;
        }

        if (!checkIndex(index(i))) {
            break;
        }

        // reset
        m_operatingIndex = i;

        // only single page show uninstall flow
        Q_EMIT signalWorkerStart();

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
        setWorkerStatus(WorkerPrepare);
        Q_EMIT signalWorkerFinished();
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
        setWorkerStatus(WorkerPrepare);
        Q_EMIT signalWorkerFinished();
        return;
    }

    // check finish
    if (m_operatingIndex >= rowCount()) {
        setWorkerStatus(WorkerPrepare);

        Q_EMIT signalCurrentPacakgeProgressChanged(kCompleteProgress);
        Q_EMIT signalWholeProgressChanged(kCompleteProgress);
        Q_EMIT signalWorkerFinished();
        return;
    }

    auto uabPtr = m_uabPkgList.value(m_operatingIndex);
    if (!uabPtr || Pkg::DependsOk != uabPtr->m_dependsStatus) {
        setCurrentOperation(Pkg::Failed);
        installNextUab();
        return;
    }

    setCurrentOperation(Pkg::Waiting);

    m_processor->reset();
    switch (uabPtr->installStatus()) {
        case Pkg::NotInstalled:
            m_processor->markInstall(uabPtr->info());
            break;
        case Pkg::InstalledSameVersion: {
            auto oldInfoPtr = Uab::UabBackend::instance()->findPackage(uabPtr->info()->id);
            m_processor->markUninstall(oldInfoPtr);
            m_processor->markInstall(oldInfoPtr);
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
            setWorkerStatus(WorkerPrepare);
            Q_EMIT signalWorkerFinished();
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

    auto uabPtr = m_uabPkgList.value(m_operatingIndex);
    uabPtr->m_operationStatus = s;

    Q_EMIT dataChanged(index(m_operatingIndex), index(m_operatingIndex), {PackageOperateStatusRole});
}

bool UabPackageListModel::checkIndexValid(int index) const
{
    return 0 <= index && index < rowCount();
}

}  // namespace Uab
