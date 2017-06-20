#ifndef SINGLEINSTALLPAGE_H
#define SINGLEINSTALLPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

#include <DebFile>

class SingleInstallPage : public QWidget
{
    Q_OBJECT

public:
    explicit SingleInstallPage(QWidget *parent = 0);

    void setPackage(QApt::DebFile *package);

private:
    QLabel *m_packageIcon;
    QLabel *m_packageName;
    QLabel *m_packageVersion;
    QLabel *m_packageDescription;
    QPushButton *m_installButton;
};

#endif // SINGLEINSTALLPAGE_H
