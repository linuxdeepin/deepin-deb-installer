#include "AppendLoadingWidget.h"
#include <QVBoxLayout>
#include <QDebug>

AppendLoadingWidget::AppendLoadingWidget(QWidget *parent) : QWidget(parent)
{
    qDebug() << "this is AppendLoadingWidget";
    m_pSpinner = new DSpinner(this);
//    m_pSpinner->show();
    m_pSpinner->start();
    m_pSpinner->setFixedSize(100, 100);

    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    pMainLayout->addStretch();
    pMainLayout->addWidget(m_pSpinner);
    pMainLayout->addStretch();
    pMainLayout->setAlignment(m_pSpinner, Qt::AlignCenter);

    this->setLayout(pMainLayout);
}
