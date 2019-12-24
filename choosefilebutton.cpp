#include "choosefilebutton.h"
#include "utils.h"

#include <DApplicationHelper>

ChooseFileButton::ChooseFileButton(QWidget *parent)
    :DPushButton(parent)
{
    QString fontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    Utils::bindFontBySizeAndWeight(this, fontFamily, 12, QFont::ExtraLight);

    DPalette palette = DApplicationHelper::instance()->palette(this);
//    palette.setColor(DPalette::ButtonText, palette.color(DPalette::TextLively));
    palette.setColor(DPalette::ButtonText, QColor(00,130,252));
    this->setPalette(palette);

    this->setFlat(true);
    this->setFocusPolicy(Qt::NoFocus);
}
