#include "droundbgframe.h"
#include "utils.h"

#include <QPainter>
#include <QModelIndex>
#include <QDebug>

#include <DStyleHelper>
#include <DApplicationHelper>

DRoundBgFrame::DRoundBgFrame(QWidget* parent, int bgOffsetTop, int bgOffsetBottom)
    : QWidget(parent)
    , m_bgOffsetTop(bgOffsetTop)
    , m_bgOffsetBottom(bgOffsetBottom)
    , m_bFillTop(false)
    , m_bFillBottom(false)
{
}

void DRoundBgFrame::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QRectF originRect = this->rect();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    DPalette pa = DApplicationHelper::instance()->palette(this);

    if (m_bFillTop)
    {
        int topHeight = 16;

        QPainterPath pathTop;
        QRectF rect = originRect;
        rect.setX(0);
        rect.setY(m_bgOffsetTop);
        rect.setWidth(originRect.width());
        rect.setHeight(topHeight);

        pathTop.addRoundedRect(rect, 8, 8);
        QColor fillColor = pa.color(DPalette::ToolTipText);
        fillColor.setAlphaF(0.2);
        painter.fillPath(pathTop, QBrush(fillColor));

        QPainterPath pathBottom;
        rect = originRect;
        rect.setX(0);
        rect.setY(m_bgOffsetTop+topHeight);
        rect.setWidth(originRect.width());
        rect.setHeight(originRect.height()-topHeight-m_bgOffsetTop-m_bgOffsetBottom);

        pathBottom.addRoundedRect(rect, 8, 8);
        painter.fillPath(pathBottom, QBrush(pa.color(DPalette::Base)));

        return;
    }

    if (m_bFillBottom)
    {
        int topHeight = 16;

        QPainterPath pathTop;
        QRectF rect = originRect;
        rect.setX(0);
        rect.setY(m_bgOffsetTop);
        rect.setWidth(originRect.width());
        rect.setHeight(topHeight);

        pathTop.addRoundedRect(rect, 8, 8);
        QColor fillColor = pa.color(DPalette::Base);
        painter.fillPath(pathTop, QBrush(fillColor));

        QPainterPath pathBottom;
        rect = originRect;
        rect.setX(0);
        rect.setY(m_bgOffsetTop+topHeight);
        rect.setWidth(originRect.width());
        rect.setHeight(originRect.height()-topHeight-m_bgOffsetTop-m_bgOffsetBottom);

        pathBottom.addRoundedRect(rect, 8, 8);
        fillColor = pa.color(DPalette::ToolTipText);
        fillColor.setAlphaF(0.2);
        painter.fillPath(pathBottom, QBrush(fillColor));

        return;
    }

    QPainterPath path;
    QRectF rect = this->rect();
    rect.setX(0);
    rect.setY(m_bgOffsetTop);
    rect.setWidth(rect.width());
    rect.setHeight(rect.height()-m_bgOffsetBottom);

    path.addRoundedRect(rect, 8, 8);
    painter.fillPath(path, QBrush(pa.color(DPalette::Base)));
}

void DRoundBgFrame::onShowHideTopBg(bool bShow)
{
    if (bShow)
    {
        m_bFillTop = true;
    }
    else
    {
        m_bFillTop = false;
    }

    m_bFillBottom = false;
    update();
}

void DRoundBgFrame::onShowHideBottomBg(bool bShow)
{
    if (bShow)
    {
        m_bFillBottom = true;
    }
    else
    {
        m_bFillBottom = false;
    }
    m_bFillTop = false;

    update();
}
