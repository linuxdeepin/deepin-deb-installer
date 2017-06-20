#ifndef SINGLEINSTALLPAGE_H
#define SINGLEINSTALLPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

#include <QApt/DebFile>
#include <QApt/Backend>

class SingleInstallPage : public QWidget
{
    Q_OBJECT

public:
    explicit SingleInstallPage(QWidget *parent = 0);

    void setPackage(QApt::DebFile *package);

private slots:
    void install();
    void onTransactionStatusChanged(const QApt::TransactionStatus status);

private:
    QLabel *m_packageIcon;
    QLabel *m_packageName;
    QLabel *m_packageVersion;
    QLabel *m_packageDescription;
    QPushButton *m_installButton;

    QApt::DebFile *m_debFile;
    QApt::Backend *m_aptBackend;
};

#endif // SINGLEINSTALLPAGE_H
