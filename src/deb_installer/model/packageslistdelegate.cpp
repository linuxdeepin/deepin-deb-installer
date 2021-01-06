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

#include "packageslistdelegate.h"
#include "model/deblistmodel.h"
#include "utils/utils.h"

#include <QPixmap>

#include <DSvgRenderer>
#include <DPalette>
#include <DStyleHelper>
#include <DApplicationHelper>
#include <DApplication>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE

//delegate 直接传入 model 解决多次创建model packagemanager导致崩溃的问题
PackagesListDelegate::PackagesListDelegate(DebListModel *m_model, QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
    , m_parentView(parent)
    , m_fileListModel(m_model)//从新new一个对象修改为获取传入的对象
{
    qApp->installEventFilter(this);                     //事件筛选
    QFontInfo fontinfo = m_parentView->fontInfo();      //获取字体
    int fontsize = fontinfo.pixelSize();
    if (fontsize >= 16) {                               //根据字体大小设置高度
        m_itemHeight = 52;
    } else {
        m_itemHeight = 48;
    }
}

/**
 * @brief PackagesListDelegate::refreshDebItemStatus
 * @param operate_stat         操作状态
 * @param install_status_rect  安装状态的位置
 * @param painter
 * @param isSelect              是否被选中
 * @param isEnable              是否可用
 */
void PackagesListDelegate::refreshDebItemStatus(const int operate_stat,
                                                QRect install_status_rect,
                                                QPainter *painter,
                                                bool isSelect, bool isEnable) const
{

    DPalette pa = DebApplicationHelper::instance()->palette(m_parentView);

    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();
    QPen forground;                         //前景色

    QColor color;                           //画笔颜色
    QString showText;                       //要显示的文本信息（安装状态）

    DPalette::ColorGroup cg;
    if (DApplication::activeWindow()) {     //当前处于激活状态
        cg = DPalette::Active;              //设置Palette为激活状态
    } else {
        cg = DPalette::Inactive;            //设置Palette为非激活状态
    }

    //根据操作的状态显示提示语
    switch (operate_stat) {
    case DebListModel::Operating:                                   //正在安装
        painter->setPen(QPen(pa.color(DPalette::TextLively)));
        showText = tr("Installing");
        break;
    case DebListModel::Success:                                     //安装成功
        painter->setPen(QPen(pa.color(DPalette::LightLively)));
        showText = tr("Installed");
        break;
    case DebListModel::Waiting:                                     //等待安装
        painter->setPen(QPen(pa.color(DPalette::TextLively)));
        showText = tr("Waiting");
        break;
    default:                                                        //安装失败
        painter->setPen(QPen(pa.color(DPalette::TextWarning)));
        showText = tr("Failed");
        break;
    }

    if (isSelect && isEnable) {                                     //当前被选中 未被选中使用默认颜色
        forground.setColor(palette.color(cg, DPalette::HighlightedText));
        painter->setPen(forground);
    }
    painter->drawText(install_status_rect, showText, Qt::AlignVCenter | Qt::AlignRight);    //在item上添加安装提示
}

void PackagesListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{

    if (index.isValid()) {//判断传入的index是否有效
        painter->save();
        painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform, true);

        const int content_x = 46;
        painter->setOpacity(1);

        QPainterPath bgPath;
        bgPath.addRect(option.rect);
        // 将当前Item的位置参数 发送给ListView,确定右键菜单的位置。
        emit sigIndexAndRect(option.rect, index.row());
        DApplicationHelper *dAppHelper = DApplicationHelper::instance();
        DPalette palette = dAppHelper->applicationPalette();
        QBrush background;
        QPen forground;
        DPalette::ColorGroup cg;
        if (!(option.state & DStyle::State_Enabled)) {                      //当前appListView not enable
            cg = DPalette::Disabled;
        } else {
            if (!DApplication::activeWindow()) {                            //当前窗口未被激活
                cg = DPalette::Inactive;
            } else {
                cg = DPalette::Active;                                      //当前窗口被激活
            }
        }
        if (option.features & QStyleOptionViewItem::Alternate) {
            background = palette.color(cg, DPalette::AlternateBase);
        } else {
            background = palette.color(cg, DPalette::Base);
        }

        //被选中时设置颜色高亮
        forground.setColor(palette.color(cg, DPalette::Text));
        if (option.state & DStyle::State_Enabled) {
            if (option.state & DStyle::State_Selected) {
                background = palette.color(cg, DPalette::Highlight);
                forground.setColor(palette.color(cg, DPalette::HighlightedText));
            }
        }
        painter->setPen(forground);
        painter->fillPath(bgPath, background);

        int yOffset = 6;

        //绘制分割线
        QRect lineRect;
        lineRect.setX(content_x);
        int itemHeight = m_itemHeight;
        lineRect.setY(option.rect.y() + itemHeight - 1);
        lineRect.setWidth(option.rect.width() - content_x - 10);
        lineRect.setHeight(1);

        // fix bug:33728
        DStyleHelper styleHelper;
        QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), DPalette::Shadow);
        painter->fillRect(lineRect, fillColor);


        QRect bg_rect = option.rect;

        QIcon icon = QIcon::fromTheme("application-x-deb");

        // draw package icon
        const int x = 6;
        int y = bg_rect.y() + yOffset + 1;

        icon.paint(painter, x, y, 32, 32);

        // draw package name
        QRect name_rect = bg_rect;
        name_rect.setX(content_x);
        name_rect.setY(bg_rect.y() + yOffset);

        const QString pkg_name = index.data(DebListModel::PackageNameRole).toString();
        QString mediumFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansMedium);
        QString normalFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
        QString defaultFontFamily = Utils::loadFontFamilyByType(Utils::DefautFont);

        QFont pkg_name_font = Utils::loadFontBySizeAndWeight(mediumFontFamily, 14, QFont::Medium);
        pkg_name_font.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T6));

        name_rect.setHeight(pkg_name_font.pixelSize() + 2);

        painter->setFont(pkg_name_font);
        QFontMetrics fontMetric(pkg_name_font);

        const QString elided_pkg_name = fontMetric.elidedText(pkg_name, Qt::ElideRight, 150);

        painter->setPen(/*styleHelper.getColor(static_cast<const QStyleOption *>(&option), DPalette::WindowText)*/forground);
        painter->drawText(name_rect, elided_pkg_name, Qt::AlignLeft | Qt::AlignVCenter);

        // draw package version
        QRect version_rect = name_rect;

        const int version_y = version_rect.top();
        version_rect.setLeft(200);
        version_rect.setTop(version_y - 1);
        version_rect.setRight(option.rect.right() - 85);
        QFontMetrics versionFontMetric(pkg_name_font);
        const QString version = index.data(DebListModel::PackageVersionRole).toString();
        const QString version_str = versionFontMetric.elidedText(version, Qt::ElideRight, 195);
        painter->setPen(/*styleHelper.getColor(static_cast<const QStyleOption *>(&option), DPalette::BrightText)*/forground);
        QFont version_font = Utils::loadFontBySizeAndWeight(defaultFontFamily, 12, QFont::Light);
        version_font.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T8));
        painter->setFont(version_font);
        painter->drawText(version_rect, version_str, Qt::AlignLeft | Qt::AlignVCenter);

        // install status
        const int operate_stat = index.data(DebListModel::PackageOperateStatusRole).toInt();        //获取包的状态
//        qDebug() << "index:::::" << index.row() << operate_stat;
        if (operate_stat != DebListModel::Prepare) {
            QRect install_status_rect = option.rect;
            install_status_rect.setRight(option.rect.right() - 20);
            install_status_rect.setTop(version_y + yOffset - 10);

            QFont stat_font = Utils::loadFontBySizeAndWeight(mediumFontFamily, 11, QFont::Medium);
            stat_font.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T9));
            painter->setFont(stat_font);
            //刷新添加包状态的提示
            refreshDebItemStatus(operate_stat, install_status_rect, painter, (option.state & DStyle::State_Selected), (option.state & DStyle::State_Enabled));
        }

        // draw package info
        QString info_str;

        QRect info_rect = option.rect;
        info_rect.setLeft(content_x);
        info_rect.setTop(name_rect.bottom() + 2);

        //获取包的版本
        const int install_stat = index.data(DebListModel::PackageVersionStatusRole).toInt();

        //获取包的依赖状态
        const int dependsStat = index.data(DebListModel::PackageDependsStatusRole).toInt();
        DPalette pa = DebApplicationHelper::instance()->palette(m_parentView);

        //未被选中，设置正常的颜色
        forground.setColor(palette.color(cg, DPalette::ToolTipText));

        if (operate_stat == DebListModel::Failed || (dependsStat == DebListModel::DependsBreak && install_stat == DebListModel::NotInstalled)
                || (dependsStat == DebListModel::DependsAuthCancel)) {
            info_str = index.data(DebListModel::PackageFailReasonRole).toString();
            forground.setColor(palette.color(cg, DPalette::TextWarning));       //安装失败或依赖错误
        } else if (install_stat != DebListModel::NotInstalled) {
            //获取安装版本
            if (install_stat == DebListModel::InstalledSameVersion) {       //安装了相同版本
                info_str = tr("Same version installed");
            } else if (install_stat == DebListModel::InstalledLaterVersion) {//安装了更新的版本
                info_str =
                    tr("Later version installed: %1").arg(index.data(DebListModel::PackageInstalledVersionRole).toString());
            } else {                                                        //安装了较早的版本
                info_str =
                    tr("Earlier version installed: %1").arg(index.data(DebListModel::PackageInstalledVersionRole).toString());
            }
            //fix bug: 43139
            forground.setColor(palette.color(cg, DPalette::TextTips));
        } else {//当前没有安装过
            //获取包的描述
            info_str = index.data(DebListModel::PackageDescriptionRole).toString();
            //fix bug: 43139
            forground.setColor(palette.color(cg, DPalette::TextTips));
        }
        //当前选中 设置高亮
        if (option.state & DStyle::State_Enabled) {
            if (option.state & DStyle::State_Selected) {
                forground.setColor(palette.color(cg, DPalette::HighlightedText));
            }
        }
        painter->setPen(forground);

        QFont info_font = Utils::loadFontBySizeAndWeight(normalFontFamily, 12, QFont::ExtraLight);
        info_font.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T8));
        painter->setFont(info_font);
        info_str = painter->fontMetrics().elidedText(info_str, Qt::ElideRight, 306);
        painter->drawText(info_rect, info_str, Qt::AlignLeft | Qt::AlignTop);       //将提示绘制到item上

        painter->restore();
    } else {
        DStyledItemDelegate::paint(painter, option, index);
    }
}

/**
 * @brief PackagesListDelegate::sizeHint 设置item的高度
 * @param option
 * @param index
 * @return
 */
QSize PackagesListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    Q_UNUSED(option);

    QSize itemSize = QSize(0, m_itemHeight); //设置Item的高度
    return itemSize;
}

/**
 * @brief PackagesListDelegate::eventFilter 获取字体改变的事件
 * @param watched
 * @param event
 * @return
 */
bool PackagesListDelegate::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::FontChange && watched == this) {
        QFontInfo fontinfo = m_parentView->fontInfo();
        emit fontinfo.pixelSize();
    }
    return QObject::eventFilter(watched, event);
}
