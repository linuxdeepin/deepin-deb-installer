#include "GetStatusThread.h"
#include "package/Package.h"

#include <QMetaType>

#include <QFile>

GetStatusThread::GetStatusThread(PackageStatus *packageStatus)
{
    qRegisterMetaType<DependsStatus>("DependsStatus");//注册PortConfig类型
    qRegisterMetaType<InstallStatus>("InstallStatus");
    m_pPackageStatus = packageStatus;
}

void GetStatusThread::setPackage(int index, QString packagePath)
{
    m_index  = index;
    m_packagePath = packagePath;
}

void GetStatusThread::run()
{
    QTime dependsTime;
    dependsTime.start();
    QFile debFile(m_packagePath);

    if (!debFile.exists()) {
        qInfo() << "GetStatusThread" << "run" << "getPackageDeoebdsStatus" << "文件不存在";
    }
    DependsStatus DependsStatus = m_pPackageStatus->getPackageDependsStatus(m_packagePath);

    qInfo() << "GetStatusThread" << "run" << "getPackageDeoebdsStatus" << "用时" << dependsTime.elapsed();
    emit signal_dependsStatus(m_index, DependsStatus);

    QTime installTime;
    InstallStatus installStatus = m_pPackageStatus->getPackageInstallStatus(m_packagePath);
    qInfo() << "GetStatusThread" << "run" << "getInstallStatus" << "用时" << installTime.elapsed();
    emit signal_installStatus(m_index, installStatus);
}
