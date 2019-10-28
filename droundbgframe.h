#ifndef DROUNDBGFRAME_H
#define DROUNDBGFRAME_H

#include <DWidget>

DWIDGET_USE_NAMESPACE

class DRoundBgFrame : public DWidget
{
public:
    DRoundBgFrame(QWidget* parent = nullptr, bool hasFrameBorder = false, int bgOffsetTop = 0, int bgOffsetBottom = 0);

    void paintEvent(QPaintEvent *) override;

private:
    bool m_hasFrameBorder;
    int m_bgOffsetTop;
    int m_bgOffsetBottom;
};

#endif // DROUNDBGFRAME_H
