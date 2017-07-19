#include "uninstallconfirmpage.h"

#include <QVBoxLayout>

UninstallConfirmPage::UninstallConfirmPage(QWidget *parent)
    : QWidget(parent),

      m_cancelBtn(new QPushButton),
      m_confirmBtn(new QPushButton)
{
    m_cancelBtn->setText(tr("Cancel"));
    m_confirmBtn->setText(tr("Confirm"));

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_cancelBtn);
    centralLayout->addWidget(m_confirmBtn);
    centralLayout->setSpacing(0);
    centralLayout->setMargin(0);

    setLayout(centralLayout);

    connect(m_cancelBtn, &QPushButton::clicked, this, &UninstallConfirmPage::canceled);
    connect(m_confirmBtn, &QPushButton::clicked, this, &UninstallConfirmPage::accepted);
}
