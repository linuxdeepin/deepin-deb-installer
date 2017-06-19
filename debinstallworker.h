#ifndef DEBINSTALLWORKER_H
#define DEBINSTALLWORKER_H

#include <QObject>

class DebPackage;
class DebInstallWorker : public QObject
{
    Q_OBJECT

public:
    explicit DebInstallWorker(QObject *parent = 0);

public slots:
    void startInstall();
};

#endif // DEBINSTALLWORKER_H
