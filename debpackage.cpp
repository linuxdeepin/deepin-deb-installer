#include "debpackage.h"

#include <QProcess>
#include <QDebug>
#include <QRegularExpression>

DebPackage::DebPackage(const QString &filePath, QObject *parent)
    : QObject(parent),

      m_filePath(filePath)
{

    // get package info
    QProcess *proc = new QProcess;

    connect(proc, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this, &DebPackage::onProcFinsihed);

    proc->start("dpkg-deb", QStringList() << "-I" << m_filePath);
}

void DebPackage::onProcFinsihed()
{
    QProcess *proc = dynamic_cast<QProcess *>(sender());
    Q_ASSERT(proc);

    const QString output = proc->readAllStandardOutput();
    proc->deleteLater();

    qDebug().noquote() << output;

    const QRegularExpression package("^ Package: (.*)$", QRegularExpression::MultilineOption);
    m_package = package.match(output).captured(1).trimmed();

    const QRegularExpression version("^ Version: (.*)$", QRegularExpression::MultilineOption);
    m_version = version.match(output).captured(1).trimmed();

    const QRegularExpression description("^ Description: (.*)$", QRegularExpression::MultilineOption);
    m_description = description.match(output).captured(1).trimmed();

    qDebug() << m_package;
    qDebug() << m_version;
    qDebug() << m_description;
}
