#ifndef APTCONFIGMESSAGE_H
#define APTCONFIGMESSAGE_H

#include "installprocessinfoview.h"

#include <QWidget>
#include <DPushButton>
#include <DMainWindow>
#include <DLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <DIconButton>
DWIDGET_USE_NAMESPACE

class AptConfigMessage : public DMainWindow
{
    Q_OBJECT

public:
    explicit AptConfigMessage(QWidget *parent = nullptr);
    InstallProcessInfoView *m_textEdit;

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

protected:
    void paintEvent(QPaintEvent *event) override;

private:

    DIconButton *m_picon;
    QLineEdit *m_inputEdit;
    QPushButton *m_pushbutton;
    DLabel *m_pQuestionLabel;
    static AptConfigMessage *aptConfig;

    // QWidget interface

private:
    bool dealWrongAnswer(QString question, QString output);

private:
    void initUI();
    void initTitlebar();
    void initControl();

};

#endif // APTCONFIGMESSAGE_H
