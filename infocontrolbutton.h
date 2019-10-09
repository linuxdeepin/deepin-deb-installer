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

#include <DWidget>
#include <DLabel>
#include <QPen>
#include <QSettings>
#include <QVBoxLayout>
DWIDGET_USE_NAMESPACE
#define THEME_DARK 2//"dark"
#define THEME_LIGHT 1//"light"
class InfoControlButton : public DWidget
{
    Q_OBJECT

public:
    explicit InfoControlButton(const QString &expandTips, const QString &shrinkTips, DWidget *parent = 0);

    void setShowText(const QString text);
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
    DLabel *m_tipsText;

    QFont m_font;
    QVBoxLayout *centralLayout;
};

#endif // INFOCONTROLBUTTON_H
