#ifndef CHOOSEFILEBUTTON_H
#define CHOOSEFILEBUTTON_H

#include <DLabel>
#include <DPushButton>

DWIDGET_USE_NAMESPACE

class ChooseFileButton : public DPushButton
{
    Q_OBJECT
public:
    explicit ChooseFileButton(QWidget *parent = nullptr);
};

#endif // CHOOSEFILEBUTTON_H
