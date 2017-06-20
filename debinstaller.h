#ifndef DEBINSTALLER_H
#define DEBINSTALLER_H

#include <QStackedLayout>

#include <DWindow>

class FileChooseWidget;
class DebInstallWorker;
class DebListModel;
class DebInstaller : public Dtk::Widget::DWindow
{
    Q_OBJECT

public:
    DebInstaller(QWidget *parent = 0);
    ~DebInstaller();

protected:
    void keyPressEvent(QKeyEvent *e);

private slots:
    void onPackagesSelected(const QStringList &packages);

private:
    QStackedLayout *m_centralLayout;
    FileChooseWidget *m_fileChooseWidget;
    DebInstallWorker *m_installWorker;
    DebListModel *m_fileListModel;
};

#endif // DEBINSTALLER_H
