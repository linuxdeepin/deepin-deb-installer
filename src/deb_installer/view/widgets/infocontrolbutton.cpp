/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "infocontrolbutton.h"
#include "InfoCommandLinkButton.h"
#include "utils/utils.h"

#include <DStyleHelper>
#include <DApplicationHelper>

#include <QVBoxLayout>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QPixmap>
#include <QIcon>

InfoControlButton::InfoControlButton(const QString &expandTips, const QString &shrinkTips, QWidget *parent)
    : QWidget(parent)
    , m_expand(false)
    , m_expandTips(expandTips)
    , m_shrinkTips(shrinkTips)
    , m_arrowIcon(new DLabel(this))
    , m_tipsText(new InfoCommandLinkButton("", this))
{
    // 添加AccessibleName
    m_arrowIcon->setObjectName("arrowIcon");
    m_arrowIcon->setAccessibleName("arrowIcon");

    //只有DCommandLinkButton 需要焦点。
    this->setFocusPolicy(Qt::NoFocus);
    this->m_tipsText->setFocusPolicy(Qt::TabFocus);

    //设置自身自适应大小
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_arrowIcon->setAlignment(Qt::AlignCenter);

    //设置展开收缩的图标
    QIcon arrow_up = QIcon::fromTheme("di_arrow_up");
    m_arrowIcon->setPixmap(arrow_up.pixmap(QSize(25, 8)));

    //设置图标的高度
    m_arrowIcon->setFixedHeight(13);

    //默认设置展开。所以设置展开的提示语
    m_tipsText->setText(expandTips);
    QFontInfo fontinfo = m_tipsText->fontInfo();

    //设置提示语的高度

    m_tipsText->setMinimumHeight(28);

    //设置提示的字体颜色与字体大小
    QString normalFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    Utils::bindFontBySizeAndWeight(m_tipsText, normalFontFamily, 12, QFont::ExtraLight);

    //将控件设置布局
    centralLayout = new QVBoxLayout(this);
    centralLayout->setSpacing(0);

    // 设置上下左右边界
    centralLayout->setContentsMargins(0, 0, 0, 0);

    //添加控件到布局中
    centralLayout->addWidget(m_arrowIcon);
    centralLayout->addWidget(m_tipsText);

    // keep the tips in the middle
    centralLayout->setAlignment(m_tipsText, Qt::AlignCenter);

    setLayout(centralLayout);

    //适应主题变化
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this, &InfoControlButton::themeChanged);

    // add clicked connection fot expand or shrink
    connect(m_tipsText, &DCommandLinkButton::clicked, this, &InfoControlButton::onMouseRelease);


//#define SHOWBGCOLOR
#ifdef SHOWBGCOLOR
    m_tipsText->setStyleSheet("QLabel{background: cyan;}");
    m_arrowIcon->setStyleSheet("QLabel{background: red;}");
#endif
}

/**
 * @brief InfoControlButton::controlButton
 * @return 当前使用的CommandLinkButton
 */
QAbstractButton *InfoControlButton::controlButton()
{
    return m_tipsText;
}

/**
 * @brief InfoControlButton::mouseReleaseEvent 添加鼠标点击事件
 * @param e
 */
void InfoControlButton::mouseReleaseEvent(QMouseEvent *e)
{
    QWidget::mouseReleaseEvent(e);

    onMouseRelease();           //鼠标事件的响应
}

/**
 * @brief InfoControlButton::keyPressEvent 添加键盘事件响应
 * @param event
 */
void InfoControlButton::keyPressEvent(QKeyEvent *event)
{
    //添加回车键，空格键，换行键来触发展开或收缩
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Space || event->key() == Qt::Key_Enter) {
        if (this->hasFocus()) {
            m_tipsText->clicked();
        }
    }
}
/**
 * @brief InfoControlButton::onMouseRelease
 * 鼠标点击事件
 */
void InfoControlButton::onMouseRelease()
{
    if (m_expand) {                             //当前已经展开
        emit shrink();                          //发送收缩信号
    } else {
        emit expand();                          //发送展开信号
    }

    m_expand = !m_expand;                       //修改标志
    centralLayout->removeWidget(m_arrowIcon);
    centralLayout->removeWidget(m_tipsText);
    if (!m_expand) {                            //当前是收缩状态
        centralLayout->setSpacing(5);
        centralLayout->addWidget(m_arrowIcon);  //添加图片
        centralLayout->addWidget(m_tipsText);   //添加提示
        //fix bug: 33999 keep tips in the middle when install details hidden
        centralLayout->setAlignment(m_tipsText, Qt::AlignCenter);
        QIcon arrow_up = QIcon::fromTheme("di_arrow_up");       //设置图标为展开的图标
        m_arrowIcon->setPixmap(arrow_up.pixmap(QSize(25, 8)));
        m_tipsText->setText(m_expandTips);                      //设置提示为展开的提示
    } else {                                                    //当前是展开状态
        centralLayout->setSpacing(0);
        centralLayout->addWidget(m_tipsText);                   //添加提示
        //fix bug: 33999 keep tips in the middle when details show
        centralLayout->setAlignment(m_tipsText, Qt::AlignCenter);
        centralLayout->addWidget(m_arrowIcon);                  //添加图标
        QIcon arrow_up = QIcon::fromTheme("di_arrow_down");     //设置图标为收缩的图标
        m_arrowIcon->setPixmap(arrow_up.pixmap(QSize(25, 8)));
        m_tipsText->setText(m_shrinkTips);                      //设置提示为收缩的提示
    }
}

/**
 * @brief InfoControlButton::setExpandTips 接收外部展开的提示语
 * @param text  要修改的提示语
 */
void InfoControlButton::setExpandTips(const QString text)
{
    m_expandTips = text;                //保存提示语
    m_tipsText->setText(m_expandTips);  //设置提示语
}

/**
 * @brief InfoControlButton::setShrinkTips 接收外部收缩的提示语
 * @param text 要修改的提示语
 */
void InfoControlButton::setShrinkTips(const QString text)
{
    m_shrinkTips = text;                //保存提示语
    m_tipsText->setText(m_shrinkTips);  //设置提示语
}

/**
 * @brief InfoControlButton::themeChanged 主题变化的调整
 */
void InfoControlButton::themeChanged()
{
    if (m_expand) {                                                 //当前是展开状态
        QIcon arrow_down = QIcon::fromTheme("di_arrow_down");       //重新获取收缩的提示
        m_arrowIcon->setPixmap(arrow_down.pixmap(QSize(25, 8)));
    } else {                                                        //当前是收缩状态
        QIcon arrow_up = QIcon::fromTheme("di_arrow_up");           //重新设置展开的提示
        m_arrowIcon->setPixmap(arrow_up.pixmap(QSize(25, 8)));
    }
}
