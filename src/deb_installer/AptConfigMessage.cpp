#include "AptConfigMessage.h"
#include "utils.h"
#include <QGuiApplication>
#include <QDesktopWidget>
#include <DGuiApplicationHelper>
#include <DRecentManager>
#include <DTitlebar>
#include <QScreen>
#include <QMessageBox>
#include <QDebug>

AptConfigMessage *AptConfigMessage::aptConfig = nullptr;
DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE


AptConfigMessage::AptConfigMessage(QWidget *parent)
    : DMainWindow(parent)
{
    initControl();
    initUI();
    initTitlebar();
    initTabOrder();
    connect(m_pushbutton, &QPushButton::clicked, this, &AptConfigMessage::dealInput);
}

/**
 * @brief AptConfigMessage::initTabOrder 初始化tab切换焦点的顺序。
 */
void AptConfigMessage::initTabOrder()
{
    QWidget::setTabOrder(m_pushbutton, m_inputEdit->lineEdit());
    QWidget::setTabOrder(m_inputEdit->lineEdit(), m_pushbutton);
}
void AptConfigMessage::initTitlebar()
{
    DTitlebar *tb = titlebar();
    tb->setIcon(QIcon::fromTheme("deepin-deb-installer"));
    tb->setTitle("");
    tb->setVisible(false);
    tb->setMenuVisible(false);
    tb->setAutoFillBackground(false);
}

void AptConfigMessage::initControl()
{
    this->setFocusPolicy(Qt::NoFocus);

    m_textEdit = new InstallProcessInfoView(360, 196);
    m_textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_textEdit->setTextFontSize(12, QFont::Medium);
    m_textEdit->setFixedSize(360, 196);
    m_textEdit->setFocusPolicy(Qt::NoFocus);

    m_inputEdit = new DLineEdit();
    m_inputEdit->setFixedSize(220, 36);

    //设置输入框只接受两个数字，配置的选项在99个以内（1-99）
    //兼容有些包（mysql-community-server）配置时需要输入密码，取消对输入框的限制
    //    QRegExp regExp("[0-9]{1,2}");
    //    m_inputEdit->setValidator(new QRegExpValidator(regExp, this));

    m_pQuestionLabel = new DLabel(tr("Enter the number to configure: "));
    m_pQuestionLabel->setMaximumWidth(360);
    m_pQuestionLabel->setFocusPolicy(Qt::NoFocus);

    m_pushbutton = new DSuggestButton(tr("OK"));
    m_pushbutton->setDefault(true);
    m_pushbutton->setFixedSize(130, 36);

    connect(m_inputEdit, &DLineEdit::returnPressed, this, [ = ] {
        m_pushbutton->clicked();
    });
}
void AptConfigMessage::initUI()
{
    setFixedSize(380, 332);
    setTitlebarShadowEnabled(false);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addStretch(10);

    centralLayout->addWidget(m_textEdit);

    centralLayout->addWidget(m_pQuestionLabel);
    centralLayout->addStretch(10);

    QHBoxLayout *pInputLayout = new QHBoxLayout;
    pInputLayout->addWidget(m_inputEdit);
    m_inputEdit->setFocus();
    pInputLayout->addStretch(10);
    pInputLayout->addWidget(m_pushbutton);
    centralLayout->addLayout(pInputLayout);
    centralLayout->addStretch(10);
    centralLayout->setContentsMargins(10, 0, 10, 10);

    //    setLayout(centralLayout);
    QWidget *wrapWidget = new QWidget(this);
    wrapWidget->setLayout(centralLayout);
    wrapWidget->setFocusPolicy(Qt::NoFocus);
    setCentralWidget(wrapWidget);

    setWindowIcon(QIcon::fromTheme("deepin-deb-installer"));
    move(qApp->primaryScreen()->geometry().center() - geometry().center());
}

/**
 * @brief AptConfigMessage::dealWrongAnswer 判断当前输入的回答是否正确。
 * @param question m_QuestionLabel 中显示的问题
 * @param output 当前返回的提示
 * @return 当前提示是否与问题相同。
 * 当输入的回答错误时，程序将会返回再次返回问题，让用户回答。
 * 我们以此来判断当前输入的回答是否正确。
 * 输入回答错误时，不添加任何内容。
 */
bool AptConfigMessage::dealWrongAnswer(QString question, QString output)
{
    QString strText(output);
    strText.replace("\\n", "");
    strText = strText.trimmed();
    QString quesText(question);
    quesText.replace("-\n", "");
    qDebug() << "question" << quesText;
    qDebug() << "str" << strText;
    if (strText == quesText) {
        qDebug() << "return;";
        return true;
    }
    return false;
}

void AptConfigMessage::appendTextEdit(QString str)
{
    m_inputEdit->lineEdit()->setFocus();
    qDebug() << "str" << str << "str.size" << str.size();
    if (str.isEmpty() || str == "\\n")
        return;

    if (dealWrongAnswer(m_pQuestionLabel->text(), str))
        return;

    QString text;
    QString question("");
    text = str.replace("  ", "     ");

    text.remove(QChar('\"'), Qt::CaseInsensitive);
    int num = text.indexOf("\\n");
    if (num == -1) {
        m_textEdit->appendText(str);
        m_textEdit->appendText("\n");
        return;
    }
    int size = text.size();
    while (num != -1) {
        num = text.indexOf("\\n");

        QString strFilter;
        strFilter = text.mid(0, num);
        text = text.mid(num + 2, size - num - 3);

        if (strFilter[0] == '\t') strFilter.remove(0, 1);
        m_textEdit->appendText(strFilter);
        qDebug() << "strFilter" << strFilter;
        if (!strFilter.isEmpty())
            question = strFilter;
        if (num == -1 && text.size() > 0 && !text.contains("\n")) {
            break;
        }
    }
    qDebug() << "end after while";
}

/**
 * @brief AptConfigMessage::dealInput
 * 向工作线程传递输入的数据
 */
void AptConfigMessage::dealInput()
{
    qDebug() << "m_inputEdit" << m_inputEdit->text();
    if (m_inputEdit->text().isEmpty() || m_inputEdit->text() == "" || m_inputEdit->text() == "00") {
        m_inputEdit->clear();
        return;
    }
    QString str = m_inputEdit->text();
    str.remove(QChar('"'), Qt::CaseInsensitive);
    emit AptConfigInputStr(str);
    m_inputEdit->clear();
}

/**
 * @brief AptConfigMessage::paintEvent
 * @param event
 *
 * 去除界面上的 最小化 最大化 关闭 菜单 按钮
 */
void AptConfigMessage::paintEvent(QPaintEvent *event)
{
    DMainWindow::paintEvent(event);
    QLayout *layout = titlebar()->layout();
    for (int i = 0; i < layout->count(); ++i) {
        QWidget *widget = layout->itemAt(i)->widget();
        if (widget != nullptr && QString(widget->metaObject()->className()) ==  QString("QWidget")) {
            QLayout *widgetLayout = widget->layout();
            for (int j = 0; j < widgetLayout->count(); ++j) {
                QWidget *widget = widgetLayout->itemAt(j)->widget();
                if (widget != nullptr && QString(widget->metaObject()->className()) ==  QString("QWidget")) {
                    QLayout *wLayout = widget->layout();
                    for (int k = 0; k < wLayout->count(); ++k) {
                        QWidget *widget = wLayout->itemAt(k)->widget();
                        if (widget != nullptr && QString(widget->metaObject()->className()).contains("Button")) {
                            widget->setFocusPolicy(Qt::NoFocus);
                            // 隐藏标题栏所有的控件。
                            widget->setVisible(false);
                        }
                    }
                }
            }
        }
    }
}

void AptConfigMessage::clearTexts()
{
    m_textEdit->clearText();
    m_inputEdit->clear();
}

