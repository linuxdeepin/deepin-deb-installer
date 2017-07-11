#include "packagelistview.h"

PackagesListView::PackagesListView(QWidget *parent)
    : QListView(parent)
{
    setVerticalScrollMode(ScrollPerPixel);
    setSelectionMode(NoSelection);
    setAutoScroll(false);
}
