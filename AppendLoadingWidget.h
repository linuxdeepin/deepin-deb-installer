#ifndef APPENDLOADINGWIDGET_H
#define APPENDLOADINGWIDGET_H

#include <QWidget>
#include <DSpinner>

DWIDGET_USE_NAMESPACE

class AppendLoadingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AppendLoadingWidget(QWidget *parent = nullptr);

signals:

public slots:

private:
    DSpinner *m_pSpinner {nullptr};
};

#endif // APPENDLOADINGWIDGET_H
