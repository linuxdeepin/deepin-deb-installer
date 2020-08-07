#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QThread>
#include <iostream>
#include "installDebThread.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    qDebug() << "StartInstallDeepinwine";

    QCommandLineParser parser;
    parser.process(app);
    const QStringList tParamList = parser.positionalArguments();

    InstallDebThread *mThread = new InstallDebThread;
    mThread->setDependList(tParamList);
    mThread->run();
    mThread->wait();
    return mThread->m_resultFlag;
}
