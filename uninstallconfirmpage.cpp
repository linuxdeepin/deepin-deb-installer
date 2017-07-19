#include "uninstallconfirmpage.h"

#include <QVBoxLayout>
#include <QDebug>

UninstallConfirmPage::UninstallConfirmPage(QWidget *parent)
    : QWidget(parent),

      m_tips(new QLabel),
      m_cancelBtn(new QPushButton),
      m_confirmBtn(new QPushButton)
{
    m_tips->setAlignment(Qt::AlignCenter);
    m_cancelBtn->setText(tr("Cancel"));
    m_confirmBtn->setText(tr("Confirm"));

    QHBoxLayout *btnsLayout = new QHBoxLayout;
    btnsLayout->addWidget(m_cancelBtn);
    btnsLayout->addWidget(m_confirmBtn);
    btnsLayout->setSpacing(0);
    btnsLayout->setMargin(0);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_tips);
    centralLayout->addLayout(btnsLayout);
    centralLayout->setSpacing(0);
    centralLayout->setMargin(0);

    setLayout(centralLayout);

    connect(m_cancelBtn, &QPushButton::clicked, this, &UninstallConfirmPage::canceled);
    connect(m_confirmBtn, &QPushButton::clicked, this, &UninstallConfirmPage::accepted);
}

void UninstallConfirmPage::setPackage(const QString &name)
{
    QString tips(tr("Are you sure to uninstall %1?\nAll dependencies will also be removed"));

    m_tips->setText(tips.arg(name));
}

void UninstallConfirmPage::setRequiredList(const QStringList &requiredList)
{
    qDebug() << requiredList;
}
