#ifndef GETSTATUSTHREAD_H
#define GETSTATUSTHREAD_H

#include "PackageStatus.h"

#include <QObject>
#include <QThread>


class Package;
class GetStatusThread : public QThread
{
    Q_OBJECT
public:

    GetStatusThread(PackageStatus *);

    void run();

    void setPackage(int index, QString packagePath);

signals:

    void signal_dependsStatus(int, DependsStatus);
    void signal_installStatus(int, InstallStatus);
private:
    int m_index = -1;
    PackageStatus *m_pPackageStatus;
    QString m_packagePath;
};

#endif // GETSTATUSTHREAD_H
