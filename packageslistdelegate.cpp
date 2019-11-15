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
#include "deblistmodel.h"
#include "utils.h"

#include <QPixmap>

#include <DSvgRenderer>
#include <DPalette>
#include <DStyleHelper>
#include <DApplicationHelper>

DWIDGET_USE_NAMESPACE

PackagesListDelegate::PackagesListDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
    , m_parentView(parent)
{
}

void PackagesListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->save();
        painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform, true);

        DPalette pa = DApplicationHelper::instance()->palette(m_parentView);

        const int content_x = 46;

        QPainterPath bgPath;
        bgPath.addRect(option.rect);

        if (option.state & QStyle::State_Selected) {
            DStyleHelper styleHelper;
            QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), DPalette::ToolTipText);
            fillColor.setAlphaF(0.2);
            painter->setBrush(QBrush(fillColor));
            painter->fillPath(bgPath, fillColor);
        }
        else {
            DStyleHelper styleHelper;
            QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), DPalette::Base);
            painter->setBrush(QBrush(fillColor));
            painter->fillPath(bgPath, fillColor);
        }

        int yOffset = 6;

        //绘制分割线
        QRect lineRect;
        lineRect.setX(content_x);
        int itemHeight = 48;
        if (0 == index.row()) {
            itemHeight -= 5;
            yOffset = 0;
        }
        lineRect.setY(option.rect.y()+itemHeight-1);
        lineRect.setWidth(option.rect.width()-content_x-10);
        lineRect.setHeight(1);

        DStyleHelper styleHelper;
        QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), DPalette::Shadow);
        painter->fillRect(lineRect, fillColor);

        QRect bg_rect = option.rect;

        QIcon icon = QIcon::fromTheme("application-x-deb");

        // draw package icon
        const int x = 6;
        int y = bg_rect.y()+yOffset+1;

        icon.paint(painter, x, y, 32, 32);

        // draw package name
        QRect name_rect = bg_rect;
        name_rect.setX(content_x);
        name_rect.setY(bg_rect.y()+yOffset);
        name_rect.setHeight(20);

        const QString pkg_name = index.data(DebListModel::PackageNameRole).toString();
        QString mediumFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansMedium);
        QString normalFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
        QString defaultFontFamily = Utils::loadFontFamilyByType(Utils::DefautFont);

        QFont pkg_name_font = Utils::loadFontBySizeAndWeight(mediumFontFamily, 14, QFont::Medium);
        QFontMetrics fontMetric(pkg_name_font);
        painter->setFont(pkg_name_font);
        const QString elided_pkg_name = fontMetric.elidedText(pkg_name, Qt::ElideRight, 150);
        const QRectF name_bounding_rect = painter->boundingRect(name_rect, elided_pkg_name, Qt::AlignLeft | Qt::AlignBottom);

        painter->setPen(styleHelper.getColor(static_cast<const QStyleOption *>(&option), DPalette::WindowText));
        painter->drawText(name_rect, elided_pkg_name, Qt::AlignLeft | Qt::AlignVCenter);

        // draw package version
        QRect version_rect = name_rect;
        const int version_x = static_cast<int>(name_bounding_rect.right()) + 10;
        const int version_y = version_rect.top();
        version_rect.setLeft(version_x);
        version_rect.setTop(version_y);
        version_rect.setRight(option.rect.right() - 85);
        QFont version_font = Utils::loadFontBySizeAndWeight(defaultFontFamily, 12, QFont::Light);
        QFontMetrics versionFontMetric(pkg_name_font);
        const QString version = index.data(DebListModel::PackageVersionRole).toString();
        const QString version_str = versionFontMetric.elidedText(version, Qt::ElideRight, bg_rect.width()-m_packageIcon.width()-240);
        painter->setPen(styleHelper.getColor(static_cast<const QStyleOption *>(&option), DPalette::BrightText));
        painter->setFont(version_font);
        painter->drawText(version_rect, version_str, Qt::AlignLeft | Qt::AlignVCenter);

        // install status
        const int operate_stat = index.data(DebListModel::PackageOperateStatusRole).toInt();
        if (operate_stat != DebListModel::Prepare) {
            QRect install_status_rect = option.rect;
            install_status_rect.setRight(option.rect.right() - 20);
            install_status_rect.setTop(version_y+yOffset - 10);

            DPalette pa = DebApplicationHelper::instance()->palette(m_parentView);;
            painter->setFont(Utils::loadFontBySizeAndWeight(mediumFontFamily, 11, QFont::Medium));
            switch (operate_stat) {
                case DebListModel::Operating:
                    painter->setPen(QPen(pa.color(DPalette::TextLively)));
                    painter->drawText(install_status_rect, tr("Installing"), Qt::AlignVCenter | Qt::AlignRight);
                    break;
                case DebListModel::Success:
                    painter->setPen(QPen(pa.color(DPalette::LightLively)));
                    painter->drawText(install_status_rect, tr("Installed"), Qt::AlignVCenter | Qt::AlignRight);
                    break;
                case DebListModel::Waiting:
                    painter->setPen(QPen(pa.color(DPalette::TextLively)));
                    painter->drawText(install_status_rect, tr("Waiting"), Qt::AlignVCenter | Qt::AlignRight);
                    break;
                default:
                    painter->setPen(QPen(pa.color(DPalette::TextWarning)));
                    painter->drawText(install_status_rect, tr("Failed"), Qt::AlignVCenter | Qt::AlignRight);
                    break;
            }
        }

        // draw package info
        QString info_str;
        QRect info_rect = option.rect;
        info_rect.setLeft(content_x);
        info_rect.setTop(name_rect.bottom()+2);

        const int install_stat = index.data(DebListModel::PackageVersionStatusRole).toInt();
//        QColor penColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::TextTips);
        pa = DebApplicationHelper::instance()->palette(m_parentView);
        QColor penColor = pa.color(DPalette::ToolTipText);
        if (operate_stat == DebListModel::Failed) {
            info_str = index.data(DebListModel::PackageFailReasonRole).toString();
            penColor = pa.color(DPalette::TextWarning);
        } else if (install_stat != DebListModel::NotInstalled) {
            if (install_stat == DebListModel::InstalledSameVersion) {
                info_str = tr("Same version installed");
            } else {
                info_str =
                    tr("Other version installed: %1").arg(index.data(DebListModel::PackageInstalledVersionRole).toString());
            }
        } else {
            info_str = index.data(DebListModel::PackageDescriptionRole).toString();
        }

        painter->setPen(QPen(penColor));
        painter->setFont(Utils::loadFontBySizeAndWeight(normalFontFamily, 12, QFont::ExtraLight));
        info_str = painter->fontMetrics().elidedText(info_str, Qt::ElideRight, 306);
        painter->drawText(info_rect, info_str, Qt::AlignLeft | Qt::AlignTop);

        painter->restore();
    }
    else
    {
        DStyledItemDelegate::paint(painter, option, index);
    }
}

QSize PackagesListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);

    QSize itemSize = index.data(Qt::SizeHintRole).toSize();
    if (0 == index.row()) {
        return QSize(itemSize.width(), itemSize.height()-5);
    }
    return itemSize;
}
