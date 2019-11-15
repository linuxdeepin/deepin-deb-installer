#ifndef DROUNDBGFRAME_H
#define DROUNDBGFRAME_H

#include <DWidget>

DWIDGET_USE_NAMESPACE

class DRoundBgFrame : public DWidget
{
public:
    DRoundBgFrame(QWidget* parent = nullptr, int bgOffsetTop = 0, int bgOffsetBottom = 0);

    void paintEvent(QPaintEvent *) override;

public slots:
    void onShowHideTopBg(bool bShow);
    void onShowHideBottomBg(bool bShow);

private:
    int m_bgOffsetTop;
    int m_bgOffsetBottom;

    bool m_bFillTop;
    bool m_bFillBottom;
};

#endif // DROUNDBGFRAME_H
