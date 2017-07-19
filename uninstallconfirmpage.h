#ifndef UNINSTALLCONFIRMPAGE_H
#define UNINSTALLCONFIRMPAGE_H

#include <QWidget>
#include <QPushButton>

class UninstallConfirmPage : public QWidget
{
    Q_OBJECT

public:
    explicit UninstallConfirmPage(QWidget *parent = 0);

signals:
    void accepted() const;
    void canceled() const;

private:
    QPushButton *m_cancelBtn;
    QPushButton *m_confirmBtn;
};

#endif // UNINSTALLCONFIRMPAGE_H
