#ifndef DEBINFOLABEL_H
#define DEBINFOLABEL_H

#include <DLabel>

DWIDGET_USE_NAMESPACE

/**
 * @brief The DebInfoLabel class
 *  singleInstallPage 中 package name、 package version 与 tipsLabel 的控件
 *  基于DLabel 对不同情况下的Dlabel进行界面上的风格样式修改，保证界面上风格的统一
 *
 */
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
