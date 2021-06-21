/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      zhangkai <zhangkai@uniontech.com>
* Maintainer:  zhangkai <zhangkai@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "MainWindow.h"

#include <QGridLayout>
#include <QApplication>
#include <QDesktopWidget>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
  ,m_mainWindow(new DebInstaller(this))
{
    initUI();
}

MainWindow::~MainWindow()
{
    delete m_mainWindow;
}

void MainWindow::initUI()
{
     setWindowFlags(Qt::FramelessWindowHint);
     setAttribute(Qt::WA_TranslucentBackground);
     setAutoFillBackground(true);
     QGridLayout *mainLayout = new QGridLayout;
     mainLayout->addWidget(m_mainWindow);
     this->setLayout(mainLayout);
     QDesktopWidget* desktopWidget = QApplication::desktop();
     QRect screenRect = desktopWidget->screenGeometry();

     resize(screenRect.width(),screenRect.height());
     m_mainWindow->show();
}

void MainWindow::onPackagesSelected(const QStringList &packages)
{
    m_mainWindow->onPackagesSelected(packages);
}
