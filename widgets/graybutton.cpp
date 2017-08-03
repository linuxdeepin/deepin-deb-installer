#include "graybutton.h"

GrayButton::GrayButton(QWidget *parent)
    : QPushButton(parent)
{
    setFixedSize(120, 36);
    setStyleSheet("GrayButton {"
                  "color: #303030;"
                  "border: 1px solid rgba(0, 0, 0, .1);"
                  "border-radius: 4px;"
                  "}"
                  ""
                  "GrayButton:hover {"
                  "background-color: qlineargradient(x1:0 y1:0, x2:0 y2:1, stop:0 #8ccfff, stop:1 #4bb8ff);"
                  "}"
                  ""
                  "GrayButton:pressed {"
                  "background-color: qlineargradient(x1:0 y1:0, x2:0 y2:1, stop:0 #0b8cff, stop:1 #0aa1ff);"
                  "}");
}
