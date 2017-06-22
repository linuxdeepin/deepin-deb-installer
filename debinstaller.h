#ifndef DEBINSTALLER_H
#define DEBINSTALLER_H

#include <QStackedLayout>

#include <DWindow>

class FileChooseWidget;
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

    void onOutputPrinted(const QString &output) const;

private:
    DebListModel *m_fileListModel;

    QStackedLayout *m_centralLayout;
    FileChooseWidget *m_fileChooseWidget;
};

#endif // DEBINSTALLER_H
