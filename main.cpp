#include "debinstaller.h"

#include <DApplication>

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication app(argc, argv);

    DebInstaller w;
    w.show();

    return app.exec();
}
