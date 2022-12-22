// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singleInstallerApplication.h"
#include "view/pages/debinstaller.h"

#include <DWidgetUtil>
#include <DGuiApplicationHelper>

#include <QCommandLineParser>
#include <QtDBus/QtDBus>
const QString kDebInstallManagerService = "com.deepin.DebInstaller";
const QString kDebInstallManagerIface = "/com/deepin/DebInstaller";
SingleInstallerApplication::SingleInstallerApplication(int &argc, char **argv)
    : DApplication(argc, argv)
{

}

void SingleInstallerApplication::activateWindow()
{
    if (nullptr == m_qspMainWnd.get()) {
        m_qspMainWnd.reset(new DebInstaller());
        Dtk::Widget::moveToCenter(m_qspMainWnd.get());
        m_qspMainWnd->show();
    } else {
        m_qspMainWnd->setWindowState(Qt::WindowActive);
        m_qspMainWnd->activateWindow(); // Reactive main window
        m_qspMainWnd->showNormal();     //非特效模式下激活窗口
    }
    if (bIsDbus)
        m_qspMainWnd->hide();
    if (m_selectedFiles.size() > 0) {
        QMetaObject::invokeMethod(m_qspMainWnd.get(), "slotPackagesSelected", Qt::QueuedConnection, Q_ARG(QStringList, m_selectedFiles));
    }
}

void SingleInstallerApplication::InstallerDeb(const QStringList &debPathList)
{
    qDebug() << Q_FUNC_INFO << debPathList;
    if (debPathList.size() > 0) {
        QMetaObject::invokeMethod(m_qspMainWnd.get(), "slotPackagesSelected", Qt::QueuedConnection, Q_ARG(QStringList, debPathList));
    } else {
        if (m_qspMainWnd.get()) {                   //先判断当前是否已经存在一个进程。
            m_qspMainWnd.get()->activateWindow();   //特效模式下激活窗口
            m_qspMainWnd.get()->showNormal();       //无特效激活窗口
        }
    }
}

QString SingleInstallerApplication::InstallerDebPackge(const QString &debPath)
{
    QString ret;
    //启动安装
    QMetaObject::invokeMethod(m_qspMainWnd.get(), "startInstallPackge", Qt::DirectConnection, Q_RETURN_ARG(QString, ret), Q_ARG(QString, debPath));
    //调用结束关闭进程
    QTimer::singleShot(100, [&]() {
        quit();
    });
    return ret;
}

QString SingleInstallerApplication::unInstallDebPackge(const QString &debPath)
{
    QString ret;
    //卸载包
    QMetaObject::invokeMethod(m_qspMainWnd.get(), "startUnInstallPackge", Qt::DirectConnection, Q_RETURN_ARG(QString, ret), Q_ARG(QString, debPath));
    //调用结束关闭进程
    QTimer::singleShot(100, [&]() {
        quit();
    });
    return ret;
}

int SingleInstallerApplication::checkInstallStatus(const QString &debPath)
{
    int ret;
    //获取包安装状态
    QMetaObject::invokeMethod(m_qspMainWnd.get(), "checkInstallStatus", Qt::DirectConnection,  Q_RETURN_ARG(int, ret), Q_ARG(QString, debPath));
    return  ret;
}

int SingleInstallerApplication::checkDependsStatus(const QString &debPath)
{
    int ret;
    //获取包依赖状态
    QMetaObject::invokeMethod(m_qspMainWnd.get(), "checkDependsStatus", Qt::DirectConnection, Q_RETURN_ARG(int, ret), Q_ARG(QString, debPath));
    return  ret;
}

int SingleInstallerApplication::checkDigitalSignature(const QString &debPath)
{
    int ret;
    //获取包依赖状态
    QMetaObject::invokeMethod(m_qspMainWnd.get(), "checkDigitalSignature", Qt::DirectConnection, Q_RETURN_ARG(int, ret), Q_ARG(QString, debPath));
    return  ret;
}

QString SingleInstallerApplication::getPackageInfo(const QString &debPath)
{
    QString ret;
    //获取包信息
    QMetaObject::invokeMethod(m_qspMainWnd.get(), "getPackageInfo", Qt::DirectConnection, Q_RETURN_ARG(QString, ret), Q_ARG(QString, debPath));
    return  ret;
}

bool SingleInstallerApplication::parseCmdLine()
{
    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin Package Installer.");
    parser.addOption(QCommandLineOption("dbus", "enable daemon mode"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("filename", "Deb package path.", "file [file..]");
    parser.process(*this);

    QDBusConnection conn = QDBusConnection::sessionBus();

    if (!conn.registerService(kDebInstallManagerService) ||
            !conn.registerObject(kDebInstallManagerIface, this, QDBusConnection::ExportScriptableSlots)) { //注册失败 说明已经存在deb-installer
        qDebug() << "Failed to register dbus";
        QDBusInterface deb_install(kDebInstallManagerService, kDebInstallManagerIface, kDebInstallManagerService, QDBusConnection::sessionBus());
        QList<QVariant> debInstallPathList;
        debInstallPathList << parser.positionalArguments();
        //激活已有deb-installer
        QDBusMessage msg = deb_install.callWithArgumentList(QDBus::AutoDetect, "InstallerDeb", debInstallPathList);
        qWarning() << msg.errorMessage();
        return false;
    } else {
        qDebug() << "Register dbus service successfully";
        const QStringList paraList = parser.positionalArguments();
        if (paraList.isEmpty()) {
            //dbus打开不显示界面
            if (parser.isSet("dbus")) {
                bIsDbus = true;
            }
        } else {
            for (auto it : paraList) {
                m_selectedFiles.append(it);
            }
            if (paraList.size() > 0 && m_selectedFiles.size() == 0) {
                return false;
            }
        }
        return true;
    }
}
