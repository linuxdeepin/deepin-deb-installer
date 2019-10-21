#ifndef CHOOSEFILEBUTTON_H
#define CHOOSEFILEBUTTON_H

#include <DLabel>

DWIDGET_USE_NAMESPACE

class ChooseFileButton : public DLabel
{
    Q_OBJECT
public:
    explicit ChooseFileButton(DWidget *parent = nullptr);

protected:
     void mouseReleaseEvent(QMouseEvent *event);

signals:
     void clicked(void);
};

#endif // CHOOSEFILEBUTTON_H
