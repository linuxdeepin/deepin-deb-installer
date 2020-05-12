#include "AppendLoadingWidget.h"
#include <QVBoxLayout>
#include <QDebug>

AppendLoadingWidget::AppendLoadingWidget(QWidget *parent) : QWidget(parent)
{
    qDebug() << "this is AppendLoadingWidget";
    m_pSpinner = new DSpinner(this);
//    m_pSpinner->show();
    m_pSpinner->start();
    m_pSpinner->setFixedSize(50, 50);

    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    pMainLayout->addWidget(m_pSpinner);
    pMainLayout->setAlignment(m_pSpinner, Qt::AlignCenter);

    this->setLayout(pMainLayout);
}
