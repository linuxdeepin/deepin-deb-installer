#include "filechoosewidget.h"

#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QDir>

FileChooseWidget::FileChooseWidget(QWidget *parent)
    : QWidget(parent)
{
    setAcceptDrops(true);
}

void FileChooseWidget::dragEnterEvent(QDragEnterEvent *e)
{
    auto * const mime = e->mimeData();
    if (!mime->hasUrls())
        return e->ignore();

    e->accept();
}

void FileChooseWidget::dropEvent(QDropEvent *e)
{
    auto * const mime = e->mimeData();
    if (!mime->hasUrls())
        return e->ignore();

    e->accept();

    // find .deb files
    QStringList file_list;
    for (const auto &url : mime->urls())
    {
        if (!url.isLocalFile())
            continue;
        const QString local_path = url.toLocalFile();
        const QFileInfo info(local_path);

        if (info.isFile())
            file_list << local_path;
        else if (info.isDir())
            file_list << QDir(local_path).entryList(QStringList() << "*.deb", QDir::Files);
    }

    qDebug() << file_list;
}
