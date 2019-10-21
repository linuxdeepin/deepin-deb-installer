#include "installprocessinfoview.h"
#include "droundbgframe.h"

#include <QVBoxLayout>

InstallProcessInfoView::InstallProcessInfoView(QWidget *parent)
    : DFrame(parent)
    , m_editor(new DTextEdit)
{
    initUI();
}

void InstallProcessInfoView::initUI()
{
    QColor infomationTextColor = QColor("#609DC8");

    DRoundBgFrame *bgFrame = new DRoundBgFrame(this);
    bgFrame->setFixedSize(440, 200);

    QVBoxLayout *editLayout = new QVBoxLayout;
    editLayout->setSpacing(0);
    editLayout->setContentsMargins(0, 0, 0, 0);
    bgFrame->setLayout(editLayout);

    editLayout->addWidget(m_editor);

    m_editor->setTextColor(infomationTextColor);
    m_editor->setReadOnly(true);
    m_editor->setFrameShape(QFrame::NoFrame);
    m_editor->viewport()->setBackgroundRole(QPalette::Background);
    m_editor->viewport()->setAutoFillBackground(false);
}

void InstallProcessInfoView::appendText(QString text)
{
    m_editor->append(text);
}

InstallProcessInfoView::~InstallProcessInfoView()
{
}

