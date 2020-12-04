/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
*
* Author:     cuizhen <cuizhen@uniontech.com>
* Maintainer:  cuizhen <cuizhen@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "installprocessinfoview.h"
#include "droundbgframe.h"
#include "utils/utils.h"
#include "ShowInstallInfoTextEdit.h"

#include <QVBoxLayout>
#include <QScroller>

#include <DApplicationHelper>

InstallProcessInfoView::InstallProcessInfoView(int w, int h, QWidget *parent)
    : QWidget(parent)
    , m_editor(new ShowInstallInfoTextEdit(this))       //修改为自写控件
{
    initUI(w, h);

    //fix bug: 44726 https://pms.uniontech.com/zentao/bug-view-44726.html
    //使用自写控件实现滑动，废弃
//    QScroller::grabGesture(m_editor, QScroller::TouchGesture);

    //数据更新后，直接跳转到最后一行
    connect(m_editor, &QTextEdit::textChanged, this, [ = ] {
        m_editor->moveCursor(QTextCursor::End);
    });
    //设置TextEdit和InfoView为无焦点
    //fix bug:https://pms.uniontech.com/zentao/bug-view-48235.html
    this->setFocusPolicy(Qt::NoFocus);
    m_editor->setFocusPolicy(Qt::NoFocus);
}

/**
 * @brief InstallProcessInfoView::initUI 初始化ProcessInfo的大小
 * @param w 控件的宽度
 * @param h 控件的高度
 * 此处在SP3之后修改，增加宽度高度参数
 * 为适应配置框的大小与安装器installProcessInfo的大小
 */
void InstallProcessInfoView::initUI(int w, int h)
{
    //设置控件背景色
    DRoundBgFrame *bgFrame = new DRoundBgFrame(this);
    bgFrame->setMinimumSize(w,h);

    //edit的布局
    QVBoxLayout *editLayout = new QVBoxLayout(this);
    editLayout->setSpacing(0);
    editLayout->setContentsMargins(5, 1, 0, 5);         //设置上下左右边距
    bgFrame->setLayout(editLayout);

    editLayout->addWidget(m_editor);

    // 设置控件的字体类型和字体大小
    QString textFont = Utils::loadFontFamilyByType(Utils::DefautFont);
    Utils::bindFontBySizeAndWeight(m_editor, textFont, 11, QFont::Light);

    // 设置字体颜色
    DPalette pa = DebApplicationHelper::instance()->palette(m_editor);
    m_colorType = DPalette::TextTips;
    pa.setColor(DPalette::Text, pa.color(m_colorType));
    m_editor->setPalette(pa);

    //设置只读，不允许对其进行修改
    m_editor->setReadOnly(true);
    m_editor->setFrameShape(QFrame::NoFrame);                           //设置frame类型为noframe
    m_editor->viewport()->setBackgroundRole(QPalette::Background);      //设置内容的背景色
    m_editor->viewport()->setAutoFillBackground(false);

    QTextCursor textCursor = m_editor->textCursor();                    //获取游标
    QTextBlockFormat textBlockFormat;
    //设置行高
    textBlockFormat.setLineHeight(20, QTextBlockFormat::FixedHeight);
    //设置行间距
    textBlockFormat.setBottomMargin(1);
    textCursor.setBlockFormat(textBlockFormat);
    m_editor->setTextCursor(textCursor);
}

/**
 * @brief InstallProcessInfoView::setTextFontSize 设置字体大小
 * @param fontSize      字体大小        PS:此参数无实际意义
 * @param fontWeight    字体大小
 */
void InstallProcessInfoView::setTextFontSize(int fontSize, int fontWeight)
{
    QString textFont = Utils::loadFontFamilyByType(Utils::DefautFont);
    Utils::bindFontBySizeAndWeight(m_editor, textFont, fontSize, fontWeight);
}

/**
 * @brief InstallProcessInfoView::setTextColor 设置字体颜色
 * @param ct    字体颜色
 */
void InstallProcessInfoView::setTextColor(DPalette::ColorType ct)
{
    m_colorType = ct;                                                   //保存传入的颜色类型
    DPalette pa = DebApplicationHelper::instance()->palette(m_editor);
    pa.setColor(DPalette::Text, pa.color(m_colorType));                 //editor设置传入的颜色类型
    m_editor->setPalette(pa);
}

/**
 * @brief InstallProcessInfoView::appendText 向editor添加要显示的数据
 * @param text  要显示的数据
 */
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

    // 获取当前的主题
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();

    if (themeType == DGuiApplicationHelper::LightType) {      //当前是浅色主题
        pa.setColor(DPalette::Text, QColor(96, 157, 200));
        m_editor->setPalette(pa);
    } else if (themeType == DGuiApplicationHelper::DarkType) {// 当前是深色主题
        pa.setColor(DPalette::Text, QColor(109, 124, 136));
        m_editor->setPalette(pa);
    } else {                                                  //默认使用浅色主题
        pa.setColor(DPalette::Text, QColor(96, 157, 200));
        m_editor->setPalette(pa);
    }
}

void InstallProcessInfoView::clearText()
{
    m_editor->clear();
}
