#include "AptConfigMessage.h"
#include "utils.h"
#include <QGuiApplication>
#include <QDesktopWidget>
#include <DGuiApplicationHelper>
#include <DRecentManager>
#include <DTitlebar>
#include <DIconButton>
#include <QScreen>
#include <QMessageBox>
#include <QDebug>

AptConfigMessage *AptConfigMessage::aptConfig = nullptr;
DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE


AptConfigMessage::AptConfigMessage(QWidget *parent)
    : DMainWindow(parent)
{
    initTitlebar();
    initControl();
    initUI();
    connect(m_pushbutton, &QPushButton::clicked, this, &AptConfigMessage::dealInput);
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
    m_textEdit = new InstallProcessInfoView();
    m_textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_textEdit->setTextFontSize(14, QFont::Medium);
    m_textEdit->setFixedSize(360, 152);

    m_inputEdit = new QLineEdit();
    m_inputEdit->setFixedSize(360, 36);

    m_pQuestionLabel = new DLabel();
    m_pQuestionLabel->setMaximumWidth(360);

    m_pushbutton = new QPushButton("确定");
    m_pushbutton->setDefault(true);
    m_pushbutton->setFixedSize(360, 36);
}
void AptConfigMessage::initUI()
{
    setMinimumSize(380, 330);
    setTitlebarShadowEnabled(false);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->setSpacing(10);
    centralLayout->addStretch();

    centralLayout->addWidget(m_textEdit);
    centralLayout->addSpacing(15);

    centralLayout->addWidget(m_pQuestionLabel);
    centralLayout->addStretch();

    centralLayout->addWidget(m_inputEdit);
    centralLayout->addStretch();

    centralLayout->addWidget(m_pushbutton);
    centralLayout->addStretch();

    centralLayout->setContentsMargins(10, 0, 10, 0);

//    setLayout(centralLayout);
    QWidget *wrapWidget = new QWidget(this);
    wrapWidget->setLayout(centralLayout);
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
        m_textEdit->appendText(Utils::holdTextInRect(m_textEdit->font(), str, m_textEdit->width()));
        m_pQuestionLabel->setText(Utils::holdTextInRect(m_pQuestionLabel->font(), str, m_pQuestionLabel->width()));
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
        m_textEdit->appendText(Utils::holdTextInRect(m_textEdit->font(), strFilter, m_textEdit->width()));
        qDebug() << "strFilter" << strFilter;
        if (!strFilter.isEmpty())
            question = strFilter;
        if (num == -1 && text.size() > 0 && !text.contains("\n")) {
            break;
        }
    }
    qDebug() << "end after while";
    if (num == -1 && text.size() > 0 && !text.contains("\n")) {
        m_pQuestionLabel->setText(Utils::holdTextInRect(m_pQuestionLabel->font(), question, m_pQuestionLabel->width()));
    }
    m_textEdit->appendText("\n");
}

/**
 * @brief AptConfigMessage::dealInput
 * 向工作线程传递输入的数据
 */
void AptConfigMessage::dealInput()
{
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
                            //widget->setFocusPolicy(Qt::NoFocus);
                            if ("Dtk::Widget::DWindowOptionButton" == QString(widget->metaObject()->className())) {
                                widget->setVisible(false);
                            }
                            if ("Dtk::Widget::DWindowMinButton" == QString(widget->metaObject()->className())) {
                                widget->setVisible(false);
                            }
                            if ("Dtk::Widget::DWindowCloseButton" == QString(widget->metaObject()->className())) {
                                widget->setVisible(false);
                            }
                            if ("Dtk::Widget::DWindowMaxButton" == QString(widget->metaObject()->className())) {
                                widget->setVisible(false);
                            }
                        }
                    }
                }
            }
        }
    }
}
