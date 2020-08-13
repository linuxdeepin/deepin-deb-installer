/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INFOCONTROLBUTTON_H
#define INFOCONTROLBUTTON_H

#include <QSettings>
#include <QVBoxLayout>

#include <DLabel>
#include <DCommandLinkButton>

DWIDGET_USE_NAMESPACE
#define THEME_DARK 2//"dark"
#define THEME_LIGHT 1//"light"

class InfoControlButton : public QWidget
{
    Q_OBJECT

public:
    explicit InfoControlButton(const QString &expandTips, const QString &shrinkTips, QWidget *parent = nullptr);

    void setShrinkTips(const QString text);
    void setExpandTips(const QString text);
signals:
    void expand();
    void shrink();

protected:
    void mouseReleaseEvent(QMouseEvent *);

private slots:
    void onMouseRelease();
    void themeChanged();

private:
    bool m_expand;
    QString m_expandTips;
    QString m_shrinkTips;

    DLabel *m_arrowIcon;
    QVBoxLayout *centralLayout;

public:
    //fix bug:33999 change DButton to DCommandLinkButton for Activity color
    DCommandLinkButton *m_tipsText;
};

#endif // INFOCONTROLBUTTON_H
