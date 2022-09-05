// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QThread>
#include <iostream>
#include "installDebThread.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.process(app);
    const QStringList tParamList = parser.positionalArguments();

    InstallDebThread mThread;
    mThread.setParam(tParamList);
    mThread.run();
    mThread.wait();
    return mThread.m_resultFlag;
}
