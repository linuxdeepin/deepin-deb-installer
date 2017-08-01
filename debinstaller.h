#ifndef DEBINSTALLER_H
#define DEBINSTALLER_H

#include <QStackedLayout>

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

private slots:
    void onPackagesSelected(const QStringList &packages);
    void showUninstallConfirmPage();
    void onUninstallAccepted();
    void onUninstallCalceled();

private:
    SingleInstallPage *backToSinglePage();

private:
    DebListModel *m_fileListModel;

    QStackedLayout *m_centralLayout;
    FileChooseWidget *m_fileChooseWidget;
};

#endif // DEBINSTALLER_H
