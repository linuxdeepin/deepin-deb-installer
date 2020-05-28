#include "appendpackagethread.h"

#include "deblistmodel.h"
#include "QApt/DebFile"
#include <QDebug>
#include <DFloatingMessage>
#include <DRecentManager>
#include <DMessageManager>

DWIDGET_USE_NAMESPACE

DCORE_USE_NAMESPACE

AppendPackageThread::AppendPackageThread(DebListModel *fileListModel, QPointer<QWidget> lastPage, QStringList packages, QWidget *widget)
{
    m_fileListModel = fileListModel;
    m_lastPage = lastPage;
    m_packages = packages;
    m_pwidget = widget;
}

void AppendPackageThread::run()
{
    qDebug() << "m_fileListModel->m_workerStatus_temp+++++++" << m_fileListModel->m_workerStatus_temp;
    const int packageCount = m_fileListModel->preparedPackages().size();
    if ((!m_lastPage.isNull() && m_fileListModel->m_workerStatus_temp != DebListModel::WorkerPrepare) ||
            m_fileListModel->m_workerStatus_temp == DebListModel::WorkerProcessing ||
            m_fileListModel->m_workerStatus_temp == DebListModel::WorkerUnInstall) {
        qDebug() << "return";
        return;
    } else {
        qDebug() << "append Package";
        for (const auto &package : m_packages) {
            QApt::DebFile *p = new QApt::DebFile(package);
            if (!p->isValid()) {
                qWarning() << "package invalid: " << package;

                delete p;
                continue;
            }

            DRecentData data;
            data.appName = "Deepin Deb Installer";
            data.appExec = "deepin-deb-installer";
            DRecentManager::addItem(package, data);

            if (!m_fileListModel->getPackageIsNull()) {
                if (!m_fileListModel->appendPackage(p, false)) {
                    emit packageAlreadyAdd();
                    return;
                }
            } else {
                m_fileListModel->appendPackage(p, true);
            }
        }
        emit refresh();
    }
}
