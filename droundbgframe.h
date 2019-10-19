#ifndef DROUNDBGFRAME_H
#define DROUNDBGFRAME_H

#include <DFrame>

DWIDGET_USE_NAMESPACE

class DRoundBgFrame : public DFrame
{
public:
    DRoundBgFrame(QWidget* parent = nullptr, bool hasFrameBorder=false);

    void paintEvent(QPaintEvent *) override;

private:
    bool m_hasFrameBorder;
};

#endif // DROUNDBGFRAME_H
