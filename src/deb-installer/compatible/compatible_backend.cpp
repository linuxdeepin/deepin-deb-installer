// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compatible_backend.h"
#include "compatible_json_parser.h"
#include "utils/ddlog.h"

#include <QApplication>
#include <QProcess>
#include <QtConcurrent/QtConcurrentRun>
#include <QTextStream>
#include <QRegExp>
#include <QStandardPaths>

#include <mutex>

namespace Compatible {

// compatible controller params
static const QString kCompatibleBin = "deepin-compatible-ctl";
static const QString kCompJsonFormat = "--json";
static const QString kCompList = "list";
static const QString kCompListAll = "-a";
static const QString kCompSepecificRootFs = "r";
static const QString kCompRootFs = "rootfs";
static const QString kCompApp = "app";
static const QString kCompInstall = "install";
static const QString kCompRemove = "remove";
static const QString kCompCheck = "check";
static const QString kCompPS = "ps";

CompatibleBackend::CompatibleBackend(QObject *parent)
    : QObject{parent}
{
    recheckCompatibleExists();
}

CompatibleBackend *CompatibleBackend::instance()
{
    static CompatibleBackend ins;
    return &ins;
}

#ifndef DISABLE_COMPATIBLE
bool CompatibleBackend::compatibleValid() const
{
    // TODO: check rootfs list later, with `app check`
    return m_init /*&& !m_rootfsList.isEmpty()*/;
}

bool CompatibleBackend::compatibleInited() const
{
    return m_init;
}

bool CompatibleBackend::compatibleExists() const
{
    return m_compatibleExists;
}

bool CompatibleBackend::recheckCompatibleExists()
{
    // find ll-cli in $PATH
    const QString execPath = QStandardPaths::findExecutable(kCompatibleBin);
    m_compatibleExists = !execPath.isEmpty();

    qCDebug(appLog) << "check compatible return: " << m_compatibleExists;
    return m_compatibleExists;
}
#endif

QList<RootfsInfo::Ptr> CompatibleBackend::rootfsList() const
{
    return m_rootfsList;
}

QString CompatibleBackend::osName(const QString &rootfsName) const
{
    qCDebug(appLog) << "get os name for rootfs: " << rootfsName;

    auto findItr = std::find_if(m_rootfsList.begin(), m_rootfsList.end(), [&](const RootfsInfo::Ptr &rootfsPtr) {
        return rootfsPtr->name == rootfsName;
    });

    if (findItr != m_rootfsList.end()) {
        auto osName = (*findItr)->osName;
        qCDebug(appLog) << "find os name: " << osName;
        return osName;
    }

    qCDebug(appLog) << "os name not found for rootfs: " << rootfsName;
    return {};
}

CompPkgInfo::Ptr CompatibleBackend::containsPackage(const QString &packageName)
{
    return m_packages.value(packageName);
}

void CompatibleBackend::packageInstalled(const CompPkgInfo::Ptr &appendPtr)
{
    qCDebug(appLog) << "Adding installed package to list:" << appendPtr->name << appendPtr->version;

    if (!appendPtr) {
        qCDebug(appLog) << "appendPtr is null, return";
        return;
    }
    appendPtr->rootfs = appendPtr->targetRootfs;

    m_packages.insert(appendPtr->name, appendPtr);
    qCDebug(appLog) << "Adding package to list:" << appendPtr->name << appendPtr->version;
}

void CompatibleBackend::packageRemoved(const CompPkgInfo::Ptr &removePtr)
{
    qCDebug(appLog) << "Removing removed package from list:" << removePtr->name << removePtr->version;

    if (!removePtr) {
        qCDebug(appLog) << "removePtr is null, return";
        return;
    }
    removePtr->rootfs.clear();

    m_packages.remove(removePtr->name);
    qCDebug(appLog) << "Removing package from list:" << removePtr->name << removePtr->version;
}

#ifndef DISABLE_COMPATIBLE
bool CompatibleBackend::supportAppCheck() const
{
    return true;
}

bool CompatibleBackend::checkPackageSupportRootfs(const CompPkgInfo::Ptr &checkPtr)
{
    qCDebug(appLog) << "Checking package support rootfs";

    if (!checkPtr || checkPtr->filePath.isEmpty() || !supportAppCheck()) {
        qCDebug(appLog) << "checkPtr is null or empty, return false";
        return false;
    }

    QString captureFilePath = checkPtr->filePath;
    captureFilePath.detach();
    qCDebug(appLog) << "check package file path: " << captureFilePath;

    QThreadPool::globalInstance()->start(
        [=]() {

#if 1
            // This is a temporary change
            QList<RootfsInfo::Ptr> rootfs;
            QProcess queryProcess;
            queryProcess.setProgram(kCompatibleBin);

            // FIXME: we need init rootfs?
            queryProcess.setArguments({kCompApp, kCompPS});
            queryProcess.start();
            // 30s not enough for init, up to 20mins.
            if (queryProcess.waitForFinished(20 * 60 * 1000)) {
                queryProcess.setArguments({kCompRootFs, kCompList});
                queryProcess.start();
                if (queryProcess.waitForFinished()) {
                    QByteArray output = queryProcess.readAllStandardOutput();
                    rootfs = parseRootfsFromRawOutputV2(output);
                }
            } else {
                qCWarning(appLog) << "Delay get(init) rootfs list failed! " << queryProcess.errorString();
            }

#else
            // app check require root privileges
            // e.g.: pkexec deepin-deb-installer-dependsInstall --install_compatible --check [file path] --user [current user]
            // And real command in backend: deepin-compatible-ctl app --json check [file path]
            QStringList params{"deepin-deb-installer-dependsInstall", "--install_compatible", "--check", captureFilePath};
            auto env = QProcessEnvironment::systemEnvironment();
            QString currentUser = env.value("USER");
            if (!currentUser.isEmpty()) {
                params << "--user" << currentUser;
            }

            // up to 10 mins
            QProcess checkProc;
            checkProc.start("pkexec", params);
            checkProc.waitForFinished(1000 * 60 * 10);
            if (QProcess::UnknownError != checkProc.error()) {
                qCWarning(appLog) << "Compatible app check failed: " << checkProc.errorString();
            }

            QList<RootfsInfo::Ptr> rootfs;
            // get last json output
            QByteArray checkOutput = checkProc.readAllStandardOutput().trimmed();
            int lastLineOffset = checkOutput.lastIndexOf('\n');

            if (-1 != lastLineOffset) {
                QByteArray lastLine = checkOutput.mid(lastLineOffset + 1);
                qCInfo(appLog) << "Parse app check return" << lastLine;

                auto ret = CompatibleJsonParser::parseCommonField(lastLine);
                if (ret && CompSuccess == ret->code) {
                    // parse rootfs info
                    rootfs = CompatibleJsonParser::parseSupportfsList(ret);
                }
            }
#endif

            // update data on GUI thread
            QMetaObject::invokeMethod(
                qApp,
                [=]() {
                    // FIXME: we need init rootfs?
                    CompatibleBackend::instance()->m_rootfsList = rootfs;

                    checkPtr->checked = true;
                    checkPtr->supportRootfs = rootfs;
                    Q_EMIT CompatibleBackend::instance()->packageSupportRootfsChanged(checkPtr);
                },
                Qt::QueuedConnection);
        },
        QThread::TimeCriticalPriority);

    qCDebug(appLog) << "Check package support rootfs finished, return true";
    return true;
}
#endif

QList<RootfsInfo::Ptr> CompatibleBackend::parseRootfsFromRawOutputV1(const QByteArray &output)
{
    QTextStream stream(output, QIODevice::ReadOnly);

    qCDebug(appLog) << "parse rootfs from raw output: " << output;

    // remove title
    stream.readLine();
    stream.readLine();

    bool convert{false};
    QString temp;
    QList<RootfsInfo::Ptr> rootfsList;
    QRegExp priorityReg("^\\d+");
    QRegExp createTimeReg("\\d{4}-\\d{2}-\\d{2}");

    while (!stream.atEnd()) {
        auto rootfsPtr = RootfsInfo::Ptr::create();
        stream >> temp;
        rootfsPtr->prioriy = temp.toInt(&convert);
        if (!convert) {
            int priority = priorityReg.indexIn(temp);
            if (priority != -1) {
                rootfsPtr->prioriy = priorityReg.cap().toInt();
            }
        }

        stream >> rootfsPtr->name;

        // special field: os name ( contains space ), wen
        temp = stream.readLine();
        int createTimePos = createTimeReg.indexIn(temp);
        if (-1 != createTimePos) {
            rootfsPtr->osName = temp.left(createTimePos).trimmed();
        }

        rootfsList.append(rootfsPtr);
    }

    qCDebug(appLog) << "parse rootfs from raw output finished, size: " << rootfsList.size();

    return rootfsList;
}

QList<RootfsInfo::Ptr> CompatibleBackend::parseRootfsFromRawOutputV2(const QByteArray &output)
{
    /* e.g.:
       ID           Name                   Image                            Status
       --------------------------------------------------------------------------------
       4aeedabc79a4 uos-rootfs-20          localhost/uos-rootfs-20:latest   Up 6 minutes
    */

    QTextStream stream(output, QIODevice::ReadOnly);
    qCDebug(appLog) << "V2 parse rootfs from raw output: " << output;
    // remove title
    stream.readLine();
    stream.readLine();

    QList<RootfsInfo::Ptr> rootfsList;
    QString deprecated;

    while (!stream.atEnd()) {
        auto rootfsPtr = RootfsInfo::Ptr::create();

        stream >> deprecated;
        stream >> rootfsPtr->name;
        stream.readLine();

        rootfsPtr->osName = rootfsPtr->name;
        rootfsList.append(rootfsPtr);
    }

    qCDebug(appLog) << "V2 parse rootfs from raw output finished, size: " << rootfsList.size();

    return rootfsList;
}

QHash<QString, CompPkgInfo::Ptr> CompatibleBackend::parseAppListFromRawOutput(const QByteArray &output)
{
    QHash<QString, CompPkgInfo::Ptr> packageList;

    qCDebug(appLog) << "parse app list from raw output: " << output;

    QTextStream stream(output, QIODevice::ReadOnly);

    // remove title
    stream.readLine();
    stream.readLine();

    while (!stream.atEnd()) {
        auto pkgPtr = CompPkgInfo::Ptr::create();

        stream >> pkgPtr->name;
        stream >> pkgPtr->version;
        stream >> pkgPtr->arch;
        stream >> pkgPtr->rootfs;

        packageList.insert(pkgPtr->name, pkgPtr);
    }

    qCDebug(appLog) << "parse app list from raw output finished, size: " << packageList.size();

    return packageList;
}

void CompatibleBackend::initBackend(bool async)
{
    qCDebug(appLog) << "init compatible backend: " << async;
    if (!compatibleExists()) {
        qCDebug(appLog) << "Compatible backend not exists, return";
        return;
    }

    static std::once_flag kCompInitFlag;
    std::call_once(kCompInitFlag, [this, async]() {
        if (async) {
            QtConcurrent::run([this]() { CompatibleBackend::backendProcessWithRaw(this); });
        } else {
            CompatibleBackend::backendProcessWithRaw(this);
        }
    });
    qCDebug(appLog) << "init compatible backend finished";
}

void CompatibleBackend::backendProcessWithRaw(CompatibleBackend *backend)
{
    QHash<QString, CompPkgInfo::Ptr> packages;
    QList<RootfsInfo::Ptr> rootfsList;
    QByteArray output;

    qCDebug(appLog) << "Compatible backend process with raw";

    QProcess queryProcess;
    queryProcess.setProgram(kCompatibleBin);

    queryProcess.setArguments({kCompRootFs, kCompList});
    queryProcess.start();
    if (queryProcess.waitForFinished()) {
        output = queryProcess.readAllStandardOutput();
        rootfsList = parseRootfsFromRawOutputV2(output);
    } else {
        qCWarning(appLog) << "Get rootfs list failed! " << queryProcess.errorString();
    }

    queryProcess.setArguments({kCompApp, kCompList, kCompListAll});
    queryProcess.start();
    if (queryProcess.waitForFinished()) {
        output = queryProcess.readAllStandardOutput();
        packages = parseAppListFromRawOutput(output);
    } else {
        qCWarning(appLog) << "Get app list failed! " << queryProcess.errorString();
    }

    // NOTE: ComaptibleBackend might not inited in main thread( no event loop ), so use qApp instaed.
    QMetaObject::invokeMethod(
        qApp, [backend, rootfsList, packages]() { backend->initFinished(rootfsList, packages); }, Qt::QueuedConnection);

    qCDebug(appLog) << "Compatible backend process finished";
}

void CompatibleBackend::backendProcessWithJson(CompatibleBackend *backend)
{
    QHash<QString, CompPkgInfo::Ptr> packages;
    QList<RootfsInfo::Ptr> rootfsList;
    QByteArray output;
    CompatibleRet::Ptr retPtr;

    qCDebug(appLog) << "Compatible backend process with json";

    QProcess queryProcess;
    queryProcess.setProcessChannelMode(QProcess::MergedChannels);
    queryProcess.setProgram(kCompatibleBin);

    queryProcess.setArguments({kCompApp, kCompJsonFormat, kCompList, kCompListAll});
    queryProcess.start();
    if (queryProcess.waitForFinished()) {
        output = queryProcess.readAll();
        retPtr = CompatibleJsonParser::parseCommonField(output);
        packages = CompatibleJsonParser::parseAppList(retPtr);
    } else {
        qCWarning(appLog) << "Get app list failed! " << queryProcess.errorString();
    }

    queryProcess.setArguments({kCompRootFs, kCompJsonFormat, kCompList});
    queryProcess.start();
    if (queryProcess.waitForFinished()) {
        output = queryProcess.readAllStandardOutput();
        retPtr = CompatibleJsonParser::parseCommonField(output);
        rootfsList = CompatibleJsonParser::parseRootfsList(retPtr);
    } else {
        qCWarning(appLog) << "Get rootfs list failed! " << queryProcess.errorString();
    }

    // NOTE: ComaptibleBackend might not inited in main thread( no event loop ), so use qApp instaed.
    QMetaObject::invokeMethod(
        qApp, [backend, rootfsList, packages]() { backend->initFinished(rootfsList, packages); }, Qt::QueuedConnection);

    qCDebug(appLog) << "Compatible backend process finished";
}

void CompatibleBackend::initFinished(const QList<RootfsInfo::Ptr> &rootfsList, const QHash<QString, CompPkgInfo::Ptr> &packages)
{
    m_init = true;
    m_rootfsList = rootfsList;
    m_packages = packages;

    qCInfo(appLog) << "Comaptible init finished, rootfs: " << m_rootfsList << "Package size: " << m_packages.size();

    Q_EMIT compatibleInitFinished();
}

};  // namespace Compatible
