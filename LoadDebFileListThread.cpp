#include "LoadDebFileListThread.h"

LoadDebFileListThread::LoadDebFileListThread(QString package, QString tempFilePath)
{
    m_package = package;
    m_filePath = tempFilePath;
    deb = new QApt::DebFile(m_package);
}

LoadDebFileListThread::~LoadDebFileListThread()
{
    delete deb;
}

void LoadDebFileListThread::run()
{
    deb->extractArchive(m_filePath);
}
