#include "debinstaller.h"

#include <DApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QTimer>
#include <DLog>

DWIDGET_USE_NAMESPACE
#ifdef DUTIL_USE_NAMESPACE
DUTIL_USE_NAMESPACE
#else
DCORE_USE_NAMESPACE
#endif

int main(int argc, char *argv[])
{
    DApplication::loadDXcbPlugin();

    DApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("deepin-deb-installer");
    app.setApplicationVersion("1.0");
    app.setApplicationAcknowledgementPage("https://www.deepin.org/acknowledgments/deepin-package-manager/");
    app.setProductIcon(QPixmap(":/images/icon.png"));
    app.loadTranslator();
    app.setProductName(QApplication::translate("main", "Deepin Package Manager"));
    app.setApplicationDescription(QApplication::translate("main", "Deepin Package Manager is used to help users install and remove local software, supports to bulk install."));
    app.setTheme("light");

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    // command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin deb package manager.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("filename", "Deb package path.", "file [file..]");

    parser.process(app);

    const QStringList file_list = parser.positionalArguments();
    qDebug() << file_list;

    DebInstaller w;
    w.show();

    // select files from args
    if (!file_list.isEmpty())
        QMetaObject::invokeMethod(&w, "onPackagesSelected", Qt::QueuedConnection, Q_ARG(QStringList, file_list));

    return app.exec();
}
