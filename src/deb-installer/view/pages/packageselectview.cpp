// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "packageselectview.h"
#include "view/widgets/packageselectitem.h"

#include <DFrame>
#include "view/widgets/droundbgframe.h"

#include <QListWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QEvent>

// 选中框最低高度（低字号下可能使得选中框显示不全）
static const int s_MinimumBoxHeight = 32;

PackageSelectView::PackageSelectView(QWidget *parent)
    : QWidget(parent)
    , listBgFrame(new DRoundBgFrame(this, 10, 0))
    , packageListWidget(new QListWidget)
    , selectAllBox(new QCheckBox(tr("Select all")))
    , installButton(new QPushButton(tr("Install", "button")))
{
    this->setFocusPolicy(Qt::NoFocus);
    selectAllBox->setFocusPolicy(Qt::StrongFocus);
    installButton->setFocusPolicy(Qt::StrongFocus);
    installButton->setDefault(true);

    selectAllBox->setMinimumHeight(s_MinimumBoxHeight);

    //全选+安装
    auto bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(selectAllBox);
    bottomLayout->addWidget(installButton);
    bottomLayout->addWidget(new QWidget);  //占位用
    bottomLayout->setContentsMargins(10, 0, 10, 20);
    bottomLayout->setSpacing(0);
    installButton->setFixedWidth(120);

    //设置列表控件
    packageListWidget->setFrameStyle(QFrame::NoFrame);
    packageListWidget->setFixedWidth(460);

    //列表区域套用圆角背景，对齐旧路径 MultipleInstallPage
    auto listLayout = new QVBoxLayout;
    listLayout->setSpacing(0);
    listLayout->setContentsMargins(0, 0, 0, 0);
    listLayout->addSpacing(15);
    listLayout->addWidget(packageListWidget);
    listLayout->addSpacing(10);
    listBgFrame->setLayout(listLayout);
    listBgFrame->setFixedWidth(460);

    //最终组装
    auto allLayout = new QVBoxLayout;
    allLayout->addWidget(listBgFrame, 1);
    allLayout->addLayout(bottomLayout);
    setLayout(allLayout);

    packageListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    connect(selectAllBox, &QCheckBox::clicked, this, &PackageSelectView::selectAll);
    connect(installButton, &QPushButton::clicked, this, &PackageSelectView::onInstallClicked);
}

void PackageSelectView::onInstallClicked()
{
    QList<int> selectIndexes;
    for (int i = 0; i != items.size(); ++i) {
        if (items[i]->isEnabled() && items[i]->isChecked()) {
            selectIndexes.push_back(i);
        }
    }
    emit packageInstallConfim(selectIndexes);
}

void PackageSelectView::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);

    // 首次展示焦点在全选(默认安装是被禁用的)
    selectAllBox->setFocus();
}

void PackageSelectView::selectAll(bool select)
{
    for (auto &item : items) {
        if (item->isEnabled()) {
            item->setChecked(select);
        }
    }
}

void PackageSelectView::checkSelect()
{
    bool isAllChecked = true;
    bool haveChecked = false;
    for (auto &item : items) {
        if (item->isEnabled() && !item->isChecked()) {
            isAllChecked = false;
        }
        if (item->isEnabled() && item->isChecked()) {
            haveChecked = true;
        }
    }
    selectAllBox->setChecked(isAllChecked);
    installButton->setEnabled(haveChecked || haveMustInstallDeb);
}

void PackageSelectView::clearDebList()
{
    packageListWidget->clear();
    items.clear();
}

void PackageSelectView::setHaveMustInstallDeb(bool have)
{
    haveMustInstallDeb = have;
    if (haveMustInstallDeb) {
        installButton->setEnabled(true);
    } else {
        checkSelect();
    }
}

void PackageSelectView::flushDebList(const QList<DebIr> &selectInfos)
{
    clearDebList(); //刷新之前做一下清理
    for (auto &eachInfo : selectInfos) {
        auto selectItem = new PackageSelectItem;
        selectItem->setDebIR(eachInfo);

        auto item = new QListWidgetItem(packageListWidget);
        item->setSizeHint(QSize(460, selectItem->sizeHint().height()));
        packageListWidget->addItem(item);
        packageListWidget->setItemWidget(item, selectItem);

        items.push_back(selectItem);
        connect(selectItem, &PackageSelectItem::checkStatusChanged, this, &PackageSelectView::checkSelect);
        connect(selectItem, &PackageSelectItem::sizeChanged, this, [this, item, selectItem]() {
            item->setSizeHint(QSize(460, selectItem->sizeHint().height()));
        });
    }
    checkSelect();
}

void PackageSelectView::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::FontChange) {
        updateItemHeights();
    }
}

void PackageSelectView::updateItemHeights()
{
    for (int i = 0; i < packageListWidget->count(); ++i) {
        auto listItem = packageListWidget->item(i);
        auto widget = packageListWidget->itemWidget(listItem);
        if (widget) {
            listItem->setSizeHint(QSize(460, widget->sizeHint().height()));
        }
    }
}
