/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
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

#include "singleInstallerApplication.h"
#include "view/pages/debinstaller.h"

#include <DWidgetUtil>
#include <DGuiApplicationHelper>

#include <QCommandLineParser>

SingleInstallerApplication::SingleInstallerApplication(int &argc, char **argv)
    : DApplication(argc, argv)
{

}

void SingleInstallerApplication::activateWindow()
{
    if (nullptr == m_qspMainWnd.get()) {
        m_qspMainWnd.reset(new DebInstaller());
        Dtk::Widget::moveToCenter(m_qspMainWnd.get());
        m_qspMainWnd->show();
    } else {
        m_qspMainWnd->setWindowState(Qt::WindowActive);
        m_qspMainWnd->activateWindow(); // Reactive main window
        m_qspMainWnd->showNormal();     //非特效模式下激活窗口
    }
    if (m_selectedFiles.size() > 0) {
        QMetaObject::invokeMethod(m_qspMainWnd.get(), "onPackagesSelected", Qt::QueuedConnection, Q_ARG(QStringList, m_selectedFiles));
    }
}

void SingleInstallerApplication::InstallerDeb(const QStringList &debPathList)
{
    if (debPathList.size() > 0) {
        QMetaObject::invokeMethod(m_qspMainWnd.get(), "onPackagesSelected", Qt::QueuedConnection, Q_ARG(QStringList, debPathList));
    } else {
        if (m_qspMainWnd.get()) {                   //先判断当前是否已经存在一个进程。
            m_qspMainWnd.get()->activateWindow();   //特效模式下激活窗口
            m_qspMainWnd.get()->showNormal();       //无特效激活窗口
        }
    }
}

bool SingleInstallerApplication::parseCmdLine()
{
    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin Package Installer.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("filename", "Deb package path.", "file [file..]");
    parser.process(*this);
    if (!m_selectedFiles.isEmpty()) {
        m_selectedFiles.clear();
    }
    QStringList paraList = parser.positionalArguments();
    if (!paraList.isEmpty()) {
        for (auto it : paraList) {
            m_selectedFiles.append(it);
        }
    }
    if (paraList.size() > 0 && m_selectedFiles.size() == 0) {
        return false;
    }
    return true;
}

