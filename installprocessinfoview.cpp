#include "installprocessinfoview.h"
#include "droundbgframe.h"
#include "utils.h"

#include <QVBoxLayout>

#include <DApplicationHelper>

InstallProcessInfoView::InstallProcessInfoView(QWidget *parent)
    : QWidget(parent)
    , m_editor(new QTextEdit)
{
    initUI();
}

void InstallProcessInfoView::initUI()
{
    DRoundBgFrame *bgFrame = new DRoundBgFrame(this);
    bgFrame->setFixedSize(440, 200);

    QVBoxLayout *editLayout = new QVBoxLayout;
    editLayout->setSpacing(0);
    editLayout->setContentsMargins(5, 1, 0, 5);
    bgFrame->setLayout(editLayout);

    editLayout->addWidget(m_editor);

    QString textFont = Utils::loadFontFamilyByType(Utils::DefautFont);
    Utils::bindFontBySizeAndWeight(m_editor, textFont, 11, QFont::Light);

    DPalette pa = DebApplicationHelper::instance()->palette(m_editor);
    m_colorType = DPalette::TextTips;
    pa.setColor(DPalette::Text, pa.color(m_colorType));
    m_editor->setPalette(pa);

    m_editor->setReadOnly(true);
    m_editor->setFrameShape(QFrame::NoFrame);
    m_editor->viewport()->setBackgroundRole(QPalette::Background);
    m_editor->viewport()->setAutoFillBackground(false);

    QTextCursor textCursor = m_editor->textCursor();
    QTextBlockFormat textBlockFormat;
    //设置行高
    textBlockFormat.setLineHeight(17, QTextBlockFormat::FixedHeight);
    //设置行间距
    textBlockFormat.setBottomMargin(1);
    textCursor.setBlockFormat(textBlockFormat);
    m_editor->setTextCursor(textCursor);
}

void InstallProcessInfoView::setTextFontSize(int fontSize, int fontWeight)
{
    QString textFont = Utils::loadFontFamilyByType(Utils::DefautFont);
    Utils::bindFontBySizeAndWeight(m_editor, textFont, fontSize, fontWeight);
}

void InstallProcessInfoView::setTextColor(DPalette::ColorType ct)
{
    m_colorType = ct;
    DPalette pa = DebApplicationHelper::instance()->palette(m_editor);
    pa.setColor(DPalette::Text, pa.color(m_colorType));
    m_editor->setPalette(pa);
}

void InstallProcessInfoView::appendText(QString text)
{
    m_editor->append(text);
}

InstallProcessInfoView::~InstallProcessInfoView()
{
}

void InstallProcessInfoView::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    DPalette pa = DebApplicationHelper::instance()->palette(this);
    pa.setColor(DPalette::Text, pa.color(m_colorType));
    m_editor->setPalette(pa);
}

