#ifndef CHOOSEFILEBUTTON_H
#define CHOOSEFILEBUTTON_H

#include <DLabel>
#include <DCommandLinkButton>

DWIDGET_USE_NAMESPACE

class ChooseFileButton : public DCommandLinkButton
{
    Q_OBJECT
public:
    //fix bug:33999 change DButton to DCommandLinkButton for Activity color
    explicit ChooseFileButton(QString text, QWidget *parent = nullptr);
};

#endif // CHOOSEFILEBUTTON_H
