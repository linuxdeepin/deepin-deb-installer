/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "infocontrolbutton.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>

InfoControlButton::InfoControlButton(const QString &expandTips, const QString &shrinkTips, QWidget *parent)
    : QWidget(parent),
      m_expand(false),
      m_expandTips(expandTips),
      m_shrinkTips(shrinkTips),

      m_arrowIcon(new QLabel),
      m_tipsText(new QLabel)
{
    m_arrowIcon->setAlignment(Qt::AlignCenter);
    m_arrowIcon->setPixmap(QPixmap(":/images/arrow_up.png"));
    m_tipsText->setAlignment(Qt::AlignCenter);
    m_tipsText->setText(expandTips);
    m_tipsText->setStyleSheet("QLabel {"
                              "color: #6a6a6a;"
                              "}");

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addSpacing(2);
    centralLayout->addWidget(m_arrowIcon);
    centralLayout->addStretch();
    centralLayout->addWidget(m_tipsText);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(centralLayout);
    setFixedSize(200, 33);
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
        m_tipsText->setText(m_expandTips);
        setFixedSize(200, 33);
    } else {
        m_arrowIcon->setPixmap(QPixmap(":/images/arrow_down.png"));
        m_tipsText->setText(m_shrinkTips);
        setFixedSize(200, 28);
    }
}
