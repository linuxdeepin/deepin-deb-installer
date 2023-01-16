/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
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

#ifndef FILECHOOSEWIDGET_H
#define FILECHOOSEWIDGET_H

#include <DLabel>
#include <DPushButton>

#include <QSettings>
#include <QWidget>

class ChooseFileButton;

DWIDGET_USE_NAMESPACE

class FileChooseWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileChooseWidget(QWidget *parent = nullptr);

    /**
     * @brief clearChooseFileBtnFocus 清除文件选择按钮的焦点。
     */
    void clearChooseFileBtnFocus();

signals:
    /**
     * @brief packagesSelected 选择文件信号
     * @param files 通过文件选择窗口选择的文件
     */
    void packagesSelected(const QStringList &files) const;

private slots:
    /**
     * @brief chooseFiles 打开文件选择窗口，选择文件后将文件保存
     */
    void chooseFiles();
    /**
     * @brief themeChanged 更换主题时，修改UI
     */
    void themeChanged();
private:
    ChooseFileButton    *m_chooseFileBtn    = nullptr;            //文件选择按钮
    
    DLabel              *split_line         = nullptr;                           //分割线
    DLabel              *m_dndTips          = nullptr;                            //拖入提示语
    DLabel              *m_iconImage        = nullptr;                          //图标
    
    QSettings           m_settings;                                   //保存上次打开的文件路径
};

#endif  // FILECHOOSEWIDGET_H
