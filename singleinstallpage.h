#ifndef SINGLEINSTALLPAGE_H
#define SINGLEINSTALLPAGE_H

#include "infocontrolbutton.h"

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

    void showInfo();
    void onOutputAvailable(const QString &output);
    void onWorkerFinished();
    void onWorkerProgressChanged(const int progress);

private:
    bool m_workerStarted;
    DebListModel *m_packagesModel;
    QWidget *m_itemInfoWidget;
    QLabel *m_packageIcon;
    QLabel *m_packageName;
    QLabel *m_packageVersion;
    QLabel *m_packageDescription;
    QLabel *m_tipsLabel;
    QProgressBar *m_progress;
    QTextEdit *m_workerInfomation;
    InfoControlButton *m_infoControlButton;
    QPushButton *m_installButton;
    QPushButton *m_uninstallButton;
    QPushButton *m_reinstallButton;
    QPushButton *m_confirmButton;
};

#endif // SINGLEINSTALLPAGE_H
