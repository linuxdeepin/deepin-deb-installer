#include "packagelistview.h"

PackagesListView::PackagesListView(QWidget *parent)
    : QListView(parent)
{
    setVerticalScrollMode(ScrollPerPixel);
}
