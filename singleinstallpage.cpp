#include "singleinstallpage.h"

#include <QVBoxLayout>

SingleInstallPage::SingleInstallPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *centralLayout = new QVBoxLayout;

    setLayout(centralLayout);
}
