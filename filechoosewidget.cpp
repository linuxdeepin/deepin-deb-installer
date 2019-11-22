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

FileChooseWidget::FileChooseWidget(QWidget *parent)
    : QWidget(parent)
    , m_settings("deepin", "deepin-deb-install")
{
    const auto ratio = devicePixelRatioF();
    setFocusPolicy(Qt::NoFocus);

    DPalette palette;

    QPixmap iconPix = Utils::renderSVG(":/images/icon.svg", QSize(160, 160));
    iconPix.setDevicePixelRatio(ratio);
    DLabel *iconImage = new DLabel;
    iconImage->setFixedSize(160, 160);
    iconImage->setPixmap(iconPix);

#ifdef SHOWBORDER
    iconImage->setStyleSheet("QLabel{border:1px solid black;}");
#endif
    m_dndTips = new DLabel(this);
    m_dndTips->setText(tr("Drag and drop file here"));
    m_dndTips->setObjectName("DNDTips");
    m_dndTips->setFixedHeight(15);
    palette = DebApplicationHelper::instance()->palette(m_dndTips);
    palette.setBrush(DPalette::WindowText, palette.color(DPalette::ToolTipText));
    m_dndTips->setPalette(palette);

    QString fontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    Utils::bindFontBySizeAndWeight(m_dndTips, fontFamily, 12, QFont::ExtraLight);

#ifdef SHOWBORDER
    dndTips->setStyleSheet("QLabel{border:1px solid black;}");
#endif
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    split_line = new DLabel;
    split_line->setObjectName("SplitLine");
    if(themeType == DGuiApplicationHelper::LightType)
        split_line->setPixmap(Utils::renderSVG(":/images/split_line.svg", QSize(220, 3)));
    else if(themeType == DGuiApplicationHelper::DarkType)
        split_line->setPixmap(Utils::renderSVG(":/images/split_line_dark.svg", QSize(220, 3)));
    else
        split_line->setPixmap(Utils::renderSVG(":/images/split_line.svg", QSize(220, 3)));
    split_line->setFixedHeight(3);

    m_chooseFileBtn = new ChooseFileButton;

    m_chooseFileBtn->setFixedHeight(28);
    m_chooseFileBtn->setObjectName("FileChooseButton");
    m_chooseFileBtn->setText(tr("Select File"));
#ifdef SHOWBORDER
    m_chooseFileBtn->setStyleSheet("QPushButton{border:1px solid black;}");
#endif

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addSpacing(32);
    centralLayout->addWidget(iconImage);
    centralLayout->setAlignment(iconImage, Qt::AlignHCenter);

    centralLayout->addSpacing(8);
    centralLayout->addWidget(m_dndTips);
    centralLayout->setAlignment(m_dndTips, Qt::AlignHCenter);

    centralLayout->addSpacing(16);
    centralLayout->addWidget(split_line);
    centralLayout->setAlignment(split_line, Qt::AlignHCenter);

    centralLayout->addSpacing(11);
    centralLayout->addWidget(m_chooseFileBtn);
    centralLayout->setAlignment(m_chooseFileBtn, Qt::AlignHCenter);
    centralLayout->addStretch();

    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(centralLayout);
    connect(m_chooseFileBtn, &ChooseFileButton::clicked, this, &FileChooseWidget::chooseFiles);

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

void FileChooseWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    DPalette palette = DebApplicationHelper::instance()->palette(m_dndTips);
    palette.setBrush(DPalette::WindowText, palette.color(DPalette::ToolTipText));
    m_dndTips->setPalette(palette);
}

void FileChooseWidget::themeChanged()
{
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();

    if(themeType == DGuiApplicationHelper::LightType)
        split_line->setPixmap(Utils::renderSVG(":/images/split_line.svg", QSize(220, 3)));
    else if(themeType == DGuiApplicationHelper::DarkType)
        split_line->setPixmap(Utils::renderSVG(":/images/split_line_dark.svg", QSize(220, 3)));
    else
        split_line->setPixmap(Utils::renderSVG(":/images/split_line.svg", QSize(220, 3)));
}
