#ifndef LOADDEBFILELISTTHREAD_H
#define LOADDEBFILELISTTHREAD_H
#include <QThread>
#include <QApt/DebFile>


class LoadDebFileListThread : public QThread
{

public:
    LoadDebFileListThread(QApt::DebFile *deb, QString tempFilePath);

    void run();

private:
    QString m_filePath;
    QApt::DebFile *m_pDebFile;
};


#endif // LOADDEBFILELISTTHREAD_H
