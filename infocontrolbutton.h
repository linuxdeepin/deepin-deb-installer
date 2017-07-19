#ifndef INFOCONTROLBUTTON_H
#define INFOCONTROLBUTTON_H

#include <QWidget>
#include <QLabel>

class InfoControlButton : public QWidget
{
    Q_OBJECT

public:
    explicit InfoControlButton(const QString &expandTips, const QString &shrinkTips, QWidget *parent = 0);

signals:
    void expand();
    void shrink();

protected:
    void mouseReleaseEvent(QMouseEvent *);

private slots:
    void onMouseRelease();

private:
    bool m_expand;
    QString m_expandTips;
    QString m_shrinkTips;

    QLabel *m_arrowIcon;
    QLabel *m_tipsText;
};

#endif // INFOCONTROLBUTTON_H
