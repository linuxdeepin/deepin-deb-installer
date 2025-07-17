// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "proxy_package_list_model.h"
#include "deblistmodel.h"
#include "uab/uab_package_list_model.h"
#include "utils/utils.h"
#include "utils/ddlog.h"

ProxyPackageListModel::ProxyPackageListModel(QObject *parent)
    : AbstractPackageListModel{parent}
{
    qCDebug(appLog) << "ProxyPackageListModel constructed.";
    m_supportPackageType = Pkg::Proxy;
}

QVariant ProxyPackageListModel::data(const QModelIndex &index, int role) const
{
    qCDebug(appLog) << "Getting data for proxy index:" << index.row() << "role:" << role;
    if (m_packageModels.empty()) {
        qCWarning(appLog) << "No package models available, returning empty variant.";
        return {};
    }

    auto modelWithIndex = findFromProxyIndex(index.row());
    ModelPtr model = modelWithIndex.first;
    if (!model) {
        qCWarning(appLog) << "Could not find a model for proxy index:" << index.row();
        return {};
    }

    QModelIndex sourceIndex = model->index(modelWithIndex.second);
    qCDebug(appLog) << "Found source model of type" << model->supportPackage() << "and source index" << sourceIndex.row();
    return model->data(sourceIndex, role);
}

bool ProxyPackageListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    qCDebug(appLog) << "Setting data for proxy index:" << index.row() << "role:" << role;
    if (m_packageModels.empty()) {
        qCWarning(appLog) << "No package models available, cannot set data.";
        return false;
    }

    auto modelWithIndex = findFromProxyIndex(index.row());
    ModelPtr model = modelWithIndex.first;
    if (!model) {
        qCWarning(appLog) << "Could not find a model for proxy index:" << index.row();
        return false;
    }

    qCDebug(appLog) << "Found source model of type" << model->supportPackage() << "and source index" << modelWithIndex.second;
    return model->setData(model->index(modelWithIndex.second), value, role);
}

int ProxyPackageListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    qCDebug(appLog) << "Getting row count";

    int count = 0;
    for (const ModelInfo &info : m_packageModels) {
        count += info.model->rowCount();
    }

    qCDebug(appLog) << "Total row count is" << count;
    return count;
}

void ProxyPackageListModel::slotAppendPackage(const QStringList &packageList)
{
    qCDebug(appLog) << "Appending packages:" << packageList;
    QMap<Pkg::PackageType, QStringList> filterList;
    for (const QString &packagePath : packageList) {
        const Pkg::PackageType type = Utils::detectPackage(packagePath);
        qCDebug(appLog) << "Package type detected:" << type << "for" << packagePath;
        filterList[type].append(packagePath);
    };

    for (auto itr = filterList.begin(); itr != filterList.end(); ++itr) {
        switch (itr.key()) {
            case Pkg::Uab:
                Q_FALLTHROUGH();
            case Pkg::Deb: {
                ModelPtr model = modelFromType(itr.key());
                if (model) {
                    model->slotAppendPackage(itr.value());
                }
            } break;
            default:
                return;
        }
    }
}

void ProxyPackageListModel::removePackage(int index)
{
    qCDebug(appLog) << "Removing package at index:" << index;
    auto modelWithIndex = findFromProxyIndex(index);
    if (!modelWithIndex.first) {
        qCWarning(appLog) << "Invalid index for removal:" << index;
        return;
    }

    modelWithIndex.first->removePackage(modelWithIndex.second);
    qCDebug(appLog) << "Package removed successfully";
}

QString ProxyPackageListModel::checkPackageValid(const QString &packagePath)
{
    qCDebug(appLog) << "Checking package validity for:" << packagePath;
    Pkg::PackageType type = Utils::detectPackage(packagePath);
    ModelPtr model = modelFromType(type);

    // TODO: static package check or remove it
    if (!model) {
        qCWarning(appLog) << "No model found for package type:" << type;
        return "";
    }

    const QString result = model->checkPackageValid(packagePath);
    qCDebug(appLog) << "Package validity check result:" << result;
    return result;
}

Pkg::PackageInstallStatus ProxyPackageListModel::checkInstallStatus(const QString &packagePath)
{
    qCDebug(appLog) << "Checking install status for:" << packagePath;
    ModelPtr model = addModelFromFile(packagePath);
    if (model) {
        const auto status = model->checkInstallStatus(packagePath);
        qCDebug(appLog) << "Install status for" << packagePath << "is" << status;
        return status;
    }

    qCWarning(appLog) << "No model found for" << packagePath << ", returning NotInstalled";
    return Pkg::NotInstalled;
}

Pkg::DependsStatus ProxyPackageListModel::checkDependsStatus(const QString &packagePath)
{
    qCDebug(appLog) << "Checking depends status for:" << packagePath;
    ModelPtr model = addModelFromFile(packagePath);
    if (model) {
        const auto status = model->checkDependsStatus(packagePath);
        qCDebug(appLog) << "Depends status for" << packagePath << "is" << status;
        return status;
    }

    qCWarning(appLog) << "No model found for" << packagePath << ", returning DependsOk";
    return Pkg::DependsOk;
}

QStringList ProxyPackageListModel::getPackageInfo(const QString &packagePath)
{
    qCDebug(appLog) << "Getting package info for:" << packagePath;
    ModelPtr model = addModelFromFile(packagePath);
    if (model) {
        const auto info = model->getPackageInfo(packagePath);
        qCDebug(appLog) << "Found package info for" << packagePath;
        return info;
    }

    qCWarning(appLog) << "No model found for" << packagePath << ", returning empty list";
    return {};
}

QString ProxyPackageListModel::lastProcessError()
{
    qCDebug(appLog) << "Getting last process error...";
    QString errorMessage;
    (void)std::find_if(m_packageModels.rbegin(), m_packageModels.rend(), [&errorMessage](const ModelInfo &info) {
        errorMessage = info.model->lastProcessError();
        return !errorMessage.isEmpty();
    });

    if (!errorMessage.isEmpty()) {
        qCWarning(appLog) << "Last process error found:" << errorMessage;
    } else {
        qCDebug(appLog) << "No process error found.";
    }
    return errorMessage;
}

bool ProxyPackageListModel::containsSignatureFailed() const
{
    qCDebug(appLog) << "Checking if any model contains a signature failure...";
    auto findItr = std::find_if(m_packageModels.begin(), m_packageModels.end(), [](const ModelInfo &info) {
        return info.model->containsSignatureFailed();
    });
    const bool failed = findItr != m_packageModels.end();
    qCDebug(appLog) << "Signature failure check result:" << failed;
    return failed;
}

bool ProxyPackageListModel::slotInstallPackages()
{
    qCDebug(appLog) << "Starting package installation...";
    if (!isWorkerPrepare()) {
        qCWarning(appLog) << "Worker not ready for installation";
        return false;
    }

    if (m_packageModels.isEmpty() || m_packageModels.last().rightCount <= 0) {
        qCWarning(appLog) << "No packages to install";
        return false;
    }

    qCDebug(appLog) << "Starting package installation for" << m_packageModels.last().rightCount << "packages";
    setWorkerStatus(WorkerProcessing);
    m_procModelIndex = -1;
    return nextModelInstall();
}

bool ProxyPackageListModel::slotUninstallPackage(int index)
{
    qCDebug(appLog) << "Uninstalling package at proxy index:" << index;
    if (!isWorkerPrepare()) {
        qCWarning(appLog) << "Worker not ready for uninstallation";
        return false;
    }

    auto modelWithIndex = findFromProxyIndex(index);
    if (!modelWithIndex.first) {
        qCWarning(appLog) << "Could not find a model for proxy index:" << index;
        return false;
    }

    qCDebug(appLog) << "Found source model of type" << modelWithIndex.first->supportPackage() << "and source index" << modelWithIndex.second;
    setWorkerStatus(WorkerUnInstall);
    return modelWithIndex.first->slotUninstallPackage(modelWithIndex.second);
}

void ProxyPackageListModel::reset()
{
    qCDebug(appLog) << "Resetting all package models and worker status";
    for (ModelInfo &info : m_packageModels) {
        info.model->reset();
        info.count = 0;
        info.rightCount = 0;
    }
    setWorkerStatus(WorkerPrepare);
}

void ProxyPackageListModel::resetInstallStatus()
{
    qCDebug(appLog) << "Resetting install status for all models";
    for (const ModelInfo &info : m_packageModels) {
        info.model->resetInstallStatus();
    }
    setWorkerStatus(WorkerPrepare);
}

bool ProxyPackageListModel::nextModelInstall()
{
    ++m_procModelIndex;
    qCDebug(appLog) << "Processing next model install, index:" << m_procModelIndex;
    if (m_procModelIndex < 0) {
        qCWarning(appLog) << "Invalid model index, finishing worker.";
        setWorkerStatus(WorkerFinished);
        return false;
    }

    // check if all package finished
    if (m_procModelIndex >= m_packageModels.count()) {
        qCDebug(appLog) << "All models processed, finishing worker.";
        setWorkerStatus(WorkerFinished);
        Q_EMIT signalWholeProgressChanged(100);
        return true;
    }

    const ModelInfo &info = m_packageModels.at(m_procModelIndex);
    if (info.count > 0) {
        qCDebug(appLog) << "Installing packages for model with type:" << info.model->supportPackage();
        return info.model->slotInstallPackages();
    } else {
        qCDebug(appLog) << "No packages for this model, skipping to next.";
        return nextModelInstall();
    }
}

/**
   @return Pointer to an Abstract Package ListModel of a specific type,
        or null if cannot create model.
 */
ProxyPackageListModel::ModelPtr ProxyPackageListModel::modelFromType(Pkg::PackageType type)
{
    qCDebug(appLog) << "Getting model from type:" << type;
    for (const ModelInfo &info : m_packageModels) {
        if (type == info.model->supportPackage()) {
            return info.model;
        }
    }

    qCDebug(appLog) << "No model found for type:" << type << ", adding new model";
    return addModel(type);
}

ProxyPackageListModel::ModelPtr ProxyPackageListModel::addModelFromFile(const QString &packagePath)
{
    qCDebug(appLog) << "Adding model from file:" << packagePath;
    Pkg::PackageType type = Utils::detectPackage(packagePath);
    return modelFromType(type);
}

ProxyPackageListModel::ModelPtr ProxyPackageListModel::addModel(Pkg::PackageType type)
{
    qCDebug(appLog) << "Adding model from type:" << type;
    ModelPtr newModel{nullptr};
    switch (type) {
        case Pkg::Uab:
            qCDebug(appLog) << "Adding UAB model";
            newModel = new Uab::UabPackageListModel(this);
            break;
        case Pkg::Deb:
            qCDebug(appLog) << "Adding DEB model";
            newModel = new DebListModel(this);
            break;
        default:
            break;
    }

    if (newModel) {
        qCDebug(appLog) << "Connecting model:" << newModel;
        connectModel(newModel);

        ModelInfo newInfo;
        newInfo.model = newModel;
        m_packageModels.append(newInfo);

        // sort, ensure deb package model first.
        std::sort(m_packageModels.begin(), m_packageModels.end(), [](const ModelInfo &info1, const ModelInfo &info2) {
            return info1.model->supportPackage() < info2.model->supportPackage();
        });

        // refresh all count
        int rightCount = 0;
        for (ModelInfo &info : m_packageModels) {
            info.count = info.model->rowCount();
            info.rightCount = rightCount + info.count;
            rightCount = info.rightCount;
        }
    }

    return newModel;
}

void ProxyPackageListModel::connectModel(ModelPtr model)
{
    qCDebug(appLog) << "Connecting model:" << model;
    if (!model) {
        qCWarning(appLog) << "Invalid model to connect";
        return;
    }

    // pass-through
    QObject::connect(model, &AbstractPackageListModel::signalAppendStart, this, &ProxyPackageListModel::signalAppendStart);
    QObject::connect(model, &AbstractPackageListModel::signalAppendFinished, this, &ProxyPackageListModel::signalAppendFinished);
    QObject::connect(
        model, &AbstractPackageListModel::signalAppendFailMessage, this, &ProxyPackageListModel::signalAppendFailMessage);
    QObject::connect(
        model, &AbstractPackageListModel::signalAppendOutputInfo, this, &ProxyPackageListModel::signalAppendOutputInfo);
    QObject::connect(
        model, &AbstractPackageListModel::signalPackageCannotFind, this, &ProxyPackageListModel::signalPackageCannotFind);

    QObject::connect(model, &AbstractPackageListModel::signalLockForAuth, this, &ProxyPackageListModel::signalLockForAuth);
    QObject::connect(model, &AbstractPackageListModel::signalAuthCancel, this, [this]() {
        // if auth canceled, reset to prepare state.
        setWorkerStatus(WorkerPrepare);
        Q_EMIT signalAuthCancel();
    });
    QObject::connect(
        model, &AbstractPackageListModel::signalEnableReCancelBtn, this, &ProxyPackageListModel::signalEnableReCancelBtn);
    QObject::connect(model, &AbstractPackageListModel::signalDependResult, this, &ProxyPackageListModel::signalDependResult);
    QObject::connect(
        model, &AbstractPackageListModel::signalEnableCloseButton, this, &ProxyPackageListModel::signalEnableCloseButton);

    // pass-through, only single page need current package progress
    QObject::connect(model,
                     &AbstractPackageListModel::signalCurrentPacakgeProgressChanged,
                     this,
                     &ProxyPackageListModel::signalCurrentPacakgeProgressChanged);

    // signals forwarded through proxy
    QObject::connect(
        model, &AbstractPackageListModel::signalPackageCountChanged, this, &ProxyPackageListModel::onSourcePacakgeCountChanged);
    QObject::connect(
        model, &AbstractPackageListModel::signalWholeProgressChanged, this, &ProxyPackageListModel::onSourceWholeProgressChanged);
    QObject::connect(model,
                     &AbstractPackageListModel::signalCurrentProcessPackageIndex,
                     this,
                     &ProxyPackageListModel::onSourceCurrentProcessPackageIndex);
    // signalWorkerStart() does not need to be forwarded from the model being proxied, managed by proxy package list model.
    QObject::connect(model, &AbstractPackageListModel::signalWorkerFinished, this, &ProxyPackageListModel::onSoureWorkerFinished);

    // qt interface
    QObject::connect(model, &QAbstractListModel::dataChanged, this, &ProxyPackageListModel::onSourceDataChanged);
}

QPair<ProxyPackageListModel::ModelPtr, int> ProxyPackageListModel::findFromProxyIndex(int proxyIndex) const
{
    qCDebug(appLog) << "Finding from proxy index:" << proxyIndex;
    if (m_packageModels.isEmpty()) {
        qCDebug(appLog) << "No package models available";
        return qMakePair(nullptr, -1);
    }

    if (proxyIndex < 0 || proxyIndex >= m_packageModels.last().rightCount) {
        qCDebug(appLog) << "Proxy index is out of range";
        return qMakePair(nullptr, -1);
    }

    auto findItr =
        std::lower_bound(m_packageModels.begin(), m_packageModels.end(), proxyIndex, [](const ModelInfo &info, int proxyIndex) {
            return info.rightCount <= proxyIndex;
        });

    if (findItr != m_packageModels.end()) {
        const int sourceIndex = proxyIndex - ((*findItr).rightCount - (*findItr).count);
        qCDebug(appLog) << "Found model:" << (*findItr).model << "and source index:" << sourceIndex;
        return qMakePair((*findItr).model, sourceIndex);
    }
    qCDebug(appLog) << "No model found for proxy index:" << proxyIndex;
    return qMakePair(nullptr, -1);
}

int ProxyPackageListModel::proxyIndexFromModel(ModelPtr findModel, int index)
{
    qCDebug(appLog) << "Getting proxy index from model:" << findModel << "and index:" << index;
    if (m_packageModels.isEmpty()) {
        qCDebug(appLog) << "No package models available";
        return -1;
    }

    if (index < 0 || index >= m_packageModels.last().rightCount) {
        qCDebug(appLog) << "Index is out of range";
        return -1;
    }

    for (const ModelInfo &info : m_packageModels) {
        if (findModel == info.model) {
            if (info.count > index) {
                return info.rightCount - info.count + index;
            }

            break;
        }
    }

    qCDebug(appLog) << "No model found for proxy index:" << index;
    return -1;
}

void ProxyPackageListModel::onSourcePacakgeCountChanged(int count)
{
    qCDebug(appLog) << "On source package count changed:" << count;
    const auto sendModel = qobject_cast<ModelPtr>(sender());
    if (sendModel && !m_packageModels.isEmpty()) {
        qCDebug(appLog) << "Found model:" << sendModel;
        int newCount = 0;
        for (ModelInfo &info : m_packageModels) {
            if (sendModel == info.model) {
                info.count = count;
                // not break, update remaining model count.
            }

            newCount += info.count;
            info.rightCount = newCount;
        }

        qCDebug(appLog) << "New count:" << newCount;
        Q_EMIT signalPackageCountChanged(m_packageModels.last().rightCount);

    } else {
        qCWarning(appLog) << "Invalid model signal sender for package count change";
    }
}

void ProxyPackageListModel::onSourceWholeProgressChanged(int progress)
{
    qCDebug(appLog) << "On source whole progress changed:" << progress;
    const auto sendModel = qobject_cast<ModelPtr>(sender());
    if (sendModel && !m_packageModels.empty()) {
        qCDebug(appLog) << "Found model:" << sendModel;
        if (m_procModelIndex < 0 || m_procModelIndex >= m_packageModels.count()) {
            return;
        }

        const ModelInfo &info = m_packageModels.at(m_procModelIndex);
        if (info.model != sendModel) {
            return;
        }

        const int allCount = m_packageModels.last().rightCount;
        const int wholeProgress = static_cast<int>(progress * (info.count * 1.0f / allCount));
        qCDebug(appLog) << "Whole progress:" << wholeProgress;
        Q_EMIT signalWholeProgressChanged(wholeProgress);

    } else {
        qCWarning(appLog) << "Invalid model signal sender for progress change";
    }
}

void ProxyPackageListModel::onSourceCurrentProcessPackageIndex(int index)
{
    qCDebug(appLog) << "On source current process package index:" << index;
    const auto sendModel = qobject_cast<ModelPtr>(sender());
    if (sendModel && !m_packageModels.empty()) {
        qCDebug(appLog) << "Found model:" << sendModel;
        if (m_procModelIndex < 0 || m_procModelIndex >= m_packageModels.count()) {
            return;
        }

        const ModelInfo &info = m_packageModels.at(m_procModelIndex);
        if (info.model != sendModel) {
            return;
        }

        const int proxyIndex = proxyIndexFromModel(sendModel, index);
        qCDebug(appLog) << "Proxy index:" << proxyIndex;
        Q_EMIT signalCurrentProcessPackageIndex(proxyIndex);

    } else {
        qCWarning(appLog) << "Invalid model signal sender for process index";
    }
}

void ProxyPackageListModel::onSoureWorkerFinished()
{
    qCDebug(appLog) << "On source worker finished";
    if (!m_packageModels.empty()) {
        qCDebug(appLog) << "Found model:" << m_packageModels.last().model;
        switch (m_workerStatus) {
            case AbstractPackageListModel::WorkerProcessing:
                nextModelInstall();
                break;
            case AbstractPackageListModel::WorkerUnInstall:
                setWorkerStatus(WorkerFinished);
                break;
            default:
                break;
        }

    } else {
        qCWarning(appLog) << "Invalid model signal sender for worker finished";
    }
}

void ProxyPackageListModel::onSourceDataChanged(const QModelIndex &topLeft,
                                                const QModelIndex &bottomRight,
                                                const QVector<int> &roles)
{
    qCDebug(appLog) << "On source data changed:" << topLeft << bottomRight << roles;
    for (const ModelInfo &info : m_packageModels) {
        if (topLeft.model() == info.model) {
            qCDebug(appLog) << "Found model:" << info.model;
            if (topLeft.row() > info.count || bottomRight.row() > info.count) {
                return;
            }

            const int leftCount = info.rightCount - info.count;
            const int proxyTopIndex = leftCount + topLeft.row();
            const int proxyBottomIndex = leftCount + bottomRight.row();

            // check index valid interal
            const QModelIndex proxyTopLeft = this->index(proxyTopIndex);
            const QModelIndex proxyBottomRight = this->index(proxyBottomIndex);

            if (proxyTopLeft.isValid() && proxyBottomRight.isValid()) {
                qCDebug(appLog) << "Data changed in proxy model:" << proxyTopLeft << proxyBottomRight << roles;
                Q_EMIT dataChanged(proxyTopLeft, proxyBottomRight, roles);
            }

            qCDebug(appLog) << "return: Data changed in proxy model:" << topLeft << bottomRight << roles;
            return;
        }
    }
}
