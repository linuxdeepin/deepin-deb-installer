/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "infocontrolbutton.h"

#include <QVBoxLayout>
#include <QPixmap>
#include <QIcon>
#include <QPalette>

InfoControlButton::InfoControlButton(const QString &expandTips, const QString &shrinkTips, DWidget *parent)
    : DWidget(parent),
      m_expand(false),
      m_expandTips(expandTips),
      m_shrinkTips(shrinkTips),
      m_arrowIcon(new DLabel),
      m_tipsText(new DLabel),
      m_font(QFont("SourceHanSansSC-Normal"))
{

    m_font.setPixelSize(12);
    QPalette pe;
    pe.setColor(QPalette::WindowText, QColor("#0082FA"));
    m_arrowIcon->setAlignment(Qt::AlignCenter);
    m_arrowIcon->setPixmap(QIcon(":/images/arrow_up.svg").pixmap(18, 7));//21.8
    m_tipsText->setAlignment(Qt::AlignCenter);
    m_tipsText->setFont(m_font);
    m_tipsText->setText(expandTips);
    m_tipsText->setPalette(pe);
    m_tipsText->setObjectName("TipsText");

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
    DWidget::mouseReleaseEvent(e);

    onMouseRelease();
}

void InfoControlButton::onMouseRelease()
{
    if (m_expand)
        emit shrink();
    else
        emit expand();

    m_expand = !m_expand;

    if (!m_expand) {
        m_arrowIcon->setPixmap(QIcon(":/images/arrow_up.svg").pixmap(21, 8));
        m_tipsText->setText(m_expandTips);
        setFixedSize(200, 33);
    } else {
        m_arrowIcon->setPixmap(QIcon(":/images/arrow_down.svg").pixmap(21, 8));
        m_tipsText->setText(m_shrinkTips);
        setFixedSize(200, 28);
    }
}
void InfoControlButton::setShowText(const QString text)
{
    m_expandTips = text;
    m_tipsText->setText(m_expandTips);
}
