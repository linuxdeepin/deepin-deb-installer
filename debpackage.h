#ifndef DEBPACKAGE_H
#define DEBPACKAGE_H

#include <QObject>

class DebPackage : public QObject
{
    Q_OBJECT

public:
    explicit DebPackage(QObject *parent = nullptr);
};

#endif // DEBPACKAGE_H
