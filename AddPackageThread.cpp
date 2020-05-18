#include "AddPackageThread.h"
#include "deblistmodel.h"
#include "QApt/DebFile"
#include <QDebug>
#include <DFloatingMessage>
#include <DRecentManager>
#include <DMessageManager>

DWIDGET_USE_NAMESPACE

DCORE_USE_NAMESPACE

AddPackageThread::AddPackageThread(DebListModel *fileListModel, QPointer<QWidget> lastPage, QStringList packages, QWidget *widget)
{
    m_fileListModel = fileListModel;
    m_lastPage = lastPage;
    m_packages = packages;
    m_pwidget = widget;

}
void AddPackageThread::run()
{
    emit addStart();
    qDebug() << "m_fileListModel->m_workerStatus_temp+++++++" << m_fileListModel->m_workerStatus_temp;
    QApt::DebFile *p = nullptr;
    qDebug() << m_lastPage.isNull() << m_fileListModel->DebInstallFinishedFlag;

    if ((!m_lastPage.isNull() && m_fileListModel->m_workerStatus_temp != DebListModel::WorkerPrepare) ||
            m_fileListModel->m_workerStatus_temp == DebListModel::WorkerProcessing ||
            m_fileListModel->m_workerStatus_temp == DebListModel::WorkerUnInstall) {
        qDebug() << "return" << m_fileListModel->m_workerStatus_temp;
        return;
    } else {
        qDebug() << "append";
        for (const auto &package : m_packages) {
            qDebug() << "append" << package;
            p = new QApt::DebFile(package);
            if (!p->isValid()) {
                qWarning() << "package invalid: " << package;
                delete p;
                continue;
            }
            DRecentData data;
            data.appName = "Deepin Deb Installer";
            data.appExec = "deepin-deb-installer";
            DRecentManager::addItem(package, data);

            if (!m_fileListModel->appendPackage(p)) {
                emit packageAlreadyAdd();
            }
            if (m_packages.size() > 1 && m_fileListModel->preparedPackages().size() == 1) {
                continue;
            }
            emit refresh(-1);
        }
        usleep(250 * 1000);
        qDebug() << "emit add Finish";
        if (m_fileListModel->preparedPackages().size() > 1) {
            emit addMultiFinish(true);
        } else if (m_fileListModel->preparedPackages().size() <= 1) {
            emit addSingleFinish(true);
        }

    }
}
