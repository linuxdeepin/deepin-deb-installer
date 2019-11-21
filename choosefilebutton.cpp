#include "choosefilebutton.h"
#include "utils.h"

#include <DApplicationHelper>

ChooseFileButton::ChooseFileButton(QWidget *parent)
    :DPushButton(parent)
{
    QString fontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    QFont font = Utils::loadFontBySizeAndWeight(fontFamily, 12, QFont::ExtraLight);
    this->setFont(font);

    DPalette palette = DApplicationHelper::instance()->palette(this);
    palette.setColor(DPalette::ButtonText, palette.color(DPalette::Highlight));
    this->setPalette(palette);

    this->setFlat(true);
    this->setFocusPolicy(Qt::NoFocus);
}
