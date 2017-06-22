#ifndef SINGLEINSTALLPAGE_H
#define SINGLEINSTALLPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

#include <QApt/DebFile>

class DebListModel;
class SingleInstallPage : public QWidget
{
    Q_OBJECT

public:
    explicit SingleInstallPage(DebListModel *model, QWidget *parent = 0);

private slots:
    void install();

private:
    void setPackageInfo();

private:
    DebListModel *m_packagesModel;
    QLabel *m_packageIcon;
    QLabel *m_packageName;
    QLabel *m_packageVersion;
    QLabel *m_packageDescription;
    QPushButton *m_installButton;
};

#endif // SINGLEINSTALLPAGE_H
