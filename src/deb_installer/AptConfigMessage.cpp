#include "AptConfigMessage.h"
#include <QGuiApplication>
#include <QDesktopWidget>
#include <DGuiApplicationHelper>
#include <DRecentManager>
#include <QScreen>
#include <QMessageBox>
#include <QDebug>

AptConfigMessage *AptConfigMessage::aptConfig = nullptr;

AptConfigMessage::AptConfigMessage(QWidget *parent)
    : QWidget(parent)
{
    m_textEdit = new QTextEdit;
    m_inputEdit = new QLineEdit;
    m_pushbutton = new QPushButton("input");
    setFixedSize(480, 380);

    connect(m_pushbutton, &QPushButton::clicked, this, &AptConfigMessage::dealInput);

    m_textEdit->setFixedHeight(300);
    m_textEdit->setReadOnly(true);
    m_pushbutton->setFixedSize(120, 36);

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addSpacing(5);
    centralLayout->addWidget(m_textEdit);

    centralLayout->addStretch();

    centralLayout->addWidget(m_inputEdit);

    centralLayout->addStretch();
    centralLayout->addWidget(m_pushbutton);
    centralLayout->addStretch();

    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(centralLayout);
    move(qApp->primaryScreen()->geometry().center() - geometry().center());
}

void AptConfigMessage::appendTextEdit(QString str)
{
    str.remove(QChar('\"'), Qt::CaseInsensitive);
    int num = str.indexOf("\\n");
    if (num == -1) {
        m_textEdit->append(str);
        return;
    }
    int size = str.size();
    while (num != -1) {
        num = str.indexOf("\\n");

        QString strFilter;
        strFilter = str.mid(0, num);
        str = str.mid(num + 2, size - num - 3);

        m_textEdit->append(strFilter);
        if (num == -1 && str.size() > 0)
            return;
    }
}

void AptConfigMessage::dealInput()
{
    QString str = m_inputEdit->text();
    str.remove(QChar('"'), Qt::CaseInsensitive);
    emit AptConfigInputStr(str);
}
