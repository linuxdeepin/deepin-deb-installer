#ifndef PACKAGESLISTVIEW_H
#define PACKAGESLISTVIEW_H

#include <QListView>

class PackagesListView : public QListView
{
    Q_OBJECT

public:
    explicit PackagesListView(QWidget *parent = 0);
};

#endif // PACKAGESLISTVIEW_H
