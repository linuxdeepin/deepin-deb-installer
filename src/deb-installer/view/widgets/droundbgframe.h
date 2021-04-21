/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
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

#ifndef DROUNDBGFRAME_H
#define DROUNDBGFRAME_H

#include <DLabel>

DWIDGET_USE_NAMESPACE

class DRoundBgFrame : public QWidget
{
public:
    DRoundBgFrame(QWidget *parent = nullptr, int bgOffsetTop = 0, int bgOffsetBottom = 0);

    void paintEvent(QPaintEvent *) override;

public slots:
    void slotShowHideTopBg(bool bShow);
    void slotShowHideBottomBg(bool bShow);

private:
    int m_bgOffsetTop       = 0;
    int m_bgOffsetBottom    = 0;
};

#endif // DROUNDBGFRAME_H
