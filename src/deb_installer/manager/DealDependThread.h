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

    /**
     * @brief setDependsList 添加安装列表
     * @param dependList 依赖列表
     * @param index 需要安装的依赖下标
     */
    void setDependsList(QStringList dependList, int index);

    /**
     * @brief setBrokenDepend 设置依赖不满足的依赖名称
     * @param dependName    break的依赖名
     */
    void setBrokenDepend(QString dependName);

    /**
     * @brief run 线程的运行函数
     */
    void run();
signals:

    /**
     * @brief DependResult 依赖下载的过程处理信号
     */
    void DependResult(int, int, QString);

    /**
     * @brief enableCloseButton 依赖下载时设置关闭按钮是否可用的信号
     */
    void enableCloseButton(bool);

public slots:

    /**
     * @brief onFinished 依赖下载完成的处理槽函数
     */
    void onFinished(int);

    /**
     * @brief on_readoutput 依赖下载过程中的输出函数
     */
    void on_readoutput();
private:
    QProcess *proc;                     //执行下载的进程指针
    int m_index = -1;                   //出现问题依赖的下标
    QStringList m_dependsList;          //需要安装的依赖列表
    bool bDependsStatusErr = false;     //依赖安装状态是否错误的标识
    QString m_brokenDepend;             //下载失败的依赖的名称
};

#endif // DEALDEPENDTHREAD_H
