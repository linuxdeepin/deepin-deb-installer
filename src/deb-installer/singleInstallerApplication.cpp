// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singleInstallerApplication.h"
#include "view/pages/debinstaller.h"
#include "manager/packagesmanager.h"
#include "uab/uab_backend.h"
#include "utils/ddlog.h"

#include <DWidgetUtil>
#include <DGuiApplicationHelper>

#include <QCommandLineParser>
#include <QtDBus/QtDBus>
const QString kDebInstallManagerService = "com.deepin.DebInstaller";
const QString kDebInstallManagerIface = "/com/deepin/DebInstaller";

SingleInstallerApplication::AppWorkChannel SingleInstallerApplication::mode;
std::atomic_bool SingleInstallerApplication::BackendIsRunningInit;
bool SingleInstallerApplication::s_forceCompatible = false;

SingleInstallerApplication::SingleInstallerApplication(int &argc, char **argv)
    : DApplication(argc, argv)
{
    qCDebug(appLog) << "SingleInstallerApplication constructor";
    BackendIsRunningInit = false;
}

void SingleInstallerApplication::setHasPackages(bool has)
{
    if (m_hasPackages == has)
        return;
    m_hasPackages = has;
    Q_EMIT hasPackagesChanged(has);
}

void SingleInstallerApplication::activateWindow()
{
    qCDebug(appLog) << "Activating window, mode:" << mode;
    if (!m_ddimFiles.isEmpty()) {
        qCDebug(appLog) << "DDIM files found, setting mode to DdimChannel";
        mode = DdimChannel;
    } else {
        qCDebug(appLog) << "No DDIM files found, setting mode to NormalChannel";
        mode = NormalChannel;
    }

    if (nullptr == m_qspMainWnd.get()) {
        qCDebug(appLog) << "Main window is not initialized, creating new DebInstaller";
        m_qspMainWnd.reset(new DebInstaller());
        connect(static_cast<DebInstaller *>(m_qspMainWnd.get()), &DebInstaller::packagesCleared, this, [this]() { setHasPackages(false); });
        Dtk::Widget::moveToCenter(m_qspMainWnd.get());
        m_qspMainWnd->show();
    } else {
        qCDebug(appLog) << "Main window is already initialized, activating window";
        m_qspMainWnd->setWindowState(Qt::WindowActive);
        m_qspMainWnd->activateWindow();  // Reactive main window
        m_qspMainWnd->showNormal();      // 非特效模式下激活窗口
    }

    if (bIsDbus) {
        qCDebug(appLog) << "UAB backend is enabled, initializing UAB backend";
        // init uab backend synchronous on bus mode, but must be initialized after deb backend.
        // sa PackageAnalyzer::instance()
        Uab::UabBackend::instance()->initBackend(false);

        m_qspMainWnd->hide();
    }

    if (!m_ddimFiles.isEmpty()) {
        setHasPackages(true);
        QMetaObject::invokeMethod(m_qspMainWnd.get(), "slotDdimSelected", Qt::QueuedConnection, Q_ARG(QStringList, m_ddimFiles));
    } else if (!m_selectedFiles.isEmpty()) {
        setHasPackages(true);
        PackagesManager::setForceCompatible(s_forceCompatible);
        QMetaObject::invokeMethod(
            m_qspMainWnd.get(), "slotPackagesSelected", Qt::QueuedConnection, Q_ARG(QStringList, m_selectedFiles));
    } else {  // do nothing
    }
}

void SingleInstallerApplication::InstallerDeb(const QStringList &debPathList)
{
    qCInfo(appLog) << "InstallerDeb called with" << debPathList.size() << "packages";
    // 复位兼容标志，防止同一实例内"先兼容安装、后普通安装"时标志串味
    s_forceCompatible = false;
    if (!debPathList.isEmpty())
        setHasPackages(true);
    if (mode == DdimChannel) {
        qCDebug(appLog) << "DdimChannel mode, invoking slotDdimSelected";
        QMetaObject::invokeMethod(m_qspMainWnd.get(), "slotDdimSelected", Qt::QueuedConnection, Q_ARG(QStringList, debPathList));
    } else if (debPathList.size() > 0) {
        qCDebug(appLog) << "NormalChannel mode, invoking slotPackagesSelected";
        PackagesManager::setForceCompatible(false);
        QMetaObject::invokeMethod(
            m_qspMainWnd.get(), "slotPackagesSelected", Qt::QueuedConnection, Q_ARG(QStringList, debPathList));
    } else {
        qCDebug(appLog) << "No packages selected, activating window";
        if (m_qspMainWnd.get()) {                  // 先判断当前是否已经存在一个进程。
            m_qspMainWnd.get()->activateWindow();  // 特效模式下激活窗口
            m_qspMainWnd.get()->showNormal();      // 无特效激活窗口
        }
    }
}

void SingleInstallerApplication::InstallerDebCompatible(const QStringList &debPathList)
{
    qCInfo(appLog) << "InstallerDebCompatible called with" << debPathList.size() << "packages";
    // 同步 app 级标志，避免 D-Bus 单实例转发路径下 s_forceCompatible 为过期值
    // （老实例在 parseCmdLine 时未带 --compatible，s_forceCompatible 默认为 false）
    s_forceCompatible = true;
    if (!debPathList.isEmpty()) {
        setHasPackages(true);
        PackagesManager::setForceCompatible(true);
        QMetaObject::invokeMethod(
            m_qspMainWnd.get(), "slotPackagesSelected", Qt::QueuedConnection, Q_ARG(QStringList, debPathList));
    } else {
        if (m_qspMainWnd.get()) {
            m_qspMainWnd.get()->activateWindow();
            m_qspMainWnd.get()->showNormal();
        }
    }
}

QString SingleInstallerApplication::InstallerDebPackge(const QString &debPath)
{
    qCDebug(appLog) << "Starting package installation for:" << debPath;
    QString ret;
    // 启动安装
    QMetaObject::invokeMethod(
        m_qspMainWnd.get(), "startInstallPackge", Qt::DirectConnection, Q_RETURN_ARG(QString, ret), Q_ARG(QString, debPath));
    qCDebug(appLog) << "Installation result:" << ret;
    // 调用结束关闭进程
    QTimer::singleShot(100, [&]() {
        qCDebug(appLog) << "Quitting application after installation";
        quit();
    });
    return ret;
}

QString SingleInstallerApplication::unInstallDebPackge(const QString &debPath)
{
    qCDebug(appLog) << "Starting package uninstallation for:" << debPath;
    QString ret;
    // 卸载包
    QMetaObject::invokeMethod(
        m_qspMainWnd.get(), "startUnInstallPackge", Qt::DirectConnection, Q_RETURN_ARG(QString, ret), Q_ARG(QString, debPath));
    qCDebug(appLog) << "Uninstallation result:" << ret;
    // 调用结束关闭进程
    QTimer::singleShot(100, [&]() {
        qCDebug(appLog) << "Quitting application after uninstallation";
        quit();
    });
    return ret;
}

int SingleInstallerApplication::checkInstallStatus(const QString &debPath)
{
    qCDebug(appLog) << "Checking install status for:" << debPath;
    int ret;
    // 获取包安装状态
    QMetaObject::invokeMethod(
        m_qspMainWnd.get(), "checkInstallStatus", Qt::DirectConnection, Q_RETURN_ARG(int, ret), Q_ARG(QString, debPath));
    qCDebug(appLog) << "Install status:" << ret;
    return ret;
}

int SingleInstallerApplication::checkDependsStatus(const QString &debPath)
{
    qCDebug(appLog) << "Checking dependency status for:" << debPath;
    int ret;
    // 获取包依赖状态
    QMetaObject::invokeMethod(
        m_qspMainWnd.get(), "checkDependsStatus", Qt::DirectConnection, Q_RETURN_ARG(int, ret), Q_ARG(QString, debPath));
    qCDebug(appLog) << "Dependency status:" << ret;
    return ret;
}

int SingleInstallerApplication::checkDigitalSignature(const QString &debPath)
{
    qCDebug(appLog) << "Checking digital signature for:" << debPath;
    int ret;
    // 获取包依赖状态
    QMetaObject::invokeMethod(
        m_qspMainWnd.get(), "checkDigitalSignature", Qt::DirectConnection, Q_RETURN_ARG(int, ret), Q_ARG(QString, debPath));
    qCDebug(appLog) << "Digital signature status:" << ret;
    return ret;
}

QString SingleInstallerApplication::getPackageInfo(const QString &debPath)
{
    qCDebug(appLog) << "Getting package info for:" << debPath;
    QString ret;
    // 获取包信息
    QMetaObject::invokeMethod(
        m_qspMainWnd.get(), "getPackageInfo", Qt::DirectConnection, Q_RETURN_ARG(QString, ret), Q_ARG(QString, debPath));
    qCDebug(appLog) << "Package info retrieved";
    return ret;
}

bool SingleInstallerApplication::parseCmdLine()
{
    qCDebug(appLog) << "Parsing command line arguments";
    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin Package Installer.");
    parser.addOption(QCommandLineOption("dbus", "enable daemon mode"));
    parser.addOption(QCommandLineOption("compatible", "force compatible mode installation"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("filename", "Deb package path.", "file [file..]");
    parser.process(*this);
    s_forceCompatible = parser.isSet("compatible");
    qCDebug(appLog) << "Positional arguments:" << parser.positionalArguments();
    qCDebug(appLog) << "Force compatible mode:" << s_forceCompatible;
    qCDebug(appLog) << "D-Bus mode:" << parser.isSet("dbus");

    m_selectedFiles.clear();
    m_ddimFiles.clear();

    QDBusConnection conn = QDBusConnection::sessionBus();

    if (!conn.registerService(kDebInstallManagerService) ||
        !conn.registerObject(
            kDebInstallManagerIface,
            this,
            QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableProperties)) {  // 注册失败 说明已经存在deb-installer
        qCWarning(appLog) << "Failed to register D-Bus service, another instance may be running";
        QDBusInterface deb_install(
            kDebInstallManagerService, kDebInstallManagerIface, kDebInstallManagerService, QDBusConnection::sessionBus());
        QList<QVariant> debInstallPathList;
        debInstallPathList << parser.positionalArguments();
        const QString method = s_forceCompatible ? "InstallerDebCompatible" : "InstallerDeb";
        QDBusMessage msg = deb_install.callWithArgumentList(QDBus::AutoDetect, method, debInstallPathList);
        qCWarning(appLog) << "D-Bus call" << method << "failed:" << msg.errorMessage();
        return false;
    } else {
        qCInfo(appLog) << "Registered D-Bus service successfully";
        const QStringList paraList = parser.positionalArguments();
        if (paraList.isEmpty()) {
            qCDebug(appLog) << "No packages selected, returning false";
            // hide main window on dbus
            if (parser.isSet("dbus")) {
                bIsDbus = true;
            }
        } else {
            qCDebug(appLog) << "Processing" << paraList.size() << "package files";
            for (auto it : paraList) {
                if (it.endsWith("ddim")) {
                    qCDebug(appLog) << "Adding DDIM file:" << it;
                    m_ddimFiles.append(it);
                } else {
                    qCDebug(appLog) << "Adding DEB file:" << it;
                    m_selectedFiles.append(it);
                }
            }

            if (!paraList.isEmpty() && m_selectedFiles.isEmpty() && m_ddimFiles.isEmpty()) {
                qCDebug(appLog) << "No packages selected, returning false";
                return false;
            }
        }
        qCDebug(appLog) << "Processing" << paraList.size() << "package files, returning true";
        return true;
    }

    qCDebug(appLog) << "No packages selected, returning true";
    return true;
}
