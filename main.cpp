#include "debinstaller.h"

#include <DApplication>

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication::loadDXcbPlugin();

    DApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("deepin-deb-installer");
    app.setApplicationVersion("1.0");
    app.loadTranslator();
    app.setTheme("light");

    DebInstaller w;
    w.show();

    return app.exec();
}
