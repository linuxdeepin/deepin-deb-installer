// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "packageselectmodel.h"
#include "model/packageanalyzer.h"
#include "utils/ddlog.h"

#include <QStandardItemModel>
#include <QtConcurrent/QtConcurrent>
#include <QApplication>

#include <QApt/DebFile>

PackageSelectModel::PackageSelectModel(QObject *parent)
    : QObject(parent)
    , model(new QStandardItemModel(this))
{
    qCDebug(appLog) << "PackageSelectModel constructed.";
}

QStandardItemModel *PackageSelectModel::viewModel() const
{
    // qCDebug(appLog) << "Returning view model.";
    return model;
}

void PackageSelectModel::appendDdimPackages(const QList<DdimSt> &ddims)
{
    qCDebug(appLog) << "Appending" << ddims.size() << "ddim packages";
    for (const auto &ddim : ddims) {
        qCDebug(appLog) << "Processing ddim from dir:" << ddim.dirPath << "with token:" << ddim.token;
        // 初始化ddim ir包
        DdimIrPackage ddimIrPkg;
        ddimIrPkg.dirPath = ddim.dirPath;
        ddimIrPkg.version = ddim.version;
        ddimIrPkg.token = ddim.token;

        // 逐个转换为IR模式
        // 去重优先级：必装 > 选装 > 依赖
        PackageAnalyzer::instance().startPkgAnalyze(ddim.mustInstallList.size() + ddim.selectList.size() +
                                                    ddim.dependList.size());

        QSet<QByteArray> md5s;
        auto currentMustInstallInfos =
            PackageAnalyzer::instance().analyzeDebFiles(ddim.mustInstallList, &md5s, nullptr, true, true);
        qCDebug(appLog) << "Analyzed must-install list, got" << currentMustInstallInfos.size() << "packages.";
        QStringList selectAppNameList = ddim.selectAppNameList;
        auto currentSelectInfos =
            PackageAnalyzer::instance().analyzeDebFiles(ddim.selectList, &md5s, &selectAppNameList, false, false);
        qCDebug(appLog) << "Analyzed select list, got" << currentSelectInfos.size() << "packages.";
        auto currentDependInfos = PackageAnalyzer::instance().analyzeDebFiles(ddim.dependList, &md5s, nullptr, true, false);
        qCDebug(appLog) << "Analyzed depend list, got" << currentDependInfos.size() << "packages.";

        PackageAnalyzer::instance().stopPkgAnalyze();

        for (int i = 0; i != currentSelectInfos.size(); ++i) {
            currentSelectInfos[i].appName = selectAppNameList[i];
        }

        ddimIrPkg.selectInfos = currentSelectInfos;
        ddimIrPkg.dependInfos = currentDependInfos;
        ddimIrPkg.mustInstallInfos = currentMustInstallInfos;

        // 查询是否有相同token的ddim，有相同的执行更新，没有相同的执行新增
        auto ddimIter = std::find_if(ddimIrs.begin(), ddimIrs.end(), [ddimIrPkg](const DdimIrPackage &currentDdimIrPkg) {
            return currentDdimIrPkg.token == ddimIrPkg.token;
        });
        if (ddimIter != ddimIrs.end()) {
            qCDebug(appLog) << "Updating existing ddim package with token:" << ddimIrPkg.token;
            *ddimIter = ddimIrPkg;
        } else {
            qCDebug(appLog) << "Adding new ddim package with token:" << ddimIrPkg.token;
            ddimIrs.push_back(ddimIrPkg);
        }
    }

    // 汇聚全部数据（依据ddimIrs目前的情况，刷新selectInfos，dependInfos，mustInstallInfos）
    if (collectData()) {
        qCDebug(appLog) << "Select infos changed, emitting signal";
        emit selectInfosChanged(selectInfos);  // 刷新选择界面
    } else {
        qCDebug(appLog) << "Select infos unchanged";
        emit selectInfosDoNotHaveChange();  // 告知没有必要刷新选择界面
    }
}

QList<DebIr> PackageSelectModel::analyzePackageInstallNeeded(const QList<int> &selectIndexes) const
{
    qCDebug(appLog) << "Analyzing install needs for" << selectIndexes.size() << "selected packages";
    QList<DebIr> installIrs;
    QList<DebIr> dependIrs;

    // 1.筛选需要安装的可选包
    for (int i : selectIndexes) {
        installIrs.push_back(selectInfos.at(i));
    }
    qCDebug(appLog) << "Selected" << installIrs.size() << "optional packages to install.";

    // 2.汇集可选包和必装包为需要安装的包
    qCDebug(appLog) << "Adding" << mustInstallInfos.size() << "must-install packages.";
    installIrs.append(mustInstallInfos);
    dependIrs = dependInfos;
    qCDebug(appLog) << "Total packages to consider for installation:" << installIrs.size();
    qCDebug(appLog) << "Total available dependency packages:" << dependIrs.size();

    // 3.将需要安装的包和本地可用依赖包传入分析工具，获取最佳安装顺序（原则是能不装就不装，确保安装过程最小化）
    qCDebug(appLog) << "Starting package analyzer to determine best install queue...";
    PackageAnalyzer::instance().startPkgAnalyze(installIrs.size());
    auto result = PackageAnalyzer::instance().bestInstallQueue(installIrs, dependIrs);
    PackageAnalyzer::instance().stopPkgAnalyze();
    qCDebug(appLog) << "Package analysis finished.";

    qCDebug(appLog) << "Best install queue contains" << result.size() << "packages";
    return result;
}

template <typename Func>
void getDebIrs(QList<DebIr> *container, QSet<QString> *md5s, const QList<DdimIrPackage> &ddimIrs, Func &&dataGetter)
{
    for (auto &eachIrPkg : ddimIrs) {
        auto infos = dataGetter(eachIrPkg);
        for (auto &eachInfo : infos) {
            if (!md5s->contains(eachInfo.md5)) {
                // qCDebug(appLog) << "Adding unique package to container:" << eachInfo.filePath;
                md5s->insert(eachInfo.md5);
                container->push_back(eachInfo);
            }
        }
    }
}

bool PackageSelectModel::collectData()
{
    qCDebug(appLog) << "Collecting package data from" << ddimIrs.size() << "ddim packages";
    // 1.构建新数据包，由于只需要监控可选项，因此其它两个直接清空
    QList<DebIr> newSelectInfos;
    dependInfos.clear();
    mustInstallInfos.clear();

    // 2.处理ddimIrs的数据，需要依据包的md5进行去重
    QSet<QString> md5s;

    // 2.1汇聚必装包
    qCDebug(appLog) << "Collecting must-install packages...";
    getDebIrs(&mustInstallInfos, &md5s, ddimIrs, [](const DdimIrPackage &info) { return info.mustInstallInfos; });
    qCDebug(appLog) << "Found" << mustInstallInfos.size() << "unique must-install packages.";

    // 2.2汇聚可选包
    qCDebug(appLog) << "Collecting selectable packages...";
    getDebIrs(&newSelectInfos, &md5s, ddimIrs, [](const DdimIrPackage &info) { return info.selectInfos; });
    qCDebug(appLog) << "Found" << newSelectInfos.size() << "unique selectable packages.";

    // 2.3汇聚依赖包
    qCDebug(appLog) << "Collecting dependency packages...";
    getDebIrs(&dependInfos, &md5s, ddimIrs, [](const DdimIrPackage &info) { return info.dependInfos; });
    qCDebug(appLog) << "Found" << dependInfos.size() << "unique dependency packages.";

    // 3.查看是否存在改变
    bool ret = false;
    if (newSelectInfos.size() != selectInfos.size()) {
        qCDebug(appLog) << "Select info size changed, new:" << newSelectInfos.size() << "old:" << selectInfos.size();
        ret = true;
    } else {
        for (int i = 0; i < newSelectInfos.size(); ++i) {
            if (!(newSelectInfos[i] == selectInfos[i])) {
                qCDebug(appLog) << "Select info content changed at index:" << i;
                ret = true;
                break;
            }
        }
    }

    // 4.清理老数据
    selectInfos = newSelectInfos;

    qCDebug(appLog) << "Data collection completed, changes detected:" << ret;
    return ret;
}
