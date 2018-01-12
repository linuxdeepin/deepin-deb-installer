/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
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

#include <QStackedLayout>
#include <QPointer>

#include <DMainWindow>

class FileChooseWidget;
class DebListModel;
class SingleInstallPage;
class DebInstaller : public Dtk::Widget::DMainWindow
{
    Q_OBJECT

public:
    DebInstaller(QWidget *parent = 0);
    ~DebInstaller();

protected:
    void keyPressEvent(QKeyEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);

private slots:
    void onPackagesSelected(const QStringList &packages);
    void showUninstallConfirmPage();
    void onUninstallAccepted();
    void onUninstallCalceled();
    void onAuthing(const bool authing);

    void reset();
    void removePackage(const int index);

private:
    void refreshInstallPage();
    SingleInstallPage *backToSinglePage();

private:
    QPointer<QWidget> m_lastPage;
    DebListModel *m_fileListModel;

    QStackedLayout *m_centralLayout;
    FileChooseWidget *m_fileChooseWidget;
};

#endif // DEBINSTALLER_H
