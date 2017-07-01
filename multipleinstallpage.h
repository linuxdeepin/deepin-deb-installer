#ifndef MULTIPLEINSTALLPAGE_H
#define MULTIPLEINSTALLPAGE_H

#include <QWidget>

#include <QPushButton>

class PackagesListView;
class DebListModel;
class MultipleInstallPage : public QWidget
{
    Q_OBJECT

public:
    explicit MultipleInstallPage(DebListModel *model, QWidget *parent = 0);

private slots:
    void onWorkerStarted();
    void onWorkerFinshed();

private:
    DebListModel *m_debListModel;
    PackagesListView *m_appsView;
    QPushButton *m_installButton;
    QPushButton *m_acceptButton;
};

#endif // MULTIPLEINSTALLPAGE_H
