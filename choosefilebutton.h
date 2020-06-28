#ifndef CHOOSEFILEBUTTON_H
#define CHOOSEFILEBUTTON_H

#include <DLabel>
#include <DCommandLinkButton>

DWIDGET_USE_NAMESPACE

class ChooseFileButton : public DCommandLinkButton
{
    Q_OBJECT
public:
    explicit ChooseFileButton(QString text, QWidget *parent = nullptr);
};

#endif // CHOOSEFILEBUTTON_H
