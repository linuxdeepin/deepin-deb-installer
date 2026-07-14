// SPDX-FileCopyrightText: 2023-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PACKAGEANALYZER_H
#define PACKAGEANALYZER_H

#include <QSharedPointer>
#include <QObject>
#include <QDateTime>
#include <QFileInfo>

#include <atomic>

#include "model/packageselectmodel.h"
#include "utils/package_defines.h"

namespace QApt {
class Backend;
class Package;
class Transaction;
}  // namespace QApt

class PackageAnalyzer : public QObject
{
    Q_OBJECT

public:
    static PackageAnalyzer &instance();

    // 异步信号

    // 告知外部的UI已经退出了
    void setUiExit();

    // 初始化阶段

    void initBackend();
    void updatePackageCache();
    bool isBackendReady();
    QApt::Backend *backendPtr();

    // 软件包缓存更新完成（apt 锁已释放，可安全执行 apt install 等操作）
    void setCacheUpdateFinished(bool finished);
    bool isCacheUpdateFinished() const;

    // 判断是否需要更新软件包缓存
    // 基于文件时间戳比较：比较源文件和缓存文件的修改时间
    bool shouldUpdateCache() const;

    // 选择阶段

    // 当前APT支持的架构
    bool supportArch(const QString &arch) const { return archs.contains(arch); }
    inline QStringList supportArchList() const { return archs; }

    // 软件包安装状态，first:安装状态，secend:已安装版本（当状态不为NotInstalled时有效）
    QPair<Pkg::PackageInstallStatus, QString> packageInstallStatus(const DebIr &ir) const;

    // 分析包结构

    // 启动/停止包分析，内部会根据分析进度对外发送信号
    void startPkgAnalyze(int total);
    void stopPkgAnalyze();

    // 参数：文件信息，md5码过滤集，应用名，是否去除架构不满足项，是否去除已安装或安装了高版本的项
    QList<DebIr> analyzeDebFiles(const QFileInfoList &infos,
                                 QSet<QByteArray> *md5s,
                                 QStringList *appNames,
                                 bool excludeArchNotMatched,
                                 bool excludeInstalledOrLaterVersion);

    // 提取未安装依赖，传入包名和对应的架构，返回未安装的依赖列表
    QList<QApt::DependencyItem> debDependNotInstalled(const DebIr &ir) const;

    // 依赖是否就绪
    bool dependIsReady(const QApt::DependencyItem &depend) const;

    // 虚拟包是否已安装
    bool virtualPackageIsExist(const QString &virtualPackageName) const;

    // 版本号是否匹配
    // rhs是否以relationType的方式匹配lhs
    bool versionMatched(const QString &lhs, const QString &rhs, QApt::RelationType relationType) const;

    // 抽取需要的依赖包
    void chooseDebFromDepend(QList<DebIr> *result,
                             QSet<QByteArray> *md5s,
                             const QList<QApt::DependencyItem> &depends,
                             const QList<DebIr> &debIrs) const;

    // 返回最佳安装顺序
    QList<DebIr> bestInstallQueue(const QList<DebIr> &installIrs, const QList<DebIr> &dependIrs);

signals:
    // 正在初始化后端，true：启动，false：完成
    void runBackend(bool inProcess);

    // 正在分析包情况
    void runAnalyzeDeb(bool inProcess, int currentRote, int pkgCount);

    // 软件包缓存更新已启动
    void cacheUpdateStarted();

    // 软件包缓存更新完成，apt 锁已释放
    void cacheUpdateFinished();

private:
    // 读取 DConfig bool 值，DConfig 不可用时返回 defaultValue
    bool readDConfigBool(const QString &key, bool defaultValue) const;
    // 写入 DConfig bool 值，成功返回 true
    bool writeDConfigBool(const QString &key, bool value);

    // 读取 DConfig 开关：是否允许安装前自动刷新 apt 缓存。默认 true（DConfig 不可用时也返回 true，保持原行为）
    bool isAutoUpdateBeforeInstallEnabled() const;

    // 将一次性 update 开关置 false（在 update 真正完成后调用，避免崩溃导致标志位丢失）
    void markOneTimeUpdateDone();
    // 读取一次性 update 开关，DConfig 不可用时返回 false（无法持久化完成状态，避免每次启动重复触发）
    bool isOneTimeUpdateEnabled() const;

    QApt::Package *packageWithArch(const QString &packageName, const QString &sysArch, const QString &annotation) const;
    QString resolvMultiArchAnnotation(const QString &annotation, const QString &debArch, int multiArchType) const;

    // 获取APT源文件的最新修改时间
    QDateTime getSourcesLastModified() const;

    // 获取APT缓存文件的修改时间
    QDateTime getCacheLastModified() const;

    explicit PackageAnalyzer(QObject *parent = nullptr);
    PackageAnalyzer(const PackageAnalyzer &) = delete;
    PackageAnalyzer operator=(const PackageAnalyzer &) = delete;

    QStringList archs;
    QSharedPointer<QApt::Backend> backend = nullptr;
    std::atomic_bool backendInInit;
    std::atomic_bool inPkgAnalyze;
    int pkgWaitToAnalyzeTotal = -1;
    int alreadyAnalyzed = 0;
    std::atomic_bool uiExited;
    std::atomic_bool m_cacheUpdateFinished{false};

    // 标记当前 update 是否由一次性开关触发，用于在 setCacheUpdateFinished 中延迟置 key=false
    bool m_oneTimeUpdateInProgress{false};
};

Q_DECLARE_METATYPE(QList<DebIr>);

#endif  // PACKAGEANALYZER_H
