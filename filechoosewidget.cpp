#include "filechoosewidget.h"

#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QPixmap>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>

DWIDGET_USE_NAMESPACE

FileChooseWidget::FileChooseWidget(QWidget *parent)
    : QWidget(parent),

      m_bgImage(QPixmap(":/images/img.jpg"))
{
    m_fileChooseBtn = new DLinkButton;
    m_fileChooseBtn->setText(tr("Choose Package"));

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addStretch();
    centralLayout->addWidget(m_fileChooseBtn);
    centralLayout->setAlignment(m_fileChooseBtn, Qt::AlignCenter);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 0, 0, 20);

    setLayout(centralLayout);
    setAcceptDrops(true);

    connect(m_fileChooseBtn, &QPushButton::clicked, this, &FileChooseWidget::chooseFiles);
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

        if (info.isFile() && info.suffix() == "deb")
            file_list << local_path;
        else if (info.isDir())
            file_list << QDir(local_path).entryList(QStringList() << "*.deb", QDir::Files);
    }

    emit packagesSelected(file_list);
}

void FileChooseWidget::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    const QPoint p = rect().center() - m_bgImage.rect().center();

    QPainter painter(this);
    painter.drawPixmap(p, m_bgImage);
}

void FileChooseWidget::chooseFiles()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter("Debian Pakcage Files (*.deb)");

    if (dialog.exec() != QDialog::Accepted)
        return;

    const QStringList selected_files = dialog.selectedFiles();

    qDebug() << selected_files;

    emit packagesSelected(selected_files);
}
