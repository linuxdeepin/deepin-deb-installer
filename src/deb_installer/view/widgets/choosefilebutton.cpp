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

/**
 * @brief ChooseFileButton::keyPressEvent 添加键盘响应。
 * @param event
 */
void ChooseFileButton::keyPressEvent(QKeyEvent *event)
{
    // 当按下回车、换行、或空格时，如果当前按钮存在焦点则触发click事件
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Space || event->key() == Qt::Key_Enter) {
        if (this->hasFocus()) {
            this->clicked();
        }
    }
}
