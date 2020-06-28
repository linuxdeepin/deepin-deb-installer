#ifndef LOADDEBFILELISTTHREAD_H
#define LOADDEBFILELISTTHREAD_H
#include <QThread>
#include <QApt/DebFile>


class LoadDebFileListThread : public QThread
{

public:
    LoadDebFileListThread(QString package, QString tempFilePath);
    ~LoadDebFileListThread();
    void run();
private:
    QString m_filePath;
    QString m_package;
    QApt::DebFile *deb;
};


#endif // LOADDEBFILELISTTHREAD_H
