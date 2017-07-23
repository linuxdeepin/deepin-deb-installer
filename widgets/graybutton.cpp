#include "graybutton.h"

GrayButton::GrayButton(QWidget *parent)
    : QPushButton(parent)
{
    setFixedSize(120, 36);
    setStyleSheet("GrayButton {"
                  "color: #303030;"
                  "border: 1px solid rgba(0, 0, 0, .1);"
                  "border-radius: 4px;"
                  "}");
}
