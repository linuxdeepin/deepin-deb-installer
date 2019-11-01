#ifndef DEBINFOLABEL_H
#define DEBINFOLABEL_H

#include "utils.h"

#include <DLabel>

DWIDGET_USE_NAMESPACE

class DebInfoLabel : public DLabel
{
    Q_OBJECT
public:
    explicit DebInfoLabel(QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags());

    void setCustomPalette(QPalette::ColorRole colorRole);

    void paintEvent(QPaintEvent *event) override;

private:
    QPalette::ColorRole m_colorRole;
};

#endif // DEBINFOLABEL_H
