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

/**
 * @brief The AptConfigMessage class
 * 配置包安装界面
 */
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
    /**
     * @brief appendTextEdit 获取安装进程返回的信息
     * @param str 安装进程的详细信息
     */
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

    /**
     * @brief initTabOrder 初始化tab切换焦点的顺序
     */
    void initTabOrder();
};

#endif // APTCONFIGMESSAGE_H
