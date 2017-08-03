#include "bluebutton.h"

BlueButton::BlueButton(QWidget *parent)
    : QPushButton(parent)
{
    setFixedSize(120, 36);
    setStyleSheet("BlueButton {"
                  "color: #2ca7f8;"
                  "border: 1px solid #2ca7f8;"
                  "border-radius: 4px;"
                  "}"
                  ""
                  "BlueButton:hover {"
                  "color: white;"
                  "background-color: qlineargradient(x1:0 y1:0, x2:0 y2:1, stop:0 #8ccfff, stop:1 #4bb8ff);"
                  "}"
                  ""
                  "BlueButton:pressed {"
                  "background-color: qlineargradient(x1:0 y1:0, x2:0 y2:1, stop:0 #0b8cff, stop:1 #0aa1ff);"
                  "}");
}
