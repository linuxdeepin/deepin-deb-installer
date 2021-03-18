/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      zhangkai <zhangkai@uniontech.com>
* Maintainer:  zhangkai <zhangkai@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../deb_installer/view/widgets/ShowInstallInfoTextEdit.h"
#include <stub.h>

#include <QGestureEvent>
#include <QMouseEvent>

#include <gtest/gtest.h>

Qt::MouseEventSource stud_source()
{
    return Qt::MouseEventSynthesizedByQt;
}

TEST(ShowInstallInfoTextEdit_TEST, ShowInstallInfoTextEdit_UT_slideGesture)
{
    ShowInstallInfoTextEdit *edit = new ShowInstallInfoTextEdit;
    edit->slideGesture(5);
}

TEST(ShowInstallInfoTextEdit_TEST, ShowInstallInfoTextEdit_UT_onSelectionArea)
{
    ShowInstallInfoTextEdit *edit = new ShowInstallInfoTextEdit;
    edit->m_gestureAction = ShowInstallInfoTextEdit::GA_tap;
    QTextCursor cursor = edit->textCursor();
    cursor.selectedText() = "//";
    edit->onSelectionArea();
}

TEST(FlashTween_TEST, FlashTween_UT_start)
{
    FlashTween flash;
    EXPECT_EQ(flash.sinusoidalEaseOut(1.0, 1.0, 1.0, 1.0), 1.0 * sin(1.0 / 1.0 * (3.14 / 2)) + 1.0);
    flash.start(0, 0, 1, 0, 0);
    flash.start(0, 0, 1, 1, 0);
    flash.m_timer->start(5);
    usleep(5000);
    flash.m_timer->stop();
}

int states = 1;

int state()
{
    return states;
}

TEST(ShowInstallInfoTextEdit_TEST, ShowInstallInfoTextEdit_UT_tapGestureTriggered)
{
    Stub stub;
    stub.set(ADDR(QGesture, state), state);
    ShowInstallInfoTextEdit *edit = new ShowInstallInfoTextEdit;
    while (states <= 4) {
        edit->tapGestureTriggered(nullptr);
        if (states != 2 && states != 4)
            edit->tapAndHoldGestureTriggered(nullptr);
        states++;
    }
}

TEST(ShowInstallInfoTextEdit_TEST, ShowInstallInfoTextEdit_UT_gestureEvent)
{
    ShowInstallInfoTextEdit *edit = new ShowInstallInfoTextEdit;
    QGesture *gesture = new QGesture();

    QGestureEvent gestureEvent(QList<QGesture *> {gesture, gesture});
    edit->gestureEvent(&gestureEvent);
    delete gesture;
    delete edit;
}

TEST(ShowInstallInfoTextEdit_TEST, ShowInstallInfoTextEdit_UT_mouseReleaseEvent)
{
    ShowInstallInfoTextEdit *edit = new ShowInstallInfoTextEdit;
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPoint(0, 0), QPoint(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    edit->mouseReleaseEvent(&releaseEvent);
    delete edit;
}

TEST(ShowInstallInfoTextEdit_TEST, ShowInstallInfoTextEdit_UT_mouseMoveEvent)
{
    ShowInstallInfoTextEdit *edit = new ShowInstallInfoTextEdit;
    QMouseEvent moveEvent(QEvent::MouseMove, QPoint(0, 0), QPoint(10, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    edit->mouseMoveEvent(&moveEvent);
}

TEST(ShowInstallInfoTextEdit_TEST, ShowInstallInfoTextEdit_UT_mouseMoveEvent_source)
{
    Stub stub;
    stub.set(ADDR(QMouseEvent, source), stud_source);
    ShowInstallInfoTextEdit *edit = new ShowInstallInfoTextEdit;
    edit->m_gestureAction = ShowInstallInfoTextEdit::GA_slide;
    QMouseEvent moveEvent(QEvent::MouseMove, QPoint(0, 0), QPoint(10, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    edit->mouseMoveEvent(&moveEvent);
}
