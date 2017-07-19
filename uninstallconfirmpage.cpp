#include "uninstallconfirmpage.h"

#include <QVBoxLayout>
#include <QDebug>

UninstallConfirmPage::UninstallConfirmPage(QWidget *parent)
    : QWidget(parent),

      m_icon(new QLabel),
      m_tips(new QLabel),
      m_infoControl(new InfoControlButton(tr("Display related packages"), tr("Collapse"))),
      m_dependsInfomation(new QTextEdit),
      m_cancelBtn(new QPushButton),
      m_confirmBtn(new QPushButton)
{
    const QIcon icon = QIcon::fromTheme("application-vnd.debian.binary-package", QIcon::fromTheme("debian-swirl"));

    m_icon->setFixedSize(48, 48);
    m_icon->setPixmap(icon.pixmap(48, 48));
    m_tips->setAlignment(Qt::AlignCenter);
    m_cancelBtn->setText(tr("Cancel"));
    m_cancelBtn->setFixedSize(120, 36);
    m_confirmBtn->setText(tr("Confirm"));
    m_confirmBtn->setFixedSize(120, 36);

    m_dependsInfomation->setReadOnly(true);
    m_dependsInfomation->setVisible(false);
    m_dependsInfomation->setAcceptDrops(false);
    m_dependsInfomation->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_dependsInfomation->setStyleSheet("QTextEdit {"
                                       "color: #609dc9;"
                                       "border: 1px solid #eee;"
                                       "margin: 10px 0 20px 0;"
                                       "}");

    QHBoxLayout *btnsLayout = new QHBoxLayout;
    btnsLayout->addStretch();
    btnsLayout->addWidget(m_cancelBtn);
    btnsLayout->addStretch();
    btnsLayout->addWidget(m_confirmBtn);
    btnsLayout->addStretch();
    btnsLayout->setSpacing(0);
    btnsLayout->setMargin(0);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_icon);
    centralLayout->setAlignment(m_icon, Qt::AlignHCenter);
    centralLayout->addWidget(m_tips);
    centralLayout->addWidget(m_infoControl);
    centralLayout->setAlignment(m_infoControl, Qt::AlignHCenter);
    centralLayout->addWidget(m_dependsInfomation);
    centralLayout->addLayout(btnsLayout);
    centralLayout->setSpacing(0);
    centralLayout->setMargin(0);

    setLayout(centralLayout);

    connect(m_cancelBtn, &QPushButton::clicked, this, &UninstallConfirmPage::canceled);
    connect(m_confirmBtn, &QPushButton::clicked, this, &UninstallConfirmPage::accepted);
    connect(m_infoControl, &InfoControlButton::expand, this, &UninstallConfirmPage::showDetail);
    connect(m_infoControl, &InfoControlButton::shrink, this, &UninstallConfirmPage::hideDetail);
}

void UninstallConfirmPage::setPackage(const QString &name)
{
    QString tips(tr("Are you sure to uninstall %1?\nAll dependencies will also be removed"));

    m_tips->setText(tips.arg(name));
}

void UninstallConfirmPage::setRequiredList(const QStringList &requiredList)
{
    m_infoControl->setVisible(!requiredList.isEmpty());
    m_dependsInfomation->setText(requiredList.join(", "));
}

void UninstallConfirmPage::showDetail()
{
    m_icon->setVisible(false);
    m_tips->setVisible(false);
    m_dependsInfomation->setVisible(true);
}

void UninstallConfirmPage::hideDetail()
{
    m_icon->setVisible(true);
    m_tips->setVisible(true);
    m_dependsInfomation->setVisible(false);
}
