#ifndef DEBINFOLABEL_H
#define DEBINFOLABEL_H

#include "utils.h"

#include <DLabel>

DWIDGET_USE_NAMESPACE
class DebInfoLabel : public DLabel
{
    Q_OBJECT
public:
    explicit DebInfoLabel(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    void setCustomQPalette(QPalette::ColorRole colorRole);
    void setCustomDPalette(DPalette::ColorType colorType);
    void setCustomDPalette();

    void paintEvent(QPaintEvent *event) override;

private:
    QPalette::ColorRole m_colorRole;
    DPalette::ColorType m_colorType;

    bool m_bUserColorType;
    bool m_bMultiIns = false;
};

#endif // DEBINFOLABEL_H
