#include "debinstaller.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DebInstaller w;
    w.show();

    return a.exec();
}
