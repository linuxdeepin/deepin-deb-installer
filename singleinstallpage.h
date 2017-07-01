#ifndef SINGLEINSTALLPAGE_H
#define SINGLEINSTALLPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QTextEdit>

#include <QApt/DebFile>

#include <dlinkbutton.h>

class DebListModel;
class SingleInstallPage : public QWidget
{
    Q_OBJECT

public:
    explicit SingleInstallPage(DebListModel *model, QWidget *parent = 0);

private:
    void setPackageInfo();

private slots:
    void install();
    void uninstallCurrentPackage();

    void showInfomation();
    void hideInfomation();

    void onWorkerStarted();
    void onWorkerFinished();
    void onWorkerProgressChanged(const int progress);

private:
    DebListModel *m_packagesModel;
    QWidget *m_itemInfoWidget;
    QLabel *m_packageIcon;
    QLabel *m_packageName;
    QLabel *m_packageVersion;
    QLabel *m_packageDescription;
    QLabel *m_tipsLabel;
    QProgressBar *m_progress;
    QTextEdit *m_workerInfomation;
    Dtk::Widget::DLinkButton *m_showInfoButton;
    Dtk::Widget::DLinkButton *m_hideInfoButton;
    QPushButton *m_installButton;
    QPushButton *m_uninstallButton;
    QPushButton *m_reinstallButton;
    QPushButton *m_confirmButton;
};

#endif // SINGLEINSTALLPAGE_H
