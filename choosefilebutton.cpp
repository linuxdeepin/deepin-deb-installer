#include "choosefilebutton.h"
#include "utils.h"

#include <DApplicationHelper>

ChooseFileButton::ChooseFileButton(QString text, QWidget *parent)
    : DCommandLinkButton(text, parent)
{
    QString fontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    Utils::bindFontBySizeAndWeight(this, fontFamily, 12, QFont::ExtraLight);

    //fix bug:33999 change DButton to DCommandLinkButton for Activity color

//    DPalette palette = DApplicationHelper::instance()->palette(this);
//    palette.setColor(DPalette::ButtonText, palette.color(DPalette::NoType));
//    palette.setColor(DPalette::ButtonText, QColor(00, 130, 252));
//    this->setPalette(palette);

//    this->setFlat(true);
    this->setFocusPolicy(Qt::NoFocus);

}
