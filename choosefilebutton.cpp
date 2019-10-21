#include "choosefilebutton.h"
#include "utils.h"

#include <DApplicationHelper>

ChooseFileButton::ChooseFileButton(DWidget *parent):DLabel(parent)
{
    QString fontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    QFont font = Utils::loadFontBySizeAndWeight(fontFamily, 12, QFont::ExtraLight);
    this->setFont(font);

    DPalette palette = DApplicationHelper::instance()->palette(this);
    palette.setColor(DPalette::WindowText, palette.color(DPalette::Highlight));
    this->setPalette(palette);
}

void ChooseFileButton::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    emit clicked();
}
