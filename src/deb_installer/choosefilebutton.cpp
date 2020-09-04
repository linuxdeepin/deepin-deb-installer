#include "choosefilebutton.h"
#include "utils.h"

#include <DApplicationHelper>

#include <QKeyEvent>

ChooseFileButton::ChooseFileButton(QString text, QWidget *parent)
    : DCommandLinkButton(text, parent)
{
    QString fontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    Utils::bindFontBySizeAndWeight(this, fontFamily, 12, QFont::ExtraLight);

    this->setFocusPolicy(Qt::TabFocus);

}

/**
 * @brief ChooseFileButton::keyPressEvent 添加键盘响应。
 * @param event
 */
void ChooseFileButton::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Space || event->key() == Qt::Key_Enter) {
        if (this->hasFocus()) {
            this->clicked();
        }
    }
}
