/*
* Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
*
* Author:     liupeng <liupeng@uniontech.com>
* Maintainer: liupeng <liupeng@uniontech.com>
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
#ifndef INSTALLDEBTHREAD_H
#define INSTALLDEBTHREAD_H

//#include <QObject>
#include <QThread>
#include <QProcess>

class InstallDebThread: public QThread
{
    Q_OBJECT
public:
    InstallDebThread();
    virtual ~InstallDebThread();
    void run();
    int m_resultFlag = -1;
    void setDependList(QStringList depends);

public slots:
    void onFinished(int);
    void on_readoutput();
private:
    QProcess *m_proc;
    QStringList m_dependList;
};
#endif // INSTALLDEBTHREAD_H
