#ifndef ADDPACKAGETHREAD_H
#define ADDPACKAGETHREAD_H

#include <QWidget>
#include <QPointer>
#include <QThread>

class DebListModel;

class AddPackageThread : public QThread
{
    Q_OBJECT
public:
    AddPackageThread(DebListModel *fileListModel, QPointer<QWidget> lastPage, QStringList packages, QWidget *widget);
    void run();

signals:
    void appendPackageFinish(int idx = 0);

private:
    DebListModel *m_fileListModel;
    QPointer<QWidget> m_lastPage;
    QStringList m_packages;
    QWidget *m_pwidget;

};

#endif // ADDPACKAGETHREAD_H
