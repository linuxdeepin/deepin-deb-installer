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
#include <QDir>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <DLabel>
#include <QMimeData>
#include <QPainter>
#include <QPixmap>
#include <QUrl>
#include <QVBoxLayout>



FileChooseWidget::FileChooseWidget(DWidget *parent)
    : DWidget(parent)
    , m_settings("deepin", "deepin-deb-install")
{
    const auto ratio = devicePixelRatioF();
    setFocusPolicy(Qt::ClickFocus);

    QFont font = this->font();

    QPixmap iconPix = Utils::renderSVG(":/images/icon.svg", QSize(160, 160));
    iconPix.setDevicePixelRatio(ratio);
    DLabel *iconImage = new DLabel;
    iconImage->setFixedSize(160, 160);
    iconImage->setAlignment(Qt::AlignCenter);
    iconImage->setPixmap(iconPix);
#ifdef SHOWBORDER
    iconImage->setStyleSheet("QLabel{border:1px solid black;}");
#endif
    DLabel *dndTips = new DLabel;
    dndTips->setText(tr("Drag and drop file here"));
    dndTips->setAlignment(Qt::AlignHCenter);
    dndTips->setObjectName("DNDTips");
    font.setPixelSize(12);
    dndTips->setFont(font);
    dndTips->setFixedHeight(15);
#ifdef SHOWBORDER
    dndTips->setStyleSheet("QLabel{border:1px solid black;}");
#endif
    QVBoxLayout *centerWrapLayout = new QVBoxLayout;
    centerWrapLayout->addWidget(iconImage);
    centerWrapLayout->setAlignment(iconImage, Qt::AlignTop | Qt::AlignHCenter);
    centerWrapLayout->addSpacing(28);
    centerWrapLayout->addWidget(dndTips, Qt::AlignHCenter);
    centerWrapLayout->setSpacing(0);
    centerWrapLayout->setContentsMargins(0, 0, 0, 0);

    DWidget *centerWidget = new DFrame;
    centerWidget->setFixedWidth(270);
#ifdef SHOWBORDER
centerWidget->setStyleSheet("QWidget{border:1px solid black;}");
#endif
    centerWidget->setLayout(centerWrapLayout);
    centerWidget->setObjectName("CenterWidget");
    DLabel *split_line = new DLabel;
    split_line->setObjectName("SplitLine");
    split_line->setPixmap(QPixmap(":/images/split_line.svg"));
    split_line->setAlignment(Qt::AlignCenter);

    m_fileChooseBtn = new DPushButton;
    QColor color("#0066EC");
    QPalette palette = m_fileChooseBtn->palette();
    palette.setColor(QPalette::ButtonText, color);
    m_fileChooseBtn->setPalette(palette);
    m_fileChooseBtn->setFixedSize(100, 28);
    m_fileChooseBtn->setObjectName("FileChooseButton");
    m_fileChooseBtn->setText(tr("Select File"));
    m_fileChooseBtn->setFlat(true);
    font.setPixelSize(12);
    m_fileChooseBtn->setFont(font);
#ifdef SHOWBORDER
    m_fileChooseBtn->setStyleSheet("QPushButton{border:1px solid black;}");
#endif
    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addSpacing(32);
    centralLayout->addWidget(centerWidget);
    centralLayout->setAlignment(centerWidget, Qt::AlignTop | Qt::AlignCenter);
    centralLayout->addSpacing(17);
    centralLayout->addWidget(split_line);
    centralLayout->addSpacing(12);
    centralLayout->addWidget(m_fileChooseBtn);
    centralLayout->setAlignment(m_fileChooseBtn, Qt::AlignCenter);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 0, 0, 60);

    setLayout(centralLayout);
    connect(m_fileChooseBtn, &DPushButton::clicked, this, &FileChooseWidget::chooseFiles);
}

void FileChooseWidget::chooseFiles()
{
    QString historyDir = m_settings.value("history_dir").toString();

    if (historyDir.isEmpty()) {
        historyDir = QDir::homePath();
    }

    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter("Debian Package Files (*.deb)");
    dialog.setDirectory(historyDir);

    const int mode = dialog.exec();

    // save the directory string to config file.
    m_settings.setValue("history_dir", dialog.directoryUrl().toLocalFile());

    if (mode != QDialog::Accepted) return;

    const QStringList selected_files = dialog.selectedFiles();

    emit packagesSelected(selected_files);
}
