// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compatible_backend.h"

#include <QProcess>
#include <QtConcurrent/QtConcurrentRun>
#include <QTextStream>
#include <QRegExp>
#include <QStandardPaths>

#include <mutex>

namespace Compatible {

// compatible controller params
static const QString kCompatibleBin = "deepin-compatible-ctl";
static const QString kCompList = "list";
static const QString kCompSepecificRootFs = "r";
static const QString kCompRootFs = "rootfs";
static const QString kCompApp = "app";
static const QString kCompInstall = "install";
static const QString kCompRemove = "remove";

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

bool CompatibleBackend::compatibleValid() const
{
    // test
    return true;

    return m_init && !m_rootfsList.isEmpty();
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

    return m_compatibleExists;
}

QStringList CompatibleBackend::osNameList() const
{
    QStringList nameList;
    for (const RootfsInfo::Ptr &rootfsPtr : m_rootfsList) {
        nameList.append(rootfsPtr->osName);
    }

    return nameList;
}

CompPkgInfo::Ptr CompatibleBackend::containsPackage(const QString &packageName)
{
    // TODO: test
    if (packageName == "debian-test") {
        auto ptr = CompPkgInfo::Ptr::create();
        ptr->name = "debian-test";
        ptr->filePath = "/home/uos/Downloads/a_temp/debian-test_1.0.0_amd64.deb";
        ptr->version = "1.0.0";
        return ptr;
    }

    return m_packages.value(packageName);
}

void CompatibleBackend::packageInstalled(const CompPkgInfo::Ptr &appendPtr)
{
    if (!appendPtr) {
        return;
    }
    appendPtr->installed = true;

    m_packages.insert(appendPtr->name, appendPtr);
}

void CompatibleBackend::packageRemoved(const CompPkgInfo::Ptr &removePtr)
{
    if (!removePtr) {
        return;
    }

    m_packages.remove(removePtr->name);
}

QList<RootfsInfo::Ptr> CompatibleBackend::parseRootfsFromRawOutput(const QByteArray &output)
{
    QTextStream stream(output, QIODevice::ReadOnly);

    // remove title
    stream.readLine();
    stream.readLine();

    bool convert{false};
    QString temp;
    QList<RootfsInfo::Ptr> rootfsList;

    while (!stream.atEnd()) {
        auto rootfsPtr = RootfsInfo::Ptr::create();

        stream >> temp;
        rootfsPtr->prioriy = temp.toInt(&convert);
        if (!convert) {
            QRegExp reg("^\\d+");
            int pos = 0;
            if ((pos = reg.indexIn(temp)) != -1) {
                rootfsPtr->prioriy = reg.cap().toInt();
            }
        }

        stream >> rootfsPtr->name;
        stream >> rootfsPtr->osName;

        // read next
        stream.readLine();

        rootfsList.append(rootfsPtr);
    }

    return rootfsList;
}

QHash<QString, CompPkgInfo::Ptr> CompatibleBackend::parseAppListFromRawOutput(const QByteArray &output)
{
    QHash<QString, CompPkgInfo::Ptr> packageList;

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

        // mark package from cache
        pkgPtr->installed = true;

        packageList.insert(pkgPtr->name, pkgPtr);
    }

    return packageList;
}

void CompatibleBackend::initBackend(bool async)
{
    static std::once_flag kCompInitFlag;
    std::call_once(kCompInitFlag, [this, async]() {
        if (async) {
            QtConcurrent::run([this]() { CompatibleBackend::backendProcess(this); });
        } else {
            CompatibleBackend::backendProcess(this);
        }
    });
}

void CompatibleBackend::backendProcess(CompatibleBackend *backend)
{
    QProcess queryProcess;
    queryProcess.setProgram(kCompatibleBin);
    queryProcess.setArguments({kCompRootFs, kCompList});
    queryProcess.start();
    queryProcess.waitForFinished();

    QByteArray output = queryProcess.readAllStandardOutput();
    auto rootfsList = parseRootfsFromRawOutput(output);

    queryProcess.setArguments({kCompApp, kCompList});
    output = queryProcess.readAllStandardOutput();
    auto packages = parseAppListFromRawOutput(output);

    QMetaObject::invokeMethod(backend, [backend, rootfsList, packages]() { backend->initFinished(rootfsList, packages); });
}

void CompatibleBackend::initFinished(const QList<RootfsInfo::Ptr> &rootfsList, const QHash<QString, CompPkgInfo::Ptr> &packages)
{
    m_init = true;
    m_rootfsList = rootfsList;
    m_packages = packages;

    Q_EMIT compatibleInitFinished();
}

};  // namespace Compatible
