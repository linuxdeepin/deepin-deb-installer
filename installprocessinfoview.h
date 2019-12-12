#ifndef INSTALLPROCESSINFOVIEW_H
#define INSTALLPROCESSINFOVIEW_H

#include <QPainter>
#include <QPaintEvent>
#include <QTextEdit>
#include <DPalette>

DGUI_USE_NAMESPACE

class InstallProcessInfoView : public QWidget
{
    Q_OBJECT
public:
    explicit InstallProcessInfoView(QWidget *parent = nullptr);
    virtual ~InstallProcessInfoView() override;

    void appendText(QString text);
    void setTextFontSize(int fontSize, int fontWeight);
    void setTextColor(DPalette::ColorType ct);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initUI();

    QTextEdit *m_editor;
    DPalette::ColorType m_colorType;
};

#endif // INSTALLPROCESSINFOVIEW_H
