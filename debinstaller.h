#ifndef DEBINSTALLER_H
#define DEBINSTALLER_H

#include <QWidget>

class DebInstaller : public QWidget
{
    Q_OBJECT

public:
    DebInstaller(QWidget *parent = 0);
    ~DebInstaller();
};

#endif // DEBINSTALLER_H
