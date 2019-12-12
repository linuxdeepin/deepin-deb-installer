#ifndef QUITCONFIRMDIALOG_H
#define QUITCONFIRMDIALOG_H

#include <DDialog>
#include <DLabel>
#include <DPushButton>
#include <QWidget>

DWIDGET_USE_NAMESPACE

class QuitConfirmDialog : public DDialog
{
    Q_OBJECT
public:
    explicit QuitConfirmDialog(QWidget *parent = nullptr);
    explicit QuitConfirmDialog(const QString &title, const QString& message, QWidget *parent = nullptr);

    void setTipMessage(const QString &msg);

private:
    void initUI();

//    DLabel *m_icon;
    DLabel *m_tipLabel;
    QWidget *m_infoWrapperWidget;
    DPushButton *m_cancelBtn;
    DPushButton *m_confirmBtn;
};

#endif // QUITCONFIRMDIALOG_H
