#ifndef APTCONFIGMESSAGE_H
#define APTCONFIGMESSAGE_H

#include "installprocessinfoview.h"

#include <QWidget>
#include <DSuggestButton>
#include <DMainWindow>
#include <DLabel>
#include <DLineEdit>
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
    void clearTexts();

public slots:
    void dealInput();

signals:
    void AptConfigInputStr(QString);

protected:
    void paintEvent(QPaintEvent *event) override;

private:

    DIconButton *m_picon;
    DLineEdit *m_inputEdit;
    DSuggestButton *m_pushbutton;
    DLabel *m_pQuestionLabel;
    static AptConfigMessage *aptConfig;

    // QWidget interface
private:
    bool dealWrongAnswer(QString question, QString output);

private:
    void initUI();
    void initTitlebar();
    void initControl();

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QWidget *m_OptionWindow; //titlebar main menu
    QWidget *m_MinWindow;
    QWidget *m_closeWindow;
    QWidget *m_MaxWindow;
};

#endif // APTCONFIGMESSAGE_H
