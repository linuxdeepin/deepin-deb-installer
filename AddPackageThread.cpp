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
                qWarning() << "package is Exist! ";

                DFloatingMessage *msg = new DFloatingMessage;
                msg->setMessage(tr("Already Added"));
                DMessageManager::instance()->sendMessage(m_pwidget, msg);
            }


        }
        qDebug() << "appendPackageFinish";
        emit appendPackageFinish(0);

    }
}
