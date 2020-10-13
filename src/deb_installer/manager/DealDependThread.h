/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
*
* Author:     cuizhen <cuizhen@uniontech.com>
* Maintainer:  cuizhen <cuizhen@uniontech.com>
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
#ifndef DEALDEPENDTHREAD_H
#define DEALDEPENDTHREAD_H

#include <QObject>
#include <QThread>
#include <QProcess>

class DealDependThread : public QThread
{
    Q_OBJECT
public:
    DealDependThread(QObject *parent = nullptr);
    virtual ~DealDependThread();
    void setDependsList(QStringList dependList, int index);
    void setBrokenDepend(QString dependName);
    void run();
signals:
    void DependResult(int, int, QString);
    void enableCloseButton(bool);

public slots:
    void onFinished(int);
    void on_readoutput();
private:
    QProcess *proc;
    int m_index = -1;
    QStringList m_dependsList;
    bool bDependsStatusErr = false;
    QString m_brokenDepend;
};

#endif // DEALDEPENDTHREAD_H
