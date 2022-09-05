// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "model/deblistmodel.h"
#include "utils/accessible.h"
#include "utils/utils.h"
#include "singleInstallerApplication.h"
#include "environments.h"
#include "utils/eventlogutils.h"

#include <QCommandLineParser>
#include <QDebug>
#include <QTimer>
#include <QSharedMemory>

#include <DApplicationSettings>
#include <DGuiApplicationHelper>
#include <DApplication>
#include <DLog>

#include <QDBusConnection>
#include <QDBusInterface>

DWIDGET_USE_NAMESPACE
#ifdef DUTIL_USE_NAMESPACE
DUTIL_USE_NAMESPACE
#else
DCORE_USE_NAMESPACE
#endif

#define RECENT_PATH QDir::homePath() + "/.local/share/recently-used.xbel"

#define DAPPLICATION_XSTRING(s) DAPPLICATION_STRING(s)
#define DAPPLICATION_STRING(s) #s

int main(int argc, char *argv[])
{
    SingleInstallerApplication app(argc, argv);

    app.setOrganizationName("deepin");
    app.setApplicationName("deepin-deb-installer");
    app.setApplicationVersion(VERSION);                 //qmake 转cmake 解决版本不对的问题
    app.setApplicationAcknowledgementPage("https://www.deepin.org/acknowledgments/deepin-package-manager/");
    app.setProductIcon(QIcon::fromTheme("deepin-deb-installer"));
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.loadTranslator();
    app.setProductName(QApplication::translate("main", "Package Installer"));
    app.setApplicationDisplayName(QApplication::translate("main", "Package Installer"));
    app.setApplicationDescription(QApplication::translate(
                                      "main",
                                      "Package Installer helps users install and remove local packages, and supports bulk installation."));

    qputenv("DTK_USE_SEMAPHORE_SINGLEINSTANCE", "1");

    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin"))
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);

    DApplicationSettings settings;

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    QAccessible::installFactory(accessibleFactory);//自动化测试

    qDebug() << qApp->applicationName() << "started, version = " << qApp->applicationVersion();

    QDBusConnection dbus = QDBusConnection::sessionBus();
    if (dbus.registerService("com.deepin.DebInstaller")) {
        dbus.registerObject("/com/deepin/DebInstaller", &app, QDBusConnection::ExportScriptableSlots);
        app.parseCmdLine();
        app.activateWindow();
        //埋点记录启动数据
        QJsonObject objStartEvent{
            {"tid", Eventlogutils::StartUp},
            {"vsersion", VERSION},
            {"mode", 1},
        };

        Eventlogutils::GetInstance()->writeLogs(objStartEvent);
        return app.exec();
    } else {
        QCommandLineParser parser;
        parser.process(app);
        QList<QVariant> debInstallPathList;
        debInstallPathList << parser.positionalArguments();
        QDBusInterface notification("com.deepin.DebInstaller", "/com/deepin/DebInstaller", "com.deepin.DebInstaller", QDBusConnection::sessionBus());
        QDBusMessage msg = notification.callWithArgumentList(QDBus::AutoDetect, "InstallerDeb", debInstallPathList);
        return 0;
    }
}
