/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "filechoosewidget.h"
#include "utils.h"

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
    const auto ratio = devicePixelRatioF();
    QPixmap iconPix = Utils::renderSVG(":/images/icon.svg", QSize(140, 140));
    iconPix.setDevicePixelRatio(ratio);

    QLabel *iconImage = new QLabel;
    iconImage->setFixedSize(140, 140);
    iconImage->setPixmap(iconPix);

    QLabel *dndTips = new QLabel;
    dndTips->setText(tr("Drag and drop file here"));
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
                                "}");

    QLabel *split_line = new QLabel;
    split_line->setPixmap(QPixmap(":/images/split_line.svg"));
    split_line->setAlignment(Qt::AlignCenter);

    m_fileChooseBtn = new DLinkButton;
    m_fileChooseBtn->setText(tr("Select File"));

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addStretch();
    centralLayout->addWidget(centerWidget);
    centralLayout->setAlignment(centerWidget, Qt::AlignTop | Qt::AlignCenter);
    centralLayout->addWidget(split_line);
    centralLayout->addSpacing(20);
    centralLayout->addWidget(m_fileChooseBtn);
    centralLayout->setAlignment(m_fileChooseBtn, Qt::AlignCenter);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 0, 0, 60);

    setLayout(centralLayout);

    connect(m_fileChooseBtn, &QPushButton::clicked, this, &FileChooseWidget::chooseFiles);
}

void FileChooseWidget::chooseFiles()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter("Debian Pakcage Files (*.deb)");

    if (dialog.exec() != QDialog::Accepted)
        return;

    const QStringList selected_files = dialog.selectedFiles();

    emit packagesSelected(selected_files);
}
