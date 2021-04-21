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

#include "choosefilebutton.h"
#include "utils/utils.h"

#include <DApplicationHelper>

#include <QKeyEvent>

ChooseFileButton::ChooseFileButton(QString text, QWidget *parent)
    : DCommandLinkButton(text, parent)
{
    QString fontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);   //设置字体
    Utils::bindFontBySizeAndWeight(this, fontFamily, 12, QFont::ExtraLight);       //设置字体样式与字体大小

    this->setFocusPolicy(Qt::TabFocus);                                            //设置本身可以被焦点选中

    // 添加AccessibleName
    this->setObjectName("ChooseFileButton");
    this->setAccessibleName("ChooseFileButton");
}

void ChooseFileButton::keyPressEvent(QKeyEvent *event)
{
    // 当按下回车、换行、或空格时，如果当前按钮存在焦点则触发click事件
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Space || event->key() == Qt::Key_Enter) {
        if (this->hasFocus()) {
            this->clicked();
        }
    }
}
