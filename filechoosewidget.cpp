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
#include <DApplicationHelper>
#include <DStyleHelper>

FileChooseWidget::FileChooseWidget(DWidget *parent)
    : DWidget(parent)
    , m_settings("deepin", "deepin-deb-install")
{
    const auto ratio = devicePixelRatioF();
    setFocusPolicy(Qt::ClickFocus);

    QFont font = this->font();
    DPalette palette;

    QPixmap iconPix = Utils::renderSVG(":/images/icon.svg", QSize(160, 160));
    iconPix.setDevicePixelRatio(ratio);
    DLabel *iconImage = new DLabel;
    iconImage->setFixedSize(160, 160);
    iconImage->setPixmap(iconPix);

#ifdef SHOWBORDER
    iconImage->setStyleSheet("QLabel{border:1px solid black;}");
#endif
    DLabel *dndTips = new DLabel(this);
    dndTips->setText(tr("Drag and drop file here"));
    dndTips->setObjectName("DNDTips");
    font.setPixelSize(12);
    font.setWeight(QFont::Normal);
    dndTips->setFont(font);
    dndTips->setFixedHeight(15);
    palette = DApplicationHelper::instance()->palette(dndTips);
    palette.setBrush(DPalette::Button, palette.color(DPalette::Button));
    dndTips->setPalette(palette);

#ifdef SHOWBORDER
    dndTips->setStyleSheet("QLabel{border:1px solid black;}");
#endif
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    split_line = new DLabel;
    split_line->setObjectName("SplitLine");
    if(themeType == DGuiApplicationHelper::LightType)
        split_line->setPixmap(QPixmap(":/images/split_line.svg"));
    else if(themeType == DGuiApplicationHelper::DarkType)
        split_line->setPixmap(QPixmap(":/images/split_line_dark.svg"));
    else
        split_line->setPixmap(QPixmap(":/images/split_line.svg"));
    split_line->setFixedHeight(3);

    m_fileChooseBtn = new DPushButton;
    palette = DApplicationHelper::instance()->palette(m_fileChooseBtn);
    palette.setColor(DPalette::ButtonText, palette.color(DPalette::Highlight));
    m_fileChooseBtn->setPalette(palette);

    m_fileChooseBtn->setFixedHeight(28);
    m_fileChooseBtn->setObjectName("FileChooseButton");
    m_fileChooseBtn->setText(tr("Select File"));
    m_fileChooseBtn->setFlat(true);
    font.setPixelSize(12);
    font.setWeight(QFont::Normal);
    m_fileChooseBtn->setFont(font);
#ifdef SHOWBORDER
    m_fileChooseBtn->setStyleSheet("QPushButton{border:1px solid black;}");
#endif

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addSpacing(32);
    centralLayout->addWidget(iconImage);
    centralLayout->setAlignment(iconImage, Qt::AlignHCenter);

    centralLayout->addSpacing(7);
    centralLayout->addWidget(dndTips);
    centralLayout->setAlignment(dndTips, Qt::AlignHCenter);

    centralLayout->addSpacing(16);
    centralLayout->addWidget(split_line);
    centralLayout->setAlignment(split_line, Qt::AlignHCenter);

    centralLayout->addSpacing(14);
    centralLayout->addWidget(m_fileChooseBtn);
    centralLayout->setAlignment(m_fileChooseBtn, Qt::AlignHCenter);
    centralLayout->addStretch();

    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(centralLayout);
    connect(m_fileChooseBtn, &DPushButton::clicked, this, &FileChooseWidget::chooseFiles);

    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                        this, &FileChooseWidget::themeChanged);
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
void FileChooseWidget::themeChanged()
{
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();

    if(themeType == DGuiApplicationHelper::LightType)
        split_line->setPixmap(QPixmap(":/images/split_line.svg"));
    else if(themeType == DGuiApplicationHelper::DarkType)
        split_line->setPixmap(QPixmap(":/images/split_line_dark.svg"));
    else
        split_line->setPixmap(QPixmap(":/images/split_line.svg"));

}
