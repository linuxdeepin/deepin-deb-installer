// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "removepackagesdialog.h"

#include "model/packageanalyzer.h"
#include "utils/qaptkeepmodecompat.h"

#include <QApt/Backend>
#include <QApt/Package>

#include <DCommandLinkButton>
#include <DDialog>
#include <DTextEdit>

#include <QAbstractButton>
#include <QCommandLinkButton>
#include <QCoreApplication>
#include <QIcon>
#include <QListWidget>
#include <QTimer>

DWIDGET_USE_NAMESPACE

namespace RemovePackagesDialog {

Choice exec(QWidget *parent, const QString &targetText, const QStringList &removePackages)
{
    DDialog dialog(parent);
    dialog.setModal(true);
    dialog.setIcon(QIcon::fromTheme("di_popwarning"));
    dialog.setTitle(QCoreApplication::translate("RemovePackagesDialog", "是否卸载其他软件？"));
    dialog.setMessage(QCoreApplication::translate("RemovePackagesDialog",
            "本次安装 %1 计划卸载 %2 个软件包，可能影响使用。")
            .arg(targetText)
            .arg(removePackages.size()));

    // 待卸载包列表：限高、可滚动、长包名换行
    auto *list = new QListWidget(&dialog);
    list->setWordWrap(true);
    list->setTextElideMode(Qt::ElideNone);
    list->setFixedHeight(160);
    list->addItems(removePackages);
    dialog.addContent(list);

    // 详情：默认收起
    auto *detailButton = new DCommandLinkButton(
            QCoreApplication::translate("RemovePackagesDialog", "查看详情"), &dialog);
    auto *detailView = new DTextEdit(&dialog);
    detailView->setReadOnly(true);
    detailView->setFixedHeight(100);
    detailView->setVisible(false);

    QString detailText;
    auto *backend = PackageAnalyzer::instance().backendPtr();
    for (const QString &name : removePackages) {
        if (backend) {
            if (auto *pkg = backend->package(name)) {
                detailText += QCoreApplication::translate("RemovePackagesDialog",
                        "%1  版本 %2  %3\n")
                        .arg(name)
                        .arg(pkg->installedVersion())
                        .arg(QString(pkg->architecture()));
                continue;
            }
        }
        detailText += name + '\n';
    }
    detailView->setPlainText(detailText);

    dialog.addContent(detailButton);
    dialog.addContent(detailView);
    QObject::connect(detailButton, &QCommandLinkButton::clicked, &dialog,
            [&dialog, detailView]() {
                detailView->setVisible(!detailView->isVisible());
                // 让布局先更新最小尺寸，再调整对话框大小；否则从展开切回收起时
                // 对话框高度会停留在展开后的尺寸。调整时保留当前宽度，避免窗口左右跳动。
                QTimer::singleShot(0, &dialog, [&dialog]() {
                    const int width = dialog.width();
                    dialog.adjustSize();
                    dialog.resize(width, dialog.height());
                });
            });

    dialog.addButton(QCoreApplication::translate("RemovePackagesDialog", "取消安装"),
            false, DDialog::ButtonNormal);  // 0
    dialog.addButton(QCoreApplication::translate("RemovePackagesDialog", "保留并安装"),
            true, DDialog::ButtonRecommend);  // 1
    dialog.addButton(QCoreApplication::translate("RemovePackagesDialog", "卸载并安装"),
            false, DDialog::ButtonWarning);  // 2

    // 老 libqapt 没有 keep 模式符号：禁用“保留并安装”，但不影响卸载/取消
    if (!QaptKeepModeCompat::available()) {
        if (QAbstractButton *keepButton = dialog.getButton(1)) {
            keepButton->setEnabled(false);
            keepButton->setToolTip(QCoreApplication::translate("RemovePackagesDialog",
                    "当前系统的包管理组件不支持保留安装"));
        }
        dialog.setDefaultButton(0);
    }

    // 初始焦点放在“取消安装”，“卸载并安装”不作为默认动作
    QAbstractButton *cancelButton = dialog.getButton(0);
    if (cancelButton) {
        QTimer::singleShot(0, cancelButton, [cancelButton]() {
            cancelButton->setFocus();
        });
    }

    int clickedIndex = -1;
    QObject::connect(&dialog, &DDialog::buttonClicked, &dialog,
            [&clickedIndex](int index, const QString &) { clickedIndex = index; });

    dialog.exec();

    switch (clickedIndex) {
    case 1:
        return Keep;
    case 2:
        return Uninstall;
    default:  // 取消、关闭、Esc
        return Cancel;
    }
}

}  // namespace RemovePackagesDialog
