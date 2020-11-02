#include "singleInstallerApplication.h"
#include <QCommandLineParser>

#include <DWidgetUtil>
#include <DGuiApplicationHelper>
#include "debinstaller.h"

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
        m_qspMainWnd.reset(new DebInstaller());
        Dtk::Widget::moveToCenter(m_qspMainWnd.get());
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
        if (m_qspMainWnd.get()) {                   //先判断当前是否已经存在一个进程。
            m_qspMainWnd.get()->activateWindow();   //特效模式下激活窗口
            m_qspMainWnd.get()->showNormal();       //无特效激活窗口
        }
    }
}
