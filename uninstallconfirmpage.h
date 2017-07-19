#ifndef UNINSTALLCONFIRMPAGE_H
#define UNINSTALLCONFIRMPAGE_H

#include "infocontrolbutton.h"

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>

class UninstallConfirmPage : public QWidget
{
    Q_OBJECT

public:
    explicit UninstallConfirmPage(QWidget *parent = 0);

    void setPackage(const QString &name);
    void setRequiredList(const QStringList &requiredList);

signals:
    void accepted() const;
    void canceled() const;

private slots:
    void showDetail();
    void hideDetail();

private:
    QLabel *m_icon;
    QLabel *m_tips;
    InfoControlButton *m_infoControl;
    QTextEdit *m_dependsInfomation;
    QPushButton *m_cancelBtn;
    QPushButton *m_confirmBtn;
};
#endif // UNINSTALLCONFIRMPAGE_H
