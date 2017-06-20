#include "debinstallworker.h"

#include <QDebug>

DebInstallWorker::DebInstallWorker(QObject *parent)
    : QObject(parent)
{

}

void DebInstallWorker::startInstall()
{
    qDebug() << "start install";
}
