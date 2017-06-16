#ifndef FILECHOOSEWIDGET_H
#define FILECHOOSEWIDGET_H

#include <QWidget>

class FileChooseWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileChooseWidget(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
};

#endif // FILECHOOSEWIDGET_H
