#ifndef DEBINSTALLER_H
#define DEBINSTALLER_H

#include <QWidget>
#include <QStackedLayout>

class FileChooseWidget;
class DebInstaller : public QWidget
{
    Q_OBJECT

public:
    DebInstaller(QWidget *parent = 0);
    ~DebInstaller();

protected:
    void keyPressEvent(QKeyEvent *e);

private:
    QStackedLayout *m_centralLayout;
    FileChooseWidget *m_fileChooseWidget;
};

#endif // DEBINSTALLER_H
