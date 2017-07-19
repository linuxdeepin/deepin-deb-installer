#ifndef UNINSTALLCONFIRMPAGE_H
#define UNINSTALLCONFIRMPAGE_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>

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

private:
    QLabel *m_tips;
    QPushButton *m_cancelBtn;
    QPushButton *m_confirmBtn;
};

#endif // UNINSTALLCONFIRMPAGE_H
