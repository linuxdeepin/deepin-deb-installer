#include "infocontrolbutton.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>

InfoControlButton::InfoControlButton(QWidget *parent)
    : QWidget(parent),
      m_expand(false),

      m_arrowIcon(new QLabel),
      m_tipsText(new QLabel)
{
    m_arrowIcon->setAlignment(Qt::AlignCenter);
    m_arrowIcon->setPixmap(QPixmap(":/images/arrow_up.png"));
    m_tipsText->setAlignment(Qt::AlignCenter);
    m_tipsText->setText(tr("Display installation process"));
    m_tipsText->setStyleSheet("QLabel {"
                              "color: #6a6a6a;"
                              "}");

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_arrowIcon);
    centralLayout->addWidget(m_tipsText);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(centralLayout);
    setFixedSize(200, 25);
}

void InfoControlButton::mouseReleaseEvent(QMouseEvent *e)
{
    QWidget::mouseReleaseEvent(e);

    onMouseRelease();
}

void InfoControlButton::onMouseRelease()
{
    if (m_expand)
        emit shrink();
    else
        emit expand();

    m_expand = !m_expand;

    if (!m_expand)
    {
        m_arrowIcon->setPixmap(QPixmap(":/images/arrow_up.png"));
        m_tipsText->setText(tr("Display installation process"));
    } else {
        m_arrowIcon->setPixmap(QPixmap(":/images/arrow_down.png"));
        m_tipsText->setText(tr("Collapse"));
    }
}
