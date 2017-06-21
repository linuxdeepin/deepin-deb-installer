#include "packageslistdelegate.h"
#include "deblistmodel.h"

#include <QPainter>

PackagesListDelegate::PackagesListDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{
    const QIcon icon = QIcon::fromTheme("application-vnd.debian.binary-package", QIcon::fromTheme("debian-swirl"));
    m_packageIcon = icon.pixmap(48, 48);
}

void PackagesListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
//    painter->fillRect(option.rect, Qt::gray);

    const int content_x = 65;

    // draw top border
    if (index.row())
    {
        const QPoint start(content_x, option.rect.top());
        const QPoint end(option.rect.right() - 10, option.rect.top());

        painter->setPen(QColor(220, 220, 220));
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->drawLine(start, end);
    }

    painter->setRenderHint(QPainter::Antialiasing);

    // draw package icon
    const int x = 10;
    const int y = option.rect.top() + (option.rect.height() - m_packageIcon.height()) / 2;
    painter->drawPixmap(x, y, m_packageIcon);

    // draw package name
    QRect name_rect = option.rect;
    name_rect.setLeft(content_x);
    name_rect.setHeight(name_rect.height() / 2);

    const QString name = index.data(DebListModel::PackageNameRole).toString();
    const QFont old_font = painter->font();
    QFont f = old_font;
    f.setWeight(500);
    painter->setFont(f);
    const QRectF name_bounding_rect = painter->boundingRect(name_rect, name, Qt::AlignLeft | Qt::AlignBottom);

    painter->setPen(Qt::black);
//    painter->fillRect(name_rect, Qt::red);
    painter->drawText(name_rect, name, Qt::AlignLeft | Qt::AlignBottom);
    painter->setFont(old_font);

    // draw package version
    const int version_x = name_bounding_rect.right() + 8;
    QRect version_rect = name_rect;
    version_rect.setLeft(version_x);
    painter->drawText(version_rect, index.data(DebListModel::PackageVersionRole).toString(), Qt::AlignLeft | Qt::AlignBottom);

    // draw package info
    QRect info_rect = option.rect;
    info_rect.setLeft(content_x);
    info_rect.setTop(name_rect.bottom() + 1 + 3);
//    painter->fillRect(info_rect, Qt::cyan);
    painter->setPen(QColor(80, 80, 80));
    painter->drawText(info_rect, index.data(DebListModel::PackageDescriptionRole).toString(), Qt::AlignLeft | Qt::AlignTop);
}

QSize PackagesListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);

    return index.data(Qt::SizeHintRole).toSize();
}
