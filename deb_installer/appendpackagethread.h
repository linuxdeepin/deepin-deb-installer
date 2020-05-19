#ifndef APPENDPACKAGETHREAD_H
#define APPENDPACKAGETHREAD_H

#include <QWidget>
#include <QPointer>
#include <QThread>


class DebListModel;

class AppendPackageThread : public QThread
{
    Q_OBJECT
public:
    AppendPackageThread(DebListModel *fileListModel, QPointer<QWidget> lastPage, QStringList packages, QWidget *widget);
    void run();

signals:
    void refresh(int idx = -1);
    void packageAlreadyAdd();
    void addSingleFinish(bool enable);
    void addMultiFinish(bool enable);
    void addStart(bool enable);

private:
    DebListModel *m_fileListModel;
    QPointer<QWidget> m_lastPage;
    QStringList m_packages;
    QWidget *m_pwidget;

};
#endif // APPENDPACKAGETHREAD_H
