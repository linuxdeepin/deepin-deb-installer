#ifndef DEBINSTALLER_H
#define DEBINSTALLER_H

#include <QStackedLayout>
#include <QPointer>

#include <DMainWindow>

class FileChooseWidget;
class DebListModel;
class SingleInstallPage;
class DebInstaller : public Dtk::Widget::DMainWindow
{
    Q_OBJECT

public:
    DebInstaller(QWidget *parent = 0);
    ~DebInstaller();

protected:
    void keyPressEvent(QKeyEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);

private slots:
    void onPackagesSelected(const QStringList &packages);
    void showUninstallConfirmPage();
    void onUninstallAccepted();
    void onUninstallCalceled();
    void onAuthing(const bool authing);

    void showHelp();

private:
    SingleInstallPage *backToSinglePage();

private:
    QPointer<QWidget> m_lastPage;
    DebListModel *m_fileListModel;

    QStackedLayout *m_centralLayout;
    FileChooseWidget *m_fileChooseWidget;
};

#endif // DEBINSTALLER_H
