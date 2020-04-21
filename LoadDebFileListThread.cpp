#include "LoadDebFileListThread.h"

LoadDebFileListThread::LoadDebFileListThread(QApt::DebFile *deb, QString tempFilePath)
{
    m_pDebFile = deb;
    m_filePath = tempFilePath;
}

void LoadDebFileListThread::run()
{
    m_pDebFile->extractArchive(m_filePath);
}
