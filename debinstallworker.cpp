#include "debinstallworker.h"
#include "debpackage.h"

#include <QDebug>

DebInstallWorker::DebInstallWorker(QObject *parent)
    : QObject(parent)
{

}

void DebInstallWorker::startInstall()
{
    qDebug() << "start install";
}
