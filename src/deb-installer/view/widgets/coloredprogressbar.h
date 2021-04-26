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

#pragma once
#include "dtkwidget_global.h"
#include "utils/utils.h"

#include<DObject>

#include <QProgressBar>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class ColoredProgressBarPrivate;

/*!
 * \class ColoredProgressBar
 * \brief ColoredProgressBar is the same as QProgressBar, except it can change its appearance depending on the value displayed.
 */
class ColoredProgressBar : public QProgressBar, public DObject
{
    Q_OBJECT
public:
    explicit ColoredProgressBar(QWidget *parent = nullptr);

    /*!
     * \brief ColoredProgressBar::addThreshold adds a new threshold value and specifies the brush to use once that value is reached.
     * If a threshold of the same value already exists, it will be overwritten.
     * \param threshold Minimum value for this brush to be used.
     * \param brush The brush to use when the currently displayed value is no less than \threshold and less than the next threshold value.
     */
    void addThreshold(int threshold, QBrush brush);

    /*!
     * \brief ColoredProgressBar::removeThreshold removes a threshold.
     * \param threshold The threshold value to remove.
     */
    void removeThreshold(int threshold);
    QList<int> thresholds() const;

protected:

    /*!
     * \brief ColoredProgressBar::threadsholds gets all threshold values.
     * \return A list of threshold values.
     */
    void paintEvent(QPaintEvent *) override;

private:
    DGuiApplicationHelper::ColorType m_themeType;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_d_ptr), ColoredProgressBar)

private slots:
    void themeChanged();
};
