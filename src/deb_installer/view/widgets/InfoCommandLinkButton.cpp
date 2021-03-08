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

#include "InfoCommandLinkButton.h"
#include "utils/utils.h"

#include <QKeyEvent>

InfoCommandLinkButton::InfoCommandLinkButton(QString text, QWidget *parent)
    : DCommandLinkButton(text, parent)
{
    QString fontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    Utils::bindFontBySizeAndWeight(this, fontFamily, 12, QFont::ExtraLight);            //设置字体大小

    this->setFocusPolicy(Qt::TabFocus);         //获取焦点

}

/**
 * @brief ChooseFileButton::keyPressEvent 添加键盘响应。
 * @param event
 */
void InfoCommandLinkButton::keyPressEvent(QKeyEvent *event)
{
    //添加回车，换行，空格键盘事件的响应
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Space || event->key() == Qt::Key_Enter) {
        if (this->hasFocus()) { //只有有焦点时才响应
            this->clicked();
        }
    }
}
