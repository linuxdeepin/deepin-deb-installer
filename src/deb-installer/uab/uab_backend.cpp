// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uab_backend.h"
#include "utils/ddlog.h"

#include <mutex>

#include <QApplication>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QPointer>
#include <QProcess>
#include <QStandardPaths>
#include <QtConcurrent/QtConcurrentRun>

#include "utils/utils.h"
// depends deb pacakge arch
#include "model/packageanalyzer.h"

namespace Uab {

// bin command
static const QString kUabCliBin = "ll-cli";
static const QString kUabJson = "--json";
static const QString kUabCliList = "list";
// e.g.: [path to package] --print-meta
static const QString kUabPkgCmdPrintMeta = "--print-meta";
// json field
static const QString kUabLayers = "layers";
static const QString kUabInfo = "info";
static const QString kUabKind = "kind";
static const QString kUabKindAppTag = "app";
// json package info field
static const QString kUabId = "id";
static const QString kUabName = "name";
static const QString kUabVersion = "version";
static const QString kUabArch = "arch";
static const QString kUabChannel = "channel";
static const QString kUabModule = "module";
static const QString kUabDescription = "description";

UabBackend::UabBackend(QObject *parent)
    : QObject{parent}
{
    qCDebug(appLog) << "Initializing UabBackend";
    recheckLinglongExists();
    qRegisterMetaType<QList<UabPkgInfo::Ptr>>("QList<UabPkgInfo::Ptr>");
    qCDebug(appLog) << "UabBackend initialized, linglong exists:" << m_linglongExists;
}

UabBackend::~UabBackend() {
    qCDebug(appLog) << "UabBackend destructed";
}

UabBackend *UabBackend::instance()
{
    // qCDebug(appLog) << "UabBackend::instance()";
    static UabBackend ins;
    return &ins;
}

/**
 * @brief Check uab package exist and executable.
 *        If executable, execute `uabPath --print-meta` to get package meta data.
 * @return UabPkgInfo::Ptr uab package info, or null if error.
 */
UabPkgInfo::Ptr UabBackend::packageFromMetaData(const QString &uabPath, QString *errorString)
{
    qCDebug(appLog) << "Getting package metadata from:" << uabPath;
    const QFileInfo info(uabPath);
    if (!info.exists()) {
        qCWarning(appLog) << "UAB file not exists:" << uabPath;
        if (errorString) {
            qCDebug(appLog) << "UAB file not exists, set error string";
            *errorString = QString("uab file not exists");
        }
        return {};
    }

    const QByteArray output = uabExecuteOutput(uabPath, errorString);
    if (output.isEmpty()) {
        qCDebug(appLog) << "UAB execute output is empty";
        return {};
    }

    auto uabPtr = UabBackend::packageFromMetaJson(output);
    if (uabPtr) {
        qCDebug(appLog) << "UAB execute output is not empty";
        uabPtr->filePath = info.absoluteFilePath();
    }
    return uabPtr;
}

/**
   @brief Find package named \a packageId , and version equal \a version
        If \a version is empty, the return package is the latest version of the installed packages.
   @return Uab package pointer, or null if not found.
 */
UabPkgInfo::Ptr UabBackend::findPackage(const QString &packageId, const QString &version)
{
    qCDebug(appLog) << "Finding package:" << packageId << "version:" << version;
    if (!backendInited()) {
        qCWarning(appLog) << "Uab backend not initialized for package:" << packageId;
        m_lastError = QString("uab backend not init");
        return {};
    }

    if (m_packageList.isEmpty()) {
        qCWarning(appLog) << "Package list is empty, cannot find package:" << packageId;
        return {};
    }

    auto itr = std::upper_bound(m_packageList.begin(),
                                m_packageList.end(),
                                packageId,
                                [](const QString &findPkg, const UabPkgInfo::Ptr &uabPtr) { return findPkg <= uabPtr->id; });

    // versions with the same ID are listed in descending order
    while ((m_packageList.end() != itr) && ((*itr)->id == packageId)) {
        if (version.isEmpty() || ((*itr)->version == version)) {
            qCDebug(appLog) << "Found package:" << (*itr)->id << "version:" << (*itr)->version;
            return *itr;
        }

        itr++;
    }

    qCDebug(appLog) << "Package not found:" << packageId << "version:" << version;
    return {};
}

/**
   @brief Read Linglong's package information, arch, etc.
        When the package needs to be installed, the Uab backend will be initialized.
        If 'async' is true (default), will be initialized on the child thread.
 */
void UabBackend::initBackend(bool async)
{
    qCDebug(appLog) << "Initializing backend, async:" << async;
    static std::once_flag kUabBackendFlag;
    std::call_once(kUabBackendFlag, [async, this]() {
        qCDebug(appLog) << "Backend initialization started";
        if (async) {
            QtConcurrent::run(UabBackend::backendProcess, this);
            qCDebug(appLog) << "Backend initialization running in background thread";
        } else {
            UabBackend::backendProcess(this);
            qCDebug(appLog) << "Backend initialization completed synchronously";
        }
    });
}

bool UabBackend::backendInited() const
{
    // qCDebug(appLog) << "Checking if backend is initialized:" << m_init;
    return m_init;
}

bool UabBackend::linglongExists() const
{
    qCDebug(appLog) << "Checking if linglong exists:" << m_linglongExists;
    return m_linglongExists;
}

bool UabBackend::recheckLinglongExists()
{
    qCDebug(appLog) << "Re-checking for linglong existence";
    // find ll-cli in $PATH
    const QString execPath = QStandardPaths::findExecutable(kUabCliBin);
    m_linglongExists = !execPath.isEmpty();
    qCDebug(appLog) << "Linglong exists:" << m_linglongExists << "at path:" << execPath;

    return m_linglongExists;
}

QString UabBackend::lastError() const
{
    qCDebug(appLog) << "Getting last error:" << m_lastError;
    return m_lastError;
}

void UabBackend::dumpPackageList() const
{
    qCInfo(appLog) << QString("Uab package list(count %1) support archs:").arg(m_packageList.size()) << m_supportArchSet;
    for (const auto &uabPtr : m_packageList) {
        qCInfo(appLog) << "    " << uabPtr;
    }
}

void UabBackend::backendInitData(const QList<UabPkgInfo::Ptr> &packageList, const QSet<QString> &archs)
{
    qCDebug(appLog) << "Initializing backend data with" << packageList.size() << "packages and" << archs.size() << "architectures";
    m_packageList = packageList;
    m_supportArchSet = archs;
    m_init = true;
    Q_EMIT backendInitFinsihed();
    qCDebug(appLog) << "Backend data initialized and signal emitted";
}

bool UabBackend::parsePackagesFromRawJson(const QByteArray &jsonData, QList<UabPkgInfo::Ptr> &packageList)
{
    qCDebug(appLog) << "Parsing packages from raw JSON data";
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &jsonError);
    if (QJsonParseError::NoError != jsonError.error) {
        qCWarning(appLog) << "Parse ll-cli list json data failed:" << jsonError.errorString();
        return false;
    }

    packageList.clear();
    QJsonArray rootArray = doc.array();
    qCDebug(appLog) << "Found" << rootArray.size() << "packages in JSON data";
    for (const auto &value : rootArray) {
        if (!value.isObject()) {
            qCWarning(appLog) << "Skipping non-object value in JSON array";
            continue;
        }

        QJsonObject item = value.toObject();
        auto uabPtr = UabPkgInfo::Ptr::create();

        uabPtr->id = item.value(kUabId).toString();
        uabPtr->appName = item.value(kUabName).toString();
        uabPtr->version = item.value(kUabVersion).toString();
        uabPtr->channel = item.value(kUabChannel).toString();
        uabPtr->module = item.value(kUabModule).toString();
        uabPtr->description = item.value(kUabDescription).toString();

        QJsonArray archArray = item.value(kUabArch).toArray();
        for (const auto &archItem : archArray) {
            uabPtr->architecture.append(archItem.toString());
        }

        packageList.append(uabPtr);
    }

    qCDebug(appLog) << "Finished parsing packages from JSON data, total packages:" << packageList.size();
    return true;
}

bool UabBackend::parsePackagesFromRawOutput(const QByteArray &output, QList<UabPkgInfo::Ptr> &packageList)
{
    qCDebug(appLog) << "Parsing packages from raw output";
    QTextStream stream(output, QIODevice::ReadOnly);

    // remove title
    stream.readLine();

    QString arch;

    while (!stream.atEnd()) {
        auto uabPtr = UabPkgInfo::Ptr::create();

        // title field format: id  name  version  arch  channel  module  description
        stream >> uabPtr->id;
        stream >> uabPtr->appName;
        stream >> uabPtr->version;
        stream >> arch;
        stream >> uabPtr->channel;
        stream >> uabPtr->module;

        uabPtr->architecture.append(arch);
        // to the end of the line
        uabPtr->description = stream.readLine().simplified();

        packageList.append(uabPtr);
    }
    qCDebug(appLog) << "Finished parsing packages from raw output, total packages:" << packageList.size();

    return true;
}

/**
   @brief Get Linglong package list from `ll-cli list`
        The packages are sorted by package id and package version.

   @note This function will be run in QtConcurrent::run()
*/
void UabBackend::backendProcess(const QPointer<Uab::UabBackend> &notifyPtr)
{
    qCDebug(appLog) << "Starting backend process to list packages";
    QProcess process;
    process.start(kUabCliBin, {kUabJson, kUabCliList});
    process.waitForFinished();

    const QByteArray output = process.readAllStandardOutput();
    QList<UabPkgInfo::Ptr> packageList;
    parsePackagesFromRawJson(output, packageList);
    sortPackages(packageList);

    // detect deb package init
    QSet<QString> archs;
    if (PackageAnalyzer::instance().isBackendReady()) {
        QStringList archList = PackageAnalyzer::instance().supportArchList();
#if QT_VERSION_CHECK(5, 14, 0) <= QT_VERSION
        archs = QSet<QString>(archList.begin(), archList.end());
#else
        archs = archList.toSet();
#endif

        // adapt arch name for uab
        if (archs.contains("amd64")) {
            archs.insert("x86_64");
        }

        // TODO(renbin): loongarch64 and loong64 diff arch
    }

    if (!qApp->closingDown() && !notifyPtr.isNull()) {
        QMetaObject::invokeMethod(
            notifyPtr, "backendInitData", Q_ARG(QList<UabPkgInfo::Ptr>, packageList), Q_ARG(QSet<QString>, archs));
    }
}

/**
   @brief Sort \a packageList by package id and version.
    Package id in ascending order, version in descending order.
    Make sure the latest version of a package is listed first.
 */
void UabBackend::sortPackages(QList<UabPkgInfo::Ptr> &packageList)
{
    qCDebug(appLog) << "Sorting" << packageList.size() << "packages";
    if (packageList.isEmpty()) {
        qCDebug(appLog) << "Package list is empty, skipping sort";
        return;
    }

    std::sort(packageList.begin(), packageList.end(), [](const UabPkgInfo::Ptr &left, const UabPkgInfo::Ptr &right) {
        const int ret = QString::compare(left->id, right->id);
        if (!!ret) {
            return ret < 0;
        }
        return Utils::compareVersion(left->version, right->version) > 0;
    });
    qCDebug(appLog) << "Finished sorting packages";
}

void UabBackend::packageInstalled(const UabPkgInfo::Ptr &appendPtr)
{
    qCDebug(appLog) << "Adding installed package to list:" << appendPtr->id << appendPtr->version;
    m_packageList.append(appendPtr);
    sortPackages(m_packageList);

    qCInfo(appLog) << QString("Uab package installed - ID: %1, Version: %2, Arch: %3")
                   .arg(appendPtr->id)
                   .arg(appendPtr->version)
                   .arg(appendPtr->architecture.join(","));
}

void UabBackend::packageRemoved(const UabPkgInfo::Ptr &removePtr)
{
    qCDebug(appLog) << "Removing package from list:" << removePtr->id << removePtr->version;
    auto findItr = std::find_if(m_packageList.begin(), m_packageList.end(), [&](const UabPkgInfo::Ptr &package) {
        return (removePtr->id == package->id) && (removePtr->version == package->version) &&
               (removePtr->architecture == package->architecture);
    });

    if (findItr != m_packageList.end()) {
        m_packageList.erase(findItr);

        qCInfo(appLog) << QString("Uab package removed - ID: %1, Version: %2, Arch: %3")
                       .arg(removePtr->id)
                       .arg(removePtr->version)
                       .arg(removePtr->architecture.join(","));
    } else {
        qCWarning(appLog) << "Package not found in list for removal:" << removePtr->id << removePtr->version;
    }
    // remove package dose not require sort.
}

UabPkgInfo::Ptr UabBackend::packageFromMetaJson(const QByteArray &json, QString *errorString)
{
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(json, &error);
    if (QJsonParseError::NoError != error.error) {
        if (errorString) {
            *errorString = QString("uab json parse erorr: %1, offset: %2").arg(error.errorString()).arg(error.offset);
        }
        return {};
    }

    const QJsonArray layers = doc.object().value(kUabLayers).toArray();
    if (layers.isEmpty()) {
        qCWarning(appLog) << "Uab json does not contain 'layers' field";
        if (errorString) {
            *errorString = QString("uab json not contains 'layers'");
        }
        return {};
    }

    for (const QJsonValue &layerItem : layers) {
        if (!layerItem.isObject()) {
            continue;
        }

        QJsonObject info = layerItem.toObject().value(kUabInfo).toObject();
        if (info.isEmpty()) {
            continue;
        }

        if (kUabKindAppTag != info.value(kUabKind).toString()) {
            continue;
        }

        auto uabPtr = UabPkgInfo::Ptr::create();
        uabPtr->id = info[kUabId].toString();
        uabPtr->appName = info[kUabName].toString();
        uabPtr->version = info[kUabVersion].toString();
        uabPtr->description = info[kUabDescription].toString();
        uabPtr->channel = info[kUabChannel].toString();
        uabPtr->module = info[kUabModule].toString();

        const QJsonArray archArray = info[kUabArch].toArray();
        for (const QJsonValue &archValue : archArray) {
            const QString curArch = archValue.toString();
            uabPtr->architecture.append(curArch);
        }

        return uabPtr;
    }

    if (errorString) {
        qCWarning(appLog) << "Uab json does not contain app info node";
        *errorString = QString("uab json not contains app info node");
    }
    return {};
}

QByteArray UabBackend::uabExecuteOutput(const QString &uabPath, QString *errorString)
{
    qCDebug(appLog) << "Executing uab package to get output:" << uabPath;
    QFile uabFile(uabPath);
    if (!uabFile.exists()) {
        qCWarning(appLog) << "UAB file does not exist:" << uabPath;
        if (errorString) {
            *errorString = "UAB file does not exist";
        }
        return {};
    }

    // temporarily set uab file executable to get meta info.
    static const QFile::Permissions kExecutable = QFile::ExeOwner | QFile::ExeUser | QFile::ExeGroup;
    QFile::Permissions savePermission = uabFile.permissions();
    bool needExecutable = !(savePermission & kExecutable);
    if (needExecutable && (!uabFile.setPermissions(savePermission | kExecutable))) {
        qCWarning(appLog) << "Failed to set executable permission for uab file:" << uabPath << "error:" << uabFile.errorString();
        if (errorString) {
            *errorString = QString("set uab file executable failed: %1").arg(uabFile.errorString());
        }
        return {};
    }

    QProcess proc;
    proc.setProgram(uabPath);
    proc.setArguments({kUabPkgCmdPrintMeta});
    proc.start();
    proc.waitForFinished();

    if (needExecutable) {
        qCDebug(appLog) << "Restoring original permissions for:" << uabPath;
        uabFile.setPermissions(savePermission);
    }

    if (0 != proc.exitCode() || QProcess::NormalExit != proc.exitStatus()) {
        qCWarning(appLog) << "Failed to execute uab package:" << uabPath << "exit code:" << proc.exitCode()
                      << "error:" << proc.errorString();
        if (errorString) {
            *errorString = QString("exec uab package failed: %1").arg(proc.errorString());
        }
        return {};
    }

    qCDebug(appLog) << "Successfully executed uab package and got output";
    return proc.readAllStandardOutput();
}

}  // namespace Uab

QT_BEGIN_NAMESPACE
#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug out, const Uab::UabPkgInfo &uabPkg)
{
    // qCDebug(appLog) << "UabPackageInfo:" << uabPkg;
    out << "UabPackageInfo(" << QString("0x%0").arg(reinterpret_cast<quintptr>(&uabPkg), 0, 16) << "){";
    out << Uab::kUabId << ":" << uabPkg.id << ";";
    out << Uab::kUabName << ":" << uabPkg.appName << ";";
    out << Uab::kUabVersion << ":" << uabPkg.version << ";";
    out << Uab::kUabArch << ":" << uabPkg.architecture << ";";
    out << Uab::kUabChannel << ":" << uabPkg.channel << ";";
    out << Uab::kUabModule << ":" << uabPkg.module << ";";
    out << Uab::kUabDescription << ":" << uabPkg.description << ";";
    out << "filePath:" << uabPkg.filePath << ";";

    return out;
}

Q_CORE_EXPORT QDebug operator<<(QDebug out, const Uab::UabPkgInfo::Ptr &uabPkgPtr)
{
    // qCDebug(appLog) << "UabPackageInfoPtr:" << uabPkgPtr;
    if (uabPkgPtr) {
        out << *uabPkgPtr;
    } else {
        out << QString("UabPackageInfo:Ptr(nullptr)");
    }

    return out;
}
#endif  // QT_NO_DEBUG_STREAM
QT_END_NAMESPACE
