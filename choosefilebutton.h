#ifndef CHOOSEFILEBUTTON_H
#define CHOOSEFILEBUTTON_H

#include <DPushButton>

DWIDGET_USE_NAMESPACE

class ChooseFileButton : public DPushButton
{
    Q_OBJECT
public:
    explicit ChooseFileButton(DWidget *parent = nullptr);
};

#endif // CHOOSEFILEBUTTON_H
