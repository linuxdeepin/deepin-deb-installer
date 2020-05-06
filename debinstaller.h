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

#ifndef DEBINSTALLER_H
#define DEBINSTALLER_H

#include <QPointer>
#include <QSettings>
#include <QStackedLayout>


#include <DMainWindow>
#include <QWidget>
DWIDGET_USE_NAMESPACE
class FileChooseWidget;
class DebListModel;
class SingleInstallPage;
class DebInstaller : public Dtk::Widget::DMainWindow
{
    Q_OBJECT

public:
    DebInstaller(QWidget *parent = nullptr);
    virtual ~DebInstaller() Q_DECL_OVERRIDE;

protected:
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *e) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *e) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *e) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void onPackagesSelected(const QStringList &packages);
    void showUninstallConfirmPage();
    void onUninstallAccepted();
    void onUninstallCalceled();
    void onAuthing(const bool authing);
    void onNewAppOpen(qint64 pid, const QStringList &arguments);
    void onStartInstallRequested();

    void reset();
    void removePackage(const int index);
    void changeDragFlag();
    void showHiddenButton();

private:
    void failToSysteminitUI();
    void initUI();
    void initConnections();
    void refreshInstallPage();
    void handleFocusPolicy();

    //检查黑白名单是否存在
    void checkWhiteList();
    void popErrorWindow();

    //禁用/启用 关闭按钮和菜单中的退出
    void disableCloseAndExit();
    void enableCloseAndExit();

    void sendMessage(QWidget *par, DFloatingMessage *floMsg);
    SingleInstallPage *backToSinglePage();

private:
    DebListModel *m_fileListModel;
    FileChooseWidget *m_fileChooseWidget;
    QStackedLayout *m_centralLayout;

    QPointer<QWidget> m_lastPage;
    int m_dragflag;
    bool refresh = true;
};

#endif  // DEBINSTALLER_H
