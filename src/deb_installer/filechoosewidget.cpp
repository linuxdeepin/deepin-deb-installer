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

    // fileChooseWidget的图标
    m_iconImage = new DLabel(this);
    //添加AccessibleName
    m_iconImage->setObjectName("iconImage");
    m_iconImage->setAccessibleName("iconImage");
    m_iconImage->setFixedSize(160, 160);

#ifdef SHOWBORDER
    iconImage->setStyleSheet("QLabel{border:1px solid black;}");
#endif

    // 拖入提示语
    m_dndTips = new DLabel(this);
    m_dndTips->setFixedHeight(30);
    m_dndTips->setAlignment(Qt::AlignTop);
    m_dndTips->setText(tr("Drag deb packages here"));

    //添加AccessibleName
    m_dndTips->setObjectName("DNDTips");

    //修改字体颜色
    palette = DApplicationHelper::instance()->palette(m_dndTips);
    palette.setBrush(DPalette::WindowText, palette.color(DPalette::TextTips));
    m_dndTips->setPalette(palette);

    //修改字体大小与字体类型
    QString fontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    Utils::bindFontBySizeAndWeight(m_dndTips, fontFamily, 12, QFont::ExtraLight);

#ifdef SHOWBORDER
    dndTips->setStyleSheet("QLabel{border:1px solid black;}");
#endif

    //分割线
    split_line = new DLabel(this);

    //添加AccessibleName
    split_line->setObjectName("SplitLine");
    split_line->setAccessibleName("SplitLine");

    //显示的大图标
    QIcon icon_install = QIcon::fromTheme("di_icon_install");
    m_iconImage->setPixmap(icon_install.pixmap(QSize(160, 160)));

    //显示的分割线
    QIcon icon_split_line = QIcon::fromTheme("di_split_line");
    split_line->setPixmap(icon_split_line.pixmap(QSize(220, 3)));
    split_line->setFixedHeight(3);

    //fix bug:33999 change DButton to DCommandLinkButton for Activity color
//    m_chooseFileBtn = new ChooseFileButton(this);
    m_chooseFileBtn = new ChooseFileButton("", this);
    m_chooseFileBtn->setFixedHeight(28);

    //添加AccessibleName
    m_chooseFileBtn->setObjectName("FileChooseButton");
    m_chooseFileBtn->setAccessibleName("FileChooseButton");
    m_chooseFileBtn->setText(tr("Select File"));
#ifdef SHOWBORDER
    m_chooseFileBtn->setStyleSheet("QPushButton{border:1px solid black;}");
#endif

    //大图标的布局
    QVBoxLayout *centralLayout = new QVBoxLayout(this);
    centralLayout->addSpacing(32);
    centralLayout->addWidget(m_iconImage);
    //设置图标居中
    centralLayout->setAlignment(m_iconImage, Qt::AlignHCenter);

    //图标下是提示
    centralLayout->addSpacing(8);
    centralLayout->addWidget(m_dndTips);

    //提示居中显示
    centralLayout->setAlignment(m_dndTips, Qt::AlignHCenter);

    //分割线
    centralLayout->addWidget(split_line);
    centralLayout->setAlignment(split_line, Qt::AlignHCenter);

    centralLayout->addSpacing(11);

    //文件选择按钮
    centralLayout->addWidget(m_chooseFileBtn);

    //文件选择按钮居中显示
    centralLayout->setAlignment(m_chooseFileBtn, Qt::AlignHCenter);
    centralLayout->addStretch();

    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(centralLayout);

    //文件选择后，将数据传到chooseFiles信号中
    connect(m_chooseFileBtn, &ChooseFileButton::clicked, this, &FileChooseWidget::chooseFiles);

    //主题变换时，修改图标的颜色
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this, &FileChooseWidget::themeChanged);
}
/**
 * @brief FileChooseWidget::chooseFiles
 * 选择文件
 */
void FileChooseWidget::chooseFiles()
{
    QString historyDir = m_settings.value("history_dir").toString();        //获取保存的文件路径

    if (historyDir.isEmpty()) {
        historyDir = QDir::homePath();
    }

    DFileDialog dialog;                                                     //获取FileDialog
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter("Debian Package Files (*.deb)");
    dialog.setDirectory(historyDir);                                        //设置打开的路径为保存的路径

    const int mode = dialog.exec();                                         //打开文件选择窗口

    // save the directory string to config file.
    m_settings.setValue("history_dir", dialog.directoryUrl().toLocalFile());//保存当前文件路径

    if (mode != QDialog::Accepted) return;

    const QStringList selected_files = dialog.selectedFiles();              //获取选中的文件

    emit packagesSelected(selected_files);                                  //发送信号
}

/**
 * @brief FileChooseWidget::themeChanged 根据主题更新图标
 */
void FileChooseWidget::themeChanged()
{
    //更新icon
    QIcon icon_install = QIcon::fromTheme("di_icon_install");
    m_iconImage->setPixmap(icon_install.pixmap(QSize(160, 160)));

    //更新 分割线
    QIcon icon_split_line = QIcon::fromTheme("di_split_line");
    split_line->setPixmap(icon_split_line.pixmap(QSize(220, 3)));
}

/**
 * @brief FileChooseWidget::clearChooseFileBtnFocus 在返回文件选择界面的时候清除文件选择按钮的焦点
 * fix bug: https://pms.uniontech.com/zentao/bug-view-46525.html
 */
void FileChooseWidget::clearChooseFileBtnFocus()
{
    m_chooseFileBtn->clearFocus();                              //清除文件选择按钮的焦点
}

