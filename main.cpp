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

#include "debinstaller.h"

#include <DApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QTimer>
#include <DLog>

DWIDGET_USE_NAMESPACE
#ifdef DUTIL_USE_NAMESPACE
DUTIL_USE_NAMESPACE
#else
DCORE_USE_NAMESPACE
#endif

int main(int argc, char *argv[])
{
    DApplication::loadDXcbPlugin();

    DApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("deepin-deb-installer");
    app.setApplicationVersion("1.1");
    app.setApplicationAcknowledgementPage("https://www.deepin.org/acknowledgments/deepin-package-manager/");
    app.setProductIcon(QIcon(":/images/icon.svg"));
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setAttribute(Qt::AA_EnableHighDpiScaling);
//    app.loadTranslator(QList<QLocale>() << QLocale("zh_CN"));
    app.loadTranslator();
    app.setProductName(QApplication::translate("main", "Deepin Package Manager"));
    app.setApplicationDescription(QApplication::translate("main", "Deepin Package Manager is an application used to help users install and remove local software, supports bulk install."));
    app.setTheme("light");

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    // command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin deb package manager.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("filename", "Deb package path.", "file [file..]");

    parser.process(app);

    const QStringList file_list = parser.positionalArguments();
    qDebug() << file_list;

    DebInstaller w;
    w.show();

    // select files from args
    if (!file_list.isEmpty())
        QMetaObject::invokeMethod(&w, "onPackagesSelected", Qt::QueuedConnection, Q_ARG(QStringList, file_list));

    return app.exec();
}
