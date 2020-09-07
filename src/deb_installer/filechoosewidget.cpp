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
#include "choosefilebutton.h"
#include "utils.h"

#include <QDebug>
#include <QDir>
#include <QDragEnterEvent>
#include <DFileDialog>
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
    setFocusPolicy(Qt::NoFocus);
    setAcceptDrops(true);
    DPalette palette;

    m_iconImage = new DLabel(this);
    m_iconImage->setFixedSize(160, 160);

#ifdef SHOWBORDER
    iconImage->setStyleSheet("QLabel{border:1px solid black;}");
#endif
    m_dndTips = new DLabel(this);
    m_dndTips->setFixedHeight(30);
    m_dndTips->setAlignment(Qt::AlignTop);
    m_dndTips->setText(tr("Drag deb packages here"));
    m_dndTips->setObjectName("DNDTips");

    palette = DApplicationHelper::instance()->palette(m_dndTips);
    palette.setBrush(DPalette::WindowText, palette.color(DPalette::TextTips));
    m_dndTips->setPalette(palette);

    QString fontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    Utils::bindFontBySizeAndWeight(m_dndTips, fontFamily, 12, QFont::ExtraLight);

#ifdef SHOWBORDER
    dndTips->setStyleSheet("QLabel{border:1px solid black;}");
#endif
    split_line = new DLabel(this);
    split_line->setObjectName("SplitLine");
    QIcon icon_install = QIcon::fromTheme("di_icon_install");
    m_iconImage->setPixmap(icon_install.pixmap(QSize(160, 160)));
    QIcon icon_split_line = QIcon::fromTheme("di_split_line");
    split_line->setPixmap(icon_split_line.pixmap(QSize(220, 3)));
    split_line->setFixedHeight(3);

    //fix bug:33999 change DButton to DCommandLinkButton for Activity color
//    m_chooseFileBtn = new ChooseFileButton(this);
    m_chooseFileBtn = new ChooseFileButton("", this);
    m_chooseFileBtn->setFixedHeight(28);
    m_chooseFileBtn->setObjectName("FileChooseButton");
    m_chooseFileBtn->setText(tr("Select File"));
#ifdef SHOWBORDER
    m_chooseFileBtn->setStyleSheet("QPushButton{border:1px solid black;}");
#endif

    QVBoxLayout *centralLayout = new QVBoxLayout(this);
    centralLayout->addSpacing(32);
    centralLayout->addWidget(m_iconImage);
    centralLayout->setAlignment(m_iconImage, Qt::AlignHCenter);

    centralLayout->addSpacing(8);
    centralLayout->addWidget(m_dndTips);
    centralLayout->setAlignment(m_dndTips, Qt::AlignHCenter);

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
/**
 * @brief FileChooseWidget::chooseFiles
 * 选择文件
 */
void FileChooseWidget::chooseFiles()
{
    QString historyDir = m_settings.value("history_dir").toString();

    if (historyDir.isEmpty()) {
        historyDir = QDir::homePath();
    }

    DFileDialog dialog;
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
    QIcon icon_install = QIcon::fromTheme("di_icon_install");
    m_iconImage->setPixmap(icon_install.pixmap(QSize(160, 160)));
    QIcon icon_split_line = QIcon::fromTheme("di_split_line");
    split_line->setPixmap(icon_split_line.pixmap(QSize(220, 3)));
}

/**
 * @brief FileChooseWidget::clearChooseFileBtnFocus 在返回文件选择界面的时候清除文件选择按钮的焦点
 * fix bug: https://pms.uniontech.com/zentao/bug-view-46525.html
 */
void FileChooseWidget::clearChooseFileBtnFocus()
{
    m_chooseFileBtn->clearFocus();
}

