/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
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
#include "installDebThread.h"
#include <QDebug>

InstallDebThread::InstallDebThread()
{
    m_proc = new QProcess;
    connect(m_proc, SIGNAL(finished(int)), this, SLOT(onFinished(int)));
    connect(m_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(on_readoutput()));
}
InstallDebThread::~InstallDebThread()
{
    if (m_proc)
        delete m_proc;
}

void InstallDebThread::setParam(QStringList tParam)
{
    m_listParam = tParam;
}

void InstallDebThread::getDescription(QString debPath)
{
    QString str = "dpkg -e " + debPath + " " + TEMPLATE_DIR;
    system(str.toUtf8());

    QFile file;
    file.setFileName(TEMPLATE_PATH);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString tmpData;
        while (!file.atEnd()) {
            tmpData = file.readLine().data();
            if (tmpData.size() > 13) {
                if (tmpData.contains("Description: ")) {
                    QString str = tmpData.mid(13, tmpData.size() - 13);
                    str.remove(QChar('\n'), Qt::CaseInsensitive);
                    m_listDescribeData << str;
                }
            }
        }

        file.close();
    }
}

void InstallDebThread::on_readoutput()
{
    QString tmp = m_proc->readAllStandardOutput().data();
    qDebug() << tmp;

    foreach (QString str, m_listDescribeData) {
        if (tmp.contains(str)) {
            char c_input[20];
            while (fgets(c_input, 10, stdin)) {
                QString str = c_input;
                str.remove(QChar('\\'), Qt::CaseInsensitive);
                str.remove(QChar('"'), Qt::CaseInsensitive);

                m_proc->write(str.toLatin1().data());

                m_proc->waitForFinished(1500);

                break;
            }
        }
    }
}

void InstallDebThread::onFinished(int num)
{
    m_resultFlag = num;
}

void InstallDebThread::run()
{
    if (m_listParam.size() > 0) {
        if (m_listParam[0] == "InstallDeepinWine") {
            qDebug() << "StartInstallDeepinwine";
            QStringList depends;

            for (int i = 1; i < m_listParam.size(); i++) {
                depends << m_listParam[i];
            }

            system("echo \"libpam-runtime libpam-runtime/override boolean false\" | debconf-set-selections");
            system("echo 'libc6 libraries/restart-without-asking boolean true' | sudo debconf-set-selections\n");
            m_proc->start("sudo", QStringList() << "apt-get"
                          << "install"
                          << depends
                          << "deepin-wine-helper"
                          << "--fix-missing"
                          << "-y");
            m_proc->waitForFinished(-1);
            m_proc->close();
        } else if (m_listParam[0] == "InstallConfig") {
            if (m_listParam.size() <= 1)
                return;
            const QFileInfo info(m_listParam[1]);
            const QFile debFile(m_listParam[1]);
            QString debPath = m_listParam[1];
            if (debPath.contains(" ")
                    || debPath.contains("&")
                    || debPath.contains(";")
                    || debPath.contains("|")) {
                debPath = SymbolicLink(debPath, "installPackage");
            }

            if (debFile.exists() && info.isFile() && info.suffix().toLower() == "deb") {        //大小写不敏感的判断是否为deb后缀
                qDebug() << "StartInstallAptConfig";

                getDescription(debPath);

                //m_proc->start("sudo", QStringList() << "-S" <<  "dpkg-preconfigure" << "-f" << "Teletype" << m_listParam[1]);
                m_proc->start("sudo", QStringList() << "-S" <<  "dpkg" << "-i" << debPath);
                m_proc->waitForFinished(-1);

                QDir filePath(TEMPLATE_DIR);
                if (filePath.exists())
                    filePath.removeRecursively();
                m_proc->close();
            }

        }
    }
}


/**
 * @brief PackagesManager::SymbolicLink 创建软连接
 * @param previousName 原始路径
 * @param packageName 软件包的包名
 * @return 软链接的路径
 */
QString InstallDebThread::SymbolicLink(QString previousName, QString packageName)
{
    if (!mkTempDir()) {
        qWarning() << "InstallDebThread:" << "Failed to create temporary folder";
        return previousName;
    }
    return link(previousName, packageName);
}

/**
 * @brief PackagesManager::mkTempDir 创建软链接存放的临时目录
 * @return 创建目录的结果
 */
bool InstallDebThread::mkTempDir()
{
    QDir tempPath(m_tempLinkDir);
    if (!tempPath.exists()) {
        return tempPath.mkdir(m_tempLinkDir);
    } else {
        return true;
    }
}

/**
 * @brief PackagesManager::rmTempDir 删除存放软链接的临时目录
 * @return 删除临时目录的结果
 */
bool InstallDebThread::rmTempDir()
{
    QDir tempPath(m_tempLinkDir);
    if (tempPath.exists()) {
        return tempPath.removeRecursively();
    } else {
        return true;
    }
}

/**
 * @brief PackagesManager::link 创建软链接
 * @param linkPath              原文件的路径
 * @param packageName           包的packageName
 * @return                      软链接之后的路径
 */
QString InstallDebThread::link(QString linkPath, QString packageName)
{
    qDebug() << "InstallDebThread: Create soft link for" << packageName;
    QFile linkDeb(linkPath);

    //创建软链接时，如果当前临时目录中存在同名文件，即同一个名字的应用，考虑到版本可能有变化，将后续添加进入的包重命名为{packageName}_1
    //删除后再次添加会在临时文件的后面添加_1,此问题不影响安装。如果有问题，后续再行修改。
    int count = 1;
    QString tempName = packageName;
    while (true) {
        QFile tempLinkPath(m_tempLinkDir + tempName);
        if (tempLinkPath.exists()) {
            tempName = packageName + "_" + QString::number(count);
            qWarning() << "InstallDebThread:" << "A file with the same name exists in the current temporary directory,"
                       "and the current file name is changed to"
                       << tempName;
            count++;
        } else {
            break;
        }
    }
    if (linkDeb.link(linkPath, m_tempLinkDir + tempName))
        return m_tempLinkDir + tempName;
    else {
        qWarning() << "InstallDebThread:" << "Failed to create Symbolick link error.";
        return linkPath;
    }
}
