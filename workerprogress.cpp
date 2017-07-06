#include "workerprogress.h"

WorkerProgress::WorkerProgress(QWidget *parent)
    : QProgressBar(parent)
{
    setMinimum(0);
    setMaximum(100);
    setFixedHeight(8);
    setFixedWidth(240);
    setTextVisible(false);
    setStyleSheet("QProgressBar {"
                  "border: 1px solid rgba(0, 0, 0, .03);"
                  "border-radius: 4px;"
                  "background-color: rgba(0, 0, 0, .05);"
                  "}"
                  "QProgressBar::chunk {"
                  "background-color: #378cfa"
                  "}");
}
