#ifndef DROUNDBGFRAME_H
#define DROUNDBGFRAME_H

#include <DLabel>

DWIDGET_USE_NAMESPACE

class DRoundBgFrame : public QWidget
{
public:
    DRoundBgFrame(QWidget* parent = nullptr, int bgOffsetTop = 0, int bgOffsetBottom = 0);

    void paintEvent(QPaintEvent *) override;

public slots:
    void onShowHideTopBg(bool bShow);
    void onShowHideBottomBg(bool bShow);

private:
    int m_bgOffsetTop = 0;
    int m_bgOffsetBottom = 0;
};

#endif // DROUNDBGFRAME_H
