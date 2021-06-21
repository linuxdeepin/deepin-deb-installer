#include "singleInstallerApplication.h"
#include "view/pages/debinstaller.h"
#include "view/pages/MainWindow.h"
#include "utils/DebugTimeManager.h"

#include <DWidgetUtil>
#include <DGuiApplicationHelper>

#include <QCommandLineParser>

DCORE_USE_NAMESPACE

SingleInstallerApplication::SingleInstallerApplication(int &argc, char **argv)
    : DApplication(argc, argv)
    , m_qspMainWnd(nullptr)
{

}

SingleInstallerApplication::~SingleInstallerApplication()
{

}

bool SingleInstallerApplication::parseCmdLine()
{
    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin Package Installer.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("filename", "Deb package path.", "file [file..]");

    parser.process(*this);

    if (!m_selectedFiles.isEmpty()) {
        m_selectedFiles.clear();
    }

    QStringList paraList = parser.positionalArguments();
    if (!paraList.isEmpty()) {
        for (auto it : paraList) {
            m_selectedFiles.append(it);
        }
    }

    if (paraList.size() > 0 && m_selectedFiles.size() == 0) {
        return false;
    }

    return true;
}

void SingleInstallerApplication::activateWindow()
{
    qDebug() << "Active quick install window to install file:" << m_selectedFiles << m_selectedFiles.size();

    if (nullptr == m_qspMainWnd.get()) {
        m_qspMainWnd.reset(new MainWindow());
//        Dtk::Widget::moveToCenter(m_qspMainWnd.get());
//        m_qspMainWnd.get()->moveToCenter();

        m_qspMainWnd->show();
    } else {
        m_qspMainWnd->setWindowState(Qt::WindowActive);
        m_qspMainWnd->activateWindow(); // Reactive main window
        m_qspMainWnd->showNormal();     //非特效模式下激活窗口
    }

    if (m_selectedFiles.size() > 0) {
        qDebug() << "m_selectedFiles" << m_selectedFiles;
        QMetaObject::invokeMethod(m_qspMainWnd.get(), "onPackagesSelected", Qt::QueuedConnection, Q_ARG(QStringList, m_selectedFiles));
    }
}

void SingleInstallerApplication::InstallerDeb(const QStringList &debPathList)
{
    if (debPathList.size() > 0) {
        qDebug() << "debPath List" << debPathList;
        QMetaObject::invokeMethod(m_qspMainWnd.get(), "onPackagesSelected", Qt::QueuedConnection, Q_ARG(QStringList, debPathList));
    } else {
        //启动栏或者桌面启动空的软件包安装器后激活原有的窗口
        if (m_qspMainWnd.get()) {                   //先判断当前是否已经存在一个进程。
            m_qspMainWnd.get()->activateWindow();   //特效模式下激活窗口
            m_qspMainWnd.get()->showNormal();       //无特效激活窗口
        }
    }
}
