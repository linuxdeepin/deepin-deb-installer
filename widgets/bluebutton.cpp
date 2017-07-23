#include "bluebutton.h"

BlueButton::BlueButton(QWidget *parent)
    : QPushButton(parent)
{
    setFixedSize(120, 36);
    setStyleSheet("BlueButton {"
                  "color: #2ca7f8;"
                  "border: 1px solid #2ca7f8;"
                  "border-radius: 4px;"
                  "}");
}
