#ifndef MULTIPLEINSTALLPAGE_H
#define MULTIPLEINSTALLPAGE_H

#include "infocontrolbutton.h"

#include <QWidget>

#include <QPushButton>
#include <QProgressBar>
#include <QTextEdit>
#include <QPropertyAnimation>

class PackagesListView;
class DebListModel;
class MultipleInstallPage : public QWidget
{
    Q_OBJECT

public:
    explicit MultipleInstallPage(DebListModel *model, QWidget *parent = 0);

private slots:
//    void onWorkerStarted();
    void onWorkerFinshed();
    void onOutputAvailable(const QString &output);
    void onProgressChanged(const int progress);

    void showInfo();
    void hideInfo();

private:
    DebListModel *m_debListModel;
    PackagesListView *m_appsView;
    QTextEdit *m_infoArea;
    InfoControlButton *m_infoControlButton;
    QProgressBar *m_installProgress;
    QPropertyAnimation *m_progressAnimation;
    QPushButton *m_installButton;
    QPushButton *m_acceptButton;
};

#endif // MULTIPLEINSTALLPAGE_H
