// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESSWIDGET_H
#define PROCESSWIDGET_H

#include <QWidget>
#include <QIcon>

namespace Dtk {
namespace Widget {
class DLabel;
class DProgressBar;
}
}

class ProcessWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProcessWidget(QWidget *parent = nullptr);

    void setIcon(const QIcon &icon);
    void setMainText(const QString &text);
    void setProcessText(const QString &text);
    void setProgress(int current, int all);

protected:
    /**
     * @brief showEvent 首次显示时 window handle 已创建，用正确的分数 DPR 重新渲染图标
     */
    void showEvent(QShowEvent *event) override;

private:
    Dtk::Widget::DLabel *mainIcon;
    Dtk::Widget::DLabel *mainLabel;
    Dtk::Widget::DLabel *processTextLabel;
    Dtk::Widget::DProgressBar *processBar;

    QString processText;
    QIcon m_sourceIcon;  // 保存 setIcon() 传入的源图标，供 showEvent() 延迟渲染

    /**
     * @brief updateMainIcon 使用控件级分数 DPR 渲染主图标，消除 Wayland 分数缩放下的锯齿
     */
    void updateMainIcon();
};

#endif // PROCESSWIDGET_H
