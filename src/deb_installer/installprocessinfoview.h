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
    explicit InstallProcessInfoView(int w, int h, QWidget *parent = nullptr);
    virtual ~InstallProcessInfoView() override;

    void appendText(QString text);
    void setTextFontSize(int fontSize, int fontWeight);
    void setTextColor(DPalette::ColorType ct);
    void clearText();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initUI(int w, int h);

    QTextEdit *m_editor;
    DPalette::ColorType m_colorType;
};

#endif // INSTALLPROCESSINFOVIEW_H
