// SPDX-FileCopyrightText: 2023-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PACKAGESELECTITEM_H
#define PACKAGESELECTITEM_H

#include "model/packageselectmodel.h"

#include <QWidget>
#include <DLabel>

class QCheckBox;

class PackageSelectItem : public QWidget
{
    Q_OBJECT
public:
    PackageSelectItem(QWidget *parent = nullptr);

    void setDebIR(const DebIr &ir);
    QSize sizeHint() const override;
    bool isChecked();
    bool isEnabled();
    void setChecked(bool checked);

signals:
    void checkStatusChanged(int state);
    void sizeChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    Dtk::Widget::DLabel *nameLabel;
    Dtk::Widget::DLabel *versionLabel;
    Dtk::Widget::DLabel *descriptionLabel;
    QCheckBox *checkBox;

    /**
     * @brief updateCheckBoxIcon 使用控件级分数 DPR 渲染复选框图标，消除 Wayland 分数缩放下的锯齿
     */
    void updateCheckBoxIcon();
};

#endif // PACKAGESELECTITEM_H
