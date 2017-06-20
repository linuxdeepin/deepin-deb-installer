#ifndef MULTIPLEINSTALLPAGE_H
#define MULTIPLEINSTALLPAGE_H

#include <QWidget>

#include <QPushButton>

class PackagesListView;
class QAbstractListModel;
class MultipleInstallPage : public QWidget
{
    Q_OBJECT

public:
    explicit MultipleInstallPage(QAbstractListModel *model, QWidget *parent = 0);

private:
    PackagesListView *m_appsView;
    QPushButton *m_installButton;
};

#endif // MULTIPLEINSTALLPAGE_H
