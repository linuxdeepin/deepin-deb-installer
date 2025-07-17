// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ShowInstallInfoTextEdit.h"
#include "utils/ddlog.h"

#include <QAbstractTextDocumentLayout>
#include <QTextDocumentFragment>
#include <QScrollBar>
#include <QEvent>
#include <QDebug>

ShowInstallInfoTextEdit::ShowInstallInfoTextEdit(QWidget *parent)
    : QTextEdit(parent)
{
    qCDebug(appLog) << "Initializing ShowInstallInfoTextEdit";
    setAttribute(Qt::WA_AcceptTouchEvents);  // 接受触控事件
    grabGesture(Qt::TapGesture);             // 获取触控点击事件
    grabGesture(Qt::TapAndHoldGesture);      // 获取触控点击长按事件

    // 滑动鼠标时选中的效果
    connect(this, &ShowInstallInfoTextEdit::selectionChanged, this, &ShowInstallInfoTextEdit::onSelectionArea);
    qCDebug(appLog) << "ShowInstallInfoTextEdit initialized";
}

void ShowInstallInfoTextEdit::onSelectionArea()
{
    qCDebug(appLog) << "Selection area changed";
    if (m_gestureAction != GA_null) {
        qCDebug(appLog) << "Gesture action is not null, clearing selection";
        QTextCursor cursor = textCursor();
        if (cursor.selectedText() != "") {
            cursor.clearSelection();
            setTextCursor(cursor);
        }
    }
}

bool ShowInstallInfoTextEdit::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture) {
        qCDebug(appLog) << "Gesture event received";
        gestureEvent(static_cast<QGestureEvent *>(event));
    }
    return QTextEdit::event(event);
}

bool ShowInstallInfoTextEdit::gestureEvent(QGestureEvent *event)
{
    // qCDebug(appLog) << "Processing gesture event";
    if (QGesture *tap = event->gesture(Qt::TapGesture)) {
        // qCDebug(appLog) << "Tap gesture detected";
        tapGestureTriggered(static_cast<QTapGesture *>(tap));
    }
    if (QGesture *tapAndHold = event->gesture(Qt::TapAndHoldGesture)) {
        // qCDebug(appLog) << "Tap and hold gesture detected";
        tapAndHoldGestureTriggered(static_cast<QTapAndHoldGesture *>(tapAndHold));
    }
    return true;
}

void ShowInstallInfoTextEdit::tapGestureTriggered(QTapGesture *tap)
{
    if (nullptr == tap)
        return;
    this->clearFocus();
    // 单指点击函数
    switch (tap->state()) {         // 根据点击的状态进行不同的操作
        case Qt::GestureStarted: {  // 开始点击，记录时间。时间不同 进行不同的操作
            qCDebug(appLog) << "Tap gesture started";
            m_gestureAction = GA_tap;
            m_tapBeginTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
            break;
        }
        case Qt::GestureUpdated: {
            qCDebug(appLog) << "Tap gesture updated - switching to slide mode";
            m_gestureAction = GA_slide;  // 触控滑动
            break;
        }
        case Qt::GestureCanceled: {
            // 根据时间长短区分轻触滑动
            qint64 timeSpace = QDateTime::currentDateTime().toMSecsSinceEpoch() - m_tapBeginTime;
            if (timeSpace < TAP_MOVE_DELAY || m_slideContinue) {  // 普通滑动
                qCDebug(appLog) << "Tap gesture canceled - continuing slide";
                m_slideContinue = false;
                m_gestureAction = GA_slide;
            } else {  // 选中滑动
                qCDebug(appLog) << "Tap gesture canceled - resetting";
                m_gestureAction = GA_null;
            }
            break;
        }
        case Qt::GestureFinished: {
            qCDebug(appLog) << "Tap gesture finished";
            m_gestureAction = GA_null;
            break;
        }
        default: {
            qCWarning(appLog) << "Unknown tap gesture state";
            Q_ASSERT(false);
            break;
        }
    }
}

void ShowInstallInfoTextEdit::tapAndHoldGestureTriggered(QTapAndHoldGesture *tapAndHold)
{
    qCDebug(appLog) << "Tap and hold gesture triggered";
    if (nullptr == tapAndHold) {
        qCWarning(appLog) << "Tap and hold gesture is null";
        return;
    }
    qCDebug(appLog) << "Tap and hold gesture triggered, state:" << tapAndHold->state();
    // 单指长按
    switch (tapAndHold->state()) {
        case Qt::GestureStarted:
            qCDebug(appLog) << "Tap and hold gesture started";
            m_gestureAction = GA_hold;
            break;
        case Qt::GestureUpdated:
            qCWarning(appLog) << "Unsupported tap and hold gesture state: Updated";
            Q_ASSERT(false);
            break;
        case Qt::GestureCanceled:
            qCWarning(appLog) << "Unsupported tap and hold gesture state: Canceled";
            Q_ASSERT(false);
            break;
        case Qt::GestureFinished:
            qCDebug(appLog) << "Tap and hold gesture finished";
            m_gestureAction = GA_null;
            break;
        default:
            qCWarning(appLog) << "Unknown tap and hold gesture state:" << tapAndHold->state();
            Q_ASSERT(false);
            break;
    }
}

void ShowInstallInfoTextEdit::slideGesture(qreal diff)
{
    static qreal delta = 0.0;
    int step = static_cast<int>(diff + delta);
    delta = diff + delta - step;
    qCDebug(appLog) << "Sliding with diff:" << diff << "step:" << step;
    verticalScrollBar()->setValue(verticalScrollBar()->value() + step);  // 移动滚动条
}

void ShowInstallInfoTextEdit::mouseReleaseEvent(QMouseEvent *e)
{
    // qCDebug(appLog) << "Mouse release event";
    change = 0.0;
    duration = 0.0;
    // add for single refers to the sliding
    if (e->type() == QEvent::MouseButtonRelease && e->source() == Qt::MouseEventSynthesizedByQt) {
        if (m_gestureAction == GA_slide) {
            qCDebug(appLog) << "Slide gesture finished, starting tween animation";
            // 滑动结束，开始惯性滑动
            tween.start(0, 0, change, duration, std::bind(&ShowInstallInfoTextEdit::slideGesture, this, std::placeholders::_1));
        }
        m_gestureAction = GA_null;
    }

    int i = m_end - m_start;
    if (Qt::MouseEventSynthesizedByQt == e->source() && (i > 10 && this->verticalScrollBar()->value() != 0)) {
        qCDebug(appLog) << "Synthesized mouse event, accepting and returning";
        e->accept();
        return;
    }
    return QTextEdit::mouseReleaseEvent(e);
}

void ShowInstallInfoTextEdit::mouseMoveEvent(QMouseEvent *e)
{
    // qCDebug(appLog) << "Mouse move event";
    if (Qt::MouseEventSynthesizedByQt == e->source()) {
        m_end = e->y();
    }

    // 单指滑动
    if (e->type() == QEvent::MouseMove && e->source() == Qt::MouseEventSynthesizedByQt) {
        const ulong diffTime = e->timestamp() - m_lastMouseTime;
        const int diffpos = e->pos().y() - m_lastMousepos;
        m_lastMouseTime = e->timestamp();
        m_lastMousepos = e->pos().y();

        if (m_gestureAction == GA_slide) {
            qCDebug(appLog) << "Slide gesture detected in mouse move event";
            QFont font = this->font();

            /*开根号时数值越大衰减比例越大*/
            qreal direction = diffpos < 0 ? 1.0 : -1.0;  // 确定滑动方向
            slideGesture(-direction * sqrt(abs(diffpos)) / font.pointSize());

            /*预算惯性滑动时间*/
            m_stepSpeed = static_cast<qreal>(diffpos) / static_cast<qreal>(diffTime + 0.000001);
            duration = sqrt(abs(m_stepSpeed)) * 1000;

            /*预算惯性滑动距离,4.0为调优数值*/
            m_stepSpeed /= sqrt(font.pointSize() * 4.0);
            change = m_stepSpeed * sqrt(abs(m_stepSpeed)) * 100;

            // 如果放到外面会屏蔽掉选中
            return;  // 此时屏蔽其他触控效果
        }
    }
    QTextEdit::mouseMoveEvent(e);
}

FlashTween::FlashTween()
{
    qCDebug(appLog) << "Initializing FlashTween";
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &FlashTween::__run);
}

void FlashTween::start(qreal t, qreal b, qreal c, qreal d, FunSlideInertial f)
{
    qCDebug(appLog) << "Starting FlashTween - change:" << c << "duration:" << d;
    if (c == 0.0 || d == 0.0) {
        qCDebug(appLog) << "FlashTween start ignored - zero change or duration";
        return;
    }
    qCDebug(appLog) << "Starting FlashTween - change:" << c << "duration:" << d;
    m_currentTime = t;
    m_beginValue = b;
    m_changeValue = c;
    m_durationTime = d;

    m_lastValue = 0;
    m_fSlideGesture = f;
    m_direction = m_changeValue > 0 ? 1 : -1;  // 确定滑动方向

    m_timer->stop();
    m_timer->start(CELL_TIME);
}

void FlashTween::__run()
{
    qCDebug(appLog) << "FlashTween run, current time:" << m_currentTime;
    qreal tempValue = m_lastValue;
    m_lastValue = FlashTween::sinusoidalEaseOut(m_currentTime, m_beginValue, abs(m_changeValue), m_durationTime);
    m_fSlideGesture(m_direction * (m_lastValue - tempValue));

    if (m_currentTime < m_durationTime) {
        m_currentTime += CELL_TIME;
    } else {
        qCDebug(appLog) << "FlashTween completed";
        m_timer->stop();
    }
}
