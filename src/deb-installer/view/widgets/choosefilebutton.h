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

#ifndef CHOOSEFILEBUTTON_H
#define CHOOSEFILEBUTTON_H

#include <DLabel>
#include <DCommandLinkButton>

DWIDGET_USE_NAMESPACE

/**
 * @brief The ChooseFileButton class
 * 文件选择对话框中的文件选择按钮
 * 一开始使用的控件为DPushButton ,后UI与测试提出按钮需要跟随活动色变化。后修改为DCommandLinkButton.
 */
class ChooseFileButton : public DCommandLinkButton
{
    Q_OBJECT
public:
    explicit ChooseFileButton(QString text, QWidget *parent = nullptr);

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // CHOOSEFILEBUTTON_H
