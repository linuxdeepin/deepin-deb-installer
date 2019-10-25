#ifndef DROUNDBGFRAME_H
#define DROUNDBGFRAME_H

#include <DWidget>

DWIDGET_USE_NAMESPACE

class DRoundBgFrame : public DWidget
{
public:
    DRoundBgFrame(QWidget* parent = nullptr, bool hasFrameBorder=false);

    void paintEvent(QPaintEvent *) override;

private:
    bool m_hasFrameBorder;
};

#endif // DROUNDBGFRAME_H
