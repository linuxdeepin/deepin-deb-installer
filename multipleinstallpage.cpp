#include "multipleinstallpage.h"
#include "packagelistview.h"
#include "packageslistdelegate.h"

#include <QVBoxLayout>

MultipleInstallPage::MultipleInstallPage(QAbstractListModel *model, QWidget *parent)
    : QWidget(parent),
      m_appsView(new PackagesListView),
      m_installButton(new QPushButton)
{
    m_appsView->setModel(model);
    m_appsView->setItemDelegate(new PackagesListDelegate);
    m_installButton->setText(tr("Install"));
    m_installButton->setFixedWidth(120);

    QVBoxLayout *centralLayout = new QVBoxLayout;

    centralLayout->addWidget(m_appsView);
    centralLayout->addWidget(m_installButton);
    centralLayout->setAlignment(m_installButton, Qt::AlignHCenter);

    setLayout(centralLayout);
}
