#ifndef APTCONFIGMESSAGE_H
#define APTCONFIGMESSAGE_H

#include <QWidget>
#include <DPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

class AptConfigMessage : public QWidget
{
    Q_OBJECT

public:
    explicit AptConfigMessage(QWidget *parent = nullptr);
    QTextEdit *m_textEdit;

    static AptConfigMessage *getInstance()
    {
        if (aptConfig == nullptr) {
            aptConfig = new AptConfigMessage;
        }

        return aptConfig;
    }

public:
    void appendTextEdit(QString str);

public slots:
    void dealInput();

signals:
    void AptConfigInputStr(QString);

private:
    QLineEdit *m_inputEdit;
    QPushButton *m_pushbutton;
    static AptConfigMessage *aptConfig;
};

#endif // APTCONFIGMESSAGE_H
