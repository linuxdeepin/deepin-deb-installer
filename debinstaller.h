#ifndef DEBINSTALLER_H
#define DEBINSTALLER_H

#include <QWidget>
#include <QStackedLayout>

class DebPackage;
class FileChooseWidget;
class DebInstaller : public QWidget
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

    QList<DebPackage *> m_preparedPackages;
};

#endif // DEBINSTALLER_H
