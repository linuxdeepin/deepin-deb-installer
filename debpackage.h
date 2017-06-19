#ifndef DEBPACKAGE_H
#define DEBPACKAGE_H

#include <QObject>

class DebPackage : public QObject
{
    Q_OBJECT

public:
    explicit DebPackage(const QString &filePath, QObject *parent = nullptr);

    const QString path() const { return m_filePath; }
    const QString name() const { return m_package; }
    const QString version() const { return m_version; }
    const QString description() const { return m_description; }

private slots:
    void onProcFinsihed();

private:
    const QString m_filePath;

    QString m_package;
    QString m_version;
    QString m_description;
};

#endif // DEBPACKAGE_H
