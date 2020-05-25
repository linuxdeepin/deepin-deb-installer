#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QThread>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    qDebug() << "StartInstallDeepinwine";

    QProcess proc;

    proc.start("sudo", QStringList() << "apt-get" << "install" << "deepin-wine" << "deepin-wine-helper" << "--fix-missing" << "-y");

    proc.waitForFinished(-1);
    std::cout << proc.readAllStandardOutput().data();

    proc.close();

    return 0;
}
