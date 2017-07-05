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
#include <QLabel>

DWIDGET_USE_NAMESPACE

FileChooseWidget::FileChooseWidget(QWidget *parent)
    : QWidget(parent)
{
    QLabel *iconImage = new QLabel;
    iconImage->setFixedSize(140, 140);
    iconImage->setPixmap(QPixmap(":/images/icon.png"));

    QLabel *dndTips = new QLabel;
    dndTips->setText(tr("Drag and drop files here"));
    dndTips->setAlignment(Qt::AlignCenter);
    dndTips->setStyleSheet("QLabel {"
                           "color: #6a6a6a;"
                           "}");

    QVBoxLayout *centerWrapLayout = new QVBoxLayout;
    centerWrapLayout->addWidget(iconImage);
    centerWrapLayout->setAlignment(iconImage, Qt::AlignTop | Qt::AlignHCenter);
    centerWrapLayout->addSpacing(20);
    centerWrapLayout->addWidget(dndTips);
    centerWrapLayout->setSpacing(0);
    centerWrapLayout->setContentsMargins(0, 0, 0, 15);

    QWidget *centerWidget = new QFrame;
    centerWidget->setFixedWidth(240);
    centerWidget->setLayout(centerWrapLayout);
    centerWidget->setObjectName("CenterWidget");
    centerWidget->setStyleSheet("#CenterWidget {"
                                "border:none;"
                                "border-bottom:2px dashed #eee;"
                                "}");

    m_fileChooseBtn = new DLinkButton;
    m_fileChooseBtn->setText(tr("Choose Package"));

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addStretch();
    centralLayout->addWidget(centerWidget);
    centralLayout->setAlignment(centerWidget, Qt::AlignTop | Qt::AlignCenter);
    centralLayout->addSpacing(20);
    centralLayout->addWidget(m_fileChooseBtn);
    centralLayout->setAlignment(m_fileChooseBtn, Qt::AlignCenter);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 0, 0, 60);

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
