// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "packageanalyzer.h"
#include "singleInstallerApplication.h"
#include "compatible/compatible_backend.h"
#include "utils/ddlog.h"

#include <QtDebug>
#include <QThread>
#include <QApplication>
#include <QMetaType>

#include <QApt/Backend>
#include <QApt/DebFile>

PackageAnalyzer &PackageAnalyzer::instance()
{
    static PackageAnalyzer analyzer;
    while (analyzer.backendInInit) {
        QThread::msleep(10);
        QApplication::processEvents();
    }
    return analyzer;
}

PackageAnalyzer::PackageAnalyzer(QObject *parent)
    : QObject(parent)
{
    qCDebug(appLog) << "PackageAnalyzer constructor called";
    backendInInit = false;
    inPkgAnalyze = false;
    uiExited = false;

    qRegisterMetaType<QList<DebIr>>("QList<DebIr>");
}

void PackageAnalyzer::setUiExit()
{
    qCDebug(appLog) << "UI exit signal received";
    uiExited = true;
}

void PackageAnalyzer::initBackend()
{
    qCDebug(appLog) << "PackageAnalyzer initBackend";
    if (backend != nullptr) {
        qCDebug(appLog) << "Backend already initialized";
        return;
    }

    qCInfo(appLog) << "Initializing backend...";
    emit runBackend(true);
    backendInInit = true;

    qCDebug(appLog) << "Waiting for backend running init flag to clear...";
    while (SingleInstallerApplication::BackendIsRunningInit) {
        QThread::msleep(10);
    }
    qCDebug(appLog) << "Backend running init flag cleared.";

    SingleInstallerApplication::BackendIsRunningInit = true;

    // init compatible backend with deb backend asynchronous
    if (CompBackend::instance()->compatibleExists()) {
        qCDebug(appLog) << "Compatible backend exists, initializing...";
        CompBackend::instance()->initBackend();
    }

    backend.reset(new QApt::Backend);
    qCDebug(appLog) << "QApt backend created, calling init()";
    bool initSuccess = backend->init();

    SingleInstallerApplication::BackendIsRunningInit = false;

    if (!initSuccess) {
        qCCritical(appLog) << "Backend initialization failed:" << backend->initErrorMessage();
        qFatal("%s", backend->initErrorMessage().toStdString().c_str());
    }
    qCDebug(appLog) << "Backend initialized successfully.";

    archs = backend->architectures();
    archs.append("all");
    archs.append("any");
    qCDebug(appLog) << "Supported architectures set:" << archs;

    // wait compatible init finished;
    bool nonGuiThread = qApp->thread() != QThread::currentThread();
    while (nonGuiThread && CompBackend::instance()->compatibleExists() && !CompBackend::instance()->compatibleInited()) {
        QThread::msleep(5);
    }

    backendInInit = false;
    emit runBackend(false);
    qCInfo(appLog) << "Backend initialization process finished.";
}

bool PackageAnalyzer::isBackendReady()
{
    bool ready = backend.get() != nullptr;
    qCDebug(appLog) << "Backend ready status:" << ready;
    return ready;
}

QApt::Backend *PackageAnalyzer::backendPtr()
{
    // qCDebug(appLog) << "Providing backend pointer.";
    return backend.get();
}

QPair<Pkg::PackageInstallStatus, QString> PackageAnalyzer::packageInstallStatus(const DebIr &ir) const
{
    qCDebug(appLog) << "Checking install status for package:" << ir.packageName << "version:" << ir.version;
    Pkg::PackageInstallStatus status;
    QString installedVersion;

    do {
        if (!ir.isValid) {
            qCDebug(appLog) << "Package IR is not valid, status: NotInstalled";
            status = Pkg::NotInstalled;
            break;
        }

        QApt::Package *package = packageWithArch(ir.packageName, ir.architecture, "");

        if (package == nullptr) {
            qCDebug(appLog) << "Package" << ir.packageName << "not found in backend, status: NotInstalled";
            status = Pkg::NotInstalled;
            break;
        }

        installedVersion = package->installedVersion();
        if (installedVersion.isEmpty()) {
            qCDebug(appLog) << "Package" << ir.packageName << "found, but not installed, status: NotInstalled";
            status = Pkg::NotInstalled;
            break;
        }

        qCDebug(appLog) << "Package" << ir.packageName << "is installed with version" << installedVersion << ". Comparing with" << ir.version;
        int result = QApt::Package::compareVersion(ir.version, installedVersion);
        if (result == 0) {
            qCDebug(appLog) << "Versions are the same. Status: InstalledSameVersion";
            status = Pkg::InstalledSameVersion;
        } else if (result < 0) {
            qCDebug(appLog) << "Installed version is later. Status: InstalledLaterVersion";
            status = Pkg::InstalledLaterVersion;
        } else {
            qCDebug(appLog) << "Installed version is earlier. Status: InstalledEarlierVersion";
            status = Pkg::InstalledEarlierVersion;
        }
    } while (0);

    qCDebug(appLog) << "Final status for" << ir.packageName << "is" << status << "with installed version" << installedVersion;
    return {status, installedVersion};
}

QApt::Package *
PackageAnalyzer::packageWithArch(const QString &packageName, const QString &sysArch, const QString &annotation) const
{
    qCDebug(appLog) << "Looking for package:" << packageName << "arch:" << sysArch << "annotation:" << annotation;
    QApt::Package *package =
        backend->package(packageName + resolvMultiArchAnnotation(annotation, sysArch, QApt::InvalidMultiArchType));

    if (!package) {
        qCDebug(appLog) << "Package not found with annotation, trying package name only";
        package = backend->package(packageName);
    }
    if (package) {
        qCDebug(appLog) << "Found package:" << package->name();
        return package;
    }

    qCDebug(appLog) << "Package not found, iterating through all architectures";
    for (QString arch : backend->architectures()) {
        package = backend->package(packageName + ":" + arch);
        if (package) {
            qCDebug(appLog) << "Found package with arch:" << arch;
            return package;
        }
    }

    qCDebug(appLog) << "Package not found, checking for virtual packages that provide it";
    for (auto *virtualPackage : backend->availablePackages()) {
        if (virtualPackage->name() != packageName && virtualPackage->providesList().contains(packageName)) {
            qCDebug(appLog) << "Found virtual package provider:" << virtualPackage->name() << ", recursing";
            return packageWithArch(virtualPackage->name(), sysArch, annotation);
        }
    }

    qCDebug(appLog) << "Package" << packageName << "not found";
    return nullptr;
}

QString PackageAnalyzer::resolvMultiArchAnnotation(const QString &annotation, const QString &debArch, int multiArchType) const
{
    qCDebug(appLog) << "Resolving multi-arch annotation:" << annotation << "debArch:" << debArch << "type:" << multiArchType;
    if ("native" == annotation || "any" == annotation) {
        qCDebug(appLog) << "Annotation is 'native' or 'any', returning empty string.";
        return QString();
    }
    if ("all" == annotation) {
        qCDebug(appLog) << "Annotation is 'all', returning empty string.";
        return QString();
    }
    if (multiArchType == QApt::MultiArchForeign) {
        qCDebug(appLog) << "Multi-arch type is Foreign, returning empty string.";
        return QString();
    }

    QString arch;
    if (annotation.isEmpty()) {
        arch = debArch;
        qCDebug(appLog) << "Annotation is empty, using deb arch:" << arch;
    } else {
        arch = annotation;
        qCDebug(appLog) << "Using annotation as arch:" << arch;
    }

    if (!arch.startsWith(':') && !arch.isEmpty()) {
        return arch.prepend(':');
    } else {
        qCDebug(appLog) << "Arch is empty or already starts with ':', returning as is:" << arch;
        return arch;
    }
}

bool PackageAnalyzer::virtualPackageIsExist(const QString &virtualPackageName) const
{
    qCDebug(appLog) << "Checking if virtual package exists:" << virtualPackageName;
    // 由于没法搜索虚拟包，此处只能进行全包遍历
    for (auto *package : backend->availablePackages()) {
        if (package->name() != virtualPackageName && package->providesList().contains(virtualPackageName)) {
            qCDebug(appLog) << "Found provider for" << virtualPackageName << ":" << package->name();
            return true;
        }
    }
    qCDebug(appLog) << "No provider found for virtual package:" << virtualPackageName;
    return false;
}

bool PackageAnalyzer::versionMatched(const QString &lhs, const QString &rhs, QApt::RelationType relationType) const
{
    qCDebug(appLog) << "Matching version, lhs:" << lhs << "rhs:" << rhs << "relation:" << relationType;
    if (relationType == QApt::NoOperand) {
        qCDebug(appLog) << "No operand, returning true.";
        return true;
    }

    int compareResult = QApt::Package::compareVersion(lhs, rhs);
    bool isMatched = false;
    qCDebug(appLog) << "Version comparison result:" << compareResult;
    if (compareResult == 0) {  // lhs == rhs
        if (relationType == QApt::LessOrEqual || relationType == QApt::GreaterOrEqual || relationType == QApt::Equals) {
            isMatched = true;
        }
    } else if (compareResult < 0) {  // lhs < rhs
        if (relationType == QApt::GreaterOrEqual || relationType == QApt::GreaterThan || relationType == QApt::NotEqual) {
            isMatched = true;
        }
    } else {  // lhs > rhs
        if (relationType == QApt::LessOrEqual || relationType == QApt::LessThan || relationType == QApt::NotEqual) {
            isMatched = true;
        }
    }
    qCDebug(appLog) << "Version match result:" << isMatched;
    return isMatched;
}

bool PackageAnalyzer::dependIsReady(const QApt::DependencyItem &depend) const
{
    qCDebug(appLog) << "Checking if dependency is ready:" << depend.first().packageName();
    // 1.每个item内部为或关系，只要有一个满足条件，即可认为该依赖已就绪
    bool isReady = false;
    for (const auto &item : depend) {
        // 2.简单检查item是否安装

        // 2.1获取基本数据
        auto name = item.packageName();
        auto version = item.packageVersion();
        auto type = item.relationType();
        auto arch = item.multiArchAnnotation();
        qCDebug(appLog) << "Checking OR-dependency item:" << name << "version:" << version;

        // 2.2获取包状态
        auto package = packageWithArch(name, arch, "");
        if (package != nullptr && package->isInstalled()) {
            qCDebug(appLog) << "Package" << name << "is installed, checking version.";
            // 如果已安装，则检查版本情况
            auto pkgVersion = package->version();

            if (versionMatched(version, pkgVersion, type)) {
                qCDebug(appLog) << "Version matched. Dependency is ready.";
                isReady = true;
                break;
            }
            qCDebug(appLog) << "Version did not match.";
        } else if (virtualPackageIsExist(name)) {  // 3.如果没有安装，则检查其作为虚拟包是否已安装
            qCDebug(appLog) << "Package" << name << "is not installed, but a virtual package provider exists. Dependency is ready.";
            isReady = true;
            break;
        }
    }
    qCDebug(appLog) << "Final dependency ready status:" << isReady;
    return isReady;
}

QList<QApt::DependencyItem> PackageAnalyzer::debDependNotInstalled(const DebIr &ir) const
{
    qCDebug(appLog) << "Getting not-installed dependencies for package:" << ir.packageName;
    // 获取依赖项
    auto debDepends = ir.depends;
    qCDebug(appLog) << "Initial dependency count:" << debDepends.size();

    // 获取安装状态，已安装的就丢出去
    for (int i = 0; i != debDepends.size(); ++i) {
        if (dependIsReady(debDepends.at(i))) {
            qCDebug(appLog) << "Dependency" << debDepends.at(i).first().packageName() << "is ready, removing from list.";
            debDepends.removeAt(i);
            --i;
        }
    }

    qCDebug(appLog) << "Final not-installed dependency count:" << debDepends.size();
    return debDepends;
}

void PackageAnalyzer::startPkgAnalyze(int total)
{
    qCDebug(appLog) << "Starting package analysis for total packages:" << total;
    if (total > 0) {
        pkgWaitToAnalyzeTotal = total;
        alreadyAnalyzed = 0;
    }
}

void PackageAnalyzer::stopPkgAnalyze()
{
    qCDebug(appLog) << "Stopping package analysis.";
    pkgWaitToAnalyzeTotal = -1;
    alreadyAnalyzed = 0;
    emit runAnalyzeDeb(false, 0, 0);
}

QList<DebIr> PackageAnalyzer::analyzeDebFiles(const QFileInfoList &infos,
                                              QSet<QByteArray> *md5s,
                                              QStringList *appNames,
                                              bool excludeArchNotMatched,
                                              bool excludeInstalledOrLaterVersion)
{
    qCDebug(appLog) << "Starting deb file analysis";
    QList<DebIr> irs;
    QList<int> appNameNeedRemove;
    for (int i = 0; i != infos.size(); ++i) {
        qCDebug(appLog) << "Analyzing deb file:" << infos[i].absoluteFilePath();

        if (uiExited) {
            qCDebug(appLog) << "UI exited, breaking analysis loop.";
            break;
        }

        if (pkgWaitToAnalyzeTotal > 0) {
            emit runAnalyzeDeb(true, alreadyAnalyzed++, pkgWaitToAnalyzeTotal);
        }

        auto path = infos[i].absoluteFilePath();
        QApt::DebFile deb(path);

        if (!deb.isValid()) {  // 无效包直接去除
            qCDebug(appLog) << "Deb file is invalid, skipping:" << path;
            appNameNeedRemove.append(i);
            continue;
        }

        // 如果需要丢掉不匹配的架构
        bool archMatched = supportArch(deb.architecture());
        if (excludeArchNotMatched && !archMatched) {
            qCDebug(appLog) << "Architecture not matched (" << deb.architecture() << "), skipping:" << path;
            appNameNeedRemove.append(i);
            continue;
        }

        // 如果需要丢掉已安装或已安装高版本
        if (excludeInstalledOrLaterVersion) {
            auto pkg = packageWithArch(deb.packageName(), deb.architecture(), "");
            if (pkg != nullptr && pkg->isInstalled()) {
                if (QApt::Package::compareVersion(pkg->version(), deb.version()) >= 0) {
                    qCDebug(appLog) << "Package is already installed with same or later version, skipping:" << path;
                    appNameNeedRemove.append(i);
                    continue;
                }
            }
        }

        auto packageMd5 = deb.md5Sum();
        if (md5s->contains(packageMd5)) {  // 包已存在，去重
            qCDebug(appLog) << "Duplicate package detected by MD5, skipping:" << path;
            appNameNeedRemove.append(i);
            continue;
        } else {
            md5s->insert(packageMd5);
        }

        // 生成ir
        qCDebug(appLog) << "Creating DebIr for valid package:" << deb.packageName();
        DebIr ir;
        ir.filePath = path;
        ir.version = deb.version();
        ir.shortDescription = deb.shortDescription();
        ir.packageName = deb.packageName();
        ir.architecture = deb.architecture();
        ir.archMatched = archMatched;
        ir.md5 = deb.md5Sum();
        ir.isValid = deb.isValid();
        ir.depends = deb.depends();

        // TODO：QApt不支持从deb内部提取提供的虚拟包

        irs.push_back(ir);
    }

    if (appNames != nullptr && !uiExited) {
        qCDebug(appLog) << "Removing" << appNameNeedRemove.size() << "app names for skipped packages.";
        for (int i = appNameNeedRemove.size() - 1; i != -1; --i) {
            appNames->removeAt(appNameNeedRemove[i]);
        }
    }

    qCDebug(appLog) << "Analysis finished. Returning" << irs.size() << "valid packages.";
    return irs;
}

void PackageAnalyzer::chooseDebFromDepend(QList<DebIr> *result,
                                          QSet<QByteArray> *md5s,
                                          const QList<QApt::DependencyItem> &depends,
                                          const QList<DebIr> &debIrs) const
{
    qCDebug(appLog) << "Choosing debs from dependencies. Current unsatisfied count:" << depends.size();
    for (const auto &depend : depends) {  // 对每一个不满足的依赖项
        bool finded = false;
        for (const auto &item : depend) {  // 对每一个或依赖
            auto name = item.packageName();
            auto version = item.packageVersion();
            auto type = item.relationType();
            qCDebug(appLog) << "Checking candidate provider:" << name << version;
            for (const auto &ir : debIrs) {  // 对每一个候选包
                if (ir.packageName == name && versionMatched(version, ir.version, type)) {
                    qCDebug(appLog) << "Found matching candidate package:" << ir.filePath;
                    auto depdeps = debDependNotInstalled(ir);  // 依赖包的依赖
                    if (!depdeps.isEmpty()) {  // 有未就绪的依赖，在依赖包集合里面执行递归搜索
                        qCDebug(appLog) << "Candidate" << ir.packageName << "has its own unmet dependencies, recursing.";
                        chooseDebFromDepend(result, md5s, depdeps, debIrs);
                    }
                    if (!md5s->contains(ir.md5)) {
                        qCDebug(appLog) << "Adding candidate" << ir.packageName << "to results.";
                        md5s->insert(ir.md5);
                        result->push_back(ir);
                    }
                    finded = true;
                    break;
                }
            }
            if (finded) {
                qCDebug(appLog) << "Found a provider for dependency" << depend.first().packageName() << ", moving to next dependency.";
                break;
            }
        }
    }
}

QList<DebIr> PackageAnalyzer::bestInstallQueue(const QList<DebIr> &installIrs, const QList<DebIr> &dependIrs)
{
    qCDebug(appLog) << "Calculating best install queue for" << installIrs.size() << "target packages and" << dependIrs.size() << "dependency packages.";
    // TODO：后面重构的时候需要实现安装顺序计算，本轮需求仅实现抽取需要的包

    // 1.检查依赖是否已就绪，将未就绪的项抽取出来
    QList<QApt::DependencyItem> installDeps;  // 记录每一个安装项的依赖情况
    for (const auto &installIr : installIrs) {
        qCDebug(appLog) << "Analyzing install package:" << installIr.filePath;

        if (uiExited) {
            qCWarning(appLog) << "UI exited during analysis, aborting.";
            return QList<DebIr>();
        }

        if (pkgWaitToAnalyzeTotal > 0) {
            emit runAnalyzeDeb(true, alreadyAnalyzed++, pkgWaitToAnalyzeTotal);
        }

        auto notInstallDepends = debDependNotInstalled(installIr);
        if (!notInstallDepends.isEmpty()) {
            qCDebug(appLog) << "Package" << installIr.packageName << "has" << notInstallDepends.size() << "unmet dependencies.";
            installDeps.append(notInstallDepends);
        }
    }
    qCDebug(appLog) << "Total unmet dependencies from target packages:" << installDeps.size();

    // 2.从依赖包集合中抽取目前尚未就绪的项
    QList<DebIr> realDepends;
    QSet<QByteArray> md5s;
    chooseDebFromDepend(&realDepends, &md5s, installDeps, dependIrs);
    qCDebug(appLog) << "Resolved" << realDepends.size() << "dependencies from the provided dependency packages.";

    // 3.融合并返回结果
    auto result = realDepends + installIrs;
    qCDebug(appLog) << "Final install queue size:" << result.size();
    return result;
}
