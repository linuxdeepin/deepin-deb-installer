// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "noprocesswidget.h"
#include "utils/utils.h"

#include <DSpinner>
#include <DLabel>
#include <DFontSizeManager>
#include <DPalette>

#include <QVBoxLayout>

NoProcessWidget::NoProcessWidget(QWidget *parent)
    : QWidget(parent)
{
    spinner = new Dtk::Widget::DSpinner;
    spinner->setFixedSize(24, 24);

    actionTextLabel = new Dtk::Widget::DLabel;
    Dtk::Widget::DFontSizeManager::instance()->bind(actionTextLabel, Dtk::Widget::DFontSizeManager::T6, QFont::Medium);
    actionTextLabel->setForegroundRole(Dtk::Gui::DPalette::LightLively);

    auto allLayer = new QVBoxLayout;
    allLayer->addWidget(spinner, 0, Qt::AlignHCenter | Qt::AlignBottom);
    allLayer->addWidget(actionTextLabel, 0, Qt::AlignHCenter | Qt::AlignTop);
    setLayout(allLayer);
}

void NoProcessWidget::start()
{
    spinner->start();
}

void NoProcessWidget::stop()
{
    spinner->stop();
}

void NoProcessWidget::setActionText(const QString &text)
{
    actionTextLabel->setText(text);
}
