#ifndef WORKERPROGRESS_H
#define WORKERPROGRESS_H

#include <QProgressBar>

class WorkerProgress : public QProgressBar
{
    Q_OBJECT

public:
    explicit WorkerProgress(QWidget *parent = 0);
};

#endif // WORKERPROGRESS_H
