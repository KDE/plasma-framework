/*
    Copyright 2011 Marco Martin <notmart@gmail.com>
    Copyright 2013 Sebastian Kügler <sebas@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "mouseeventlistener.h"

#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QQuickWindow>

#include <QDebug>

static const int PressAndHoldDelay = 800;

MouseEventListener::MouseEventListener(QQuickItem *parent)
    : QQuickItem(parent),
    m_pressed(false),
    m_pressAndHoldEvent(0),
    m_lastEvent(0),
    m_containsMouse(false)
{
    m_pressAndHoldTimer = new QTimer(this);
    m_pressAndHoldTimer->setSingleShot(true);
    connect(m_pressAndHoldTimer, SIGNAL(timeout()),
            this, SLOT(handlePressAndHold()));

    qmlRegisterType<KDeclarativeMouseEvent>();
    qmlRegisterType<KDeclarativeWheelEvent>();

    setFiltersChildMouseEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton|Qt::RightButton|Qt::MidButton|Qt::XButton1|Qt::XButton2);
}

MouseEventListener::~MouseEventListener()
{
}

void MouseEventListener::setHoverEnabled(bool enable)
{
    if (enable == acceptHoverEvents()) {
        return;
    }

    setAcceptHoverEvents(enable);
    emit hoverEnabledChanged(enable);
}

bool MouseEventListener::hoverEnabled() const
{
    return acceptHoverEvents();
}

void MouseEventListener::hoverEnterEvent(QHoverEvent *event)
{
    Q_UNUSED(event);

    m_containsMouse = true;
    emit containsMouseChanged(true);
}

void MouseEventListener::hoverLeaveEvent(QHoverEvent *event)
{
    Q_UNUSED(event);

    m_containsMouse = false;
    emit containsMouseChanged(false);
}

void MouseEventListener::hoverMoveEvent(QHoverEvent * event)
{
    if (m_lastEvent == event) {
        return;
    }

    QQuickWindow *w = window();
    QPoint screenPos = QPoint();
    if (w) {
        w->mapToGlobal(event->pos());
    }

    KDeclarativeMouseEvent dme(event->pos().x(), event->pos().y(), screenPos.x(), screenPos.y(), Qt::NoButton, Qt::NoButton, event->modifiers());
    emit positionChanged(&dme);
}

bool MouseEventListener::containsMouse() const
{
    return m_containsMouse;
}

void MouseEventListener::mousePressEvent(QMouseEvent *me)
{
    if (m_lastEvent == me) {
        return;
    }

    //FIXME: when a popup window is visible: a click anywhere hides it: but the old qquickitem will continue to think it's under the mouse
    //doesn't seem to be any good way to properly reset this.
    //this msolution will still caused a missed click after the popup is gone, but gets the situation unblocked.
    if (!isUnderMouse()) {
        me->ignore();
        return;
    }
    m_buttonDownPos[me->button()] = me->pos();

    KDeclarativeMouseEvent dme(me->pos().x(), me->pos().y(), me->screenPos().x(), me->screenPos().y(), me->button(), me->buttons(), me->modifiers());
    m_pressAndHoldEvent = new KDeclarativeMouseEvent(me->pos().x(), me->pos().y(), me->screenPos().x(), me->screenPos().y(), me->button(), me->buttons(), me->modifiers());
    emit pressed(&dme);
    m_pressed = true;

    m_pressAndHoldTimer->start(PressAndHoldDelay);
}

void MouseEventListener::mouseMoveEvent(QMouseEvent *me)
{
    if (m_lastEvent == me) {
        return;
    }

    KDeclarativeMouseEvent dme(me->pos().x(), me->pos().y(), me->screenPos().x(), me->screenPos().y(), me->button(), me->buttons(), me->modifiers());
    emit positionChanged(&dme);
}

void MouseEventListener::mouseReleaseEvent(QMouseEvent *me)
{
    if (m_lastEvent == me) {
        return;
    }

    KDeclarativeMouseEvent dme(me->pos().x(), me->pos().y(), me->screenPos().x(), me->screenPos().y(), me->button(), me->buttons(), me->modifiers());
    m_pressed = false;
    emit released(&dme);

    if (boundingRect().contains(me->pos()) && m_pressAndHoldTimer->isActive()) {
        emit clicked(&dme);
        m_pressAndHoldTimer->stop();
    }
}

void MouseEventListener::wheelEvent(QWheelEvent *we)
{
    if (m_lastEvent == we) {
        return;
    }

    KDeclarativeWheelEvent dwe(we->pos(), we->globalPos(), we->delta(), we->buttons(), we->modifiers(), we->orientation());
    emit wheelMoved(&dwe);
}

void MouseEventListener::handlePressAndHold()
{
    if (m_pressed) {
        emit pressAndHold(m_pressAndHoldEvent);
        //delete m_pressAndHoldEvent;
    }
}

QPointF MouseEventListener::buttonDownPos(int btn) const
{
    if (m_buttonDownPos.keys().contains(btn)) {
        return m_buttonDownPos.value(btn);
    }
    return QPointF(0, 0);
}


bool MouseEventListener::childMouseEventFilter(QQuickItem *item, QEvent *event)
{
    if (!isEnabled()) {
        return false;
    }

    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        m_lastEvent = event;
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        //the parent will receive events in its own coordinates
        const QPointF myPos = item->mapToItem(this, me->pos());
        KDeclarativeMouseEvent dme(myPos.x(), myPos.y(), me->screenPos().x(), me->screenPos().y(), me->button(), me->buttons(), me->modifiers());
        m_pressAndHoldEvent = new KDeclarativeMouseEvent(myPos.x(), myPos.y(), me->screenPos().x(), me->screenPos().y(), me->button(), me->buttons(), me->modifiers());
        //qDebug() << "pressed in sceneEventFilter";
        emit pressed(&dme);
        m_pressed = true;

        m_pressAndHoldTimer->start(PressAndHoldDelay);
        break;
    }
    case QEvent::HoverMove: {
        if (!acceptHoverEvents()) {
            break;
        }
        m_lastEvent = event;
        QHoverEvent *he = static_cast<QHoverEvent *>(event);
        const QPointF myPos = item->mapToItem(this, he->pos());

        QQuickWindow *w = window();
        QPoint screenPos = QPoint();
        if (w) {
            w->mapToGlobal(myPos.toPoint());
        }

        KDeclarativeMouseEvent dme(myPos.x(), myPos.y(), screenPos.x(), screenPos.y(), Qt::NoButton, Qt::NoButton, he->modifiers());
        //qDebug() << "positionChanged..." << dme.x() << dme.y();
        m_pressAndHoldEvent = new KDeclarativeMouseEvent(myPos.x(), myPos.y(), screenPos.x(), screenPos.y(), Qt::NoButton, Qt::NoButton, he->modifiers());
        emit positionChanged(&dme);
        break;
    }
    case QEvent::MouseMove: {
        m_lastEvent = event;
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        const QPointF myPos = item->mapToItem(this, me->pos());
        KDeclarativeMouseEvent dme(myPos.x(), myPos.y(), me->screenPos().x(), me->screenPos().y(), me->button(), me->buttons(), me->modifiers());
        //qDebug() << "positionChanged..." << dme.x() << dme.y();
        m_pressAndHoldEvent = new KDeclarativeMouseEvent(myPos.x(), myPos.y(), me->screenPos().x(), me->screenPos().y(), me->button(), me->buttons(), me->modifiers());
        emit positionChanged(&dme);
        break;
    }
    case QEvent::MouseButtonRelease: {
        m_lastEvent = event;
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        const QPointF myPos = item->mapToItem(this, me->pos());
        KDeclarativeMouseEvent dme(myPos.x(), myPos.y(), me->screenPos().x(), me->screenPos().y(), me->button(), me->buttons(), me->modifiers());
        m_pressed = false;

        emit released(&dme);

        if (QPointF(me->pos() - buttonDownPos(me->button())).manhattanLength() <= QApplication::startDragDistance() && m_pressAndHoldTimer->isActive()) {
            emit clicked(&dme);
            m_pressAndHoldTimer->stop();
        }
        break;
    }
    case QEvent::Wheel: {
        m_lastEvent = event;
        QWheelEvent *we = static_cast<QWheelEvent *>(event);
        KDeclarativeWheelEvent dwe(we->pos(), we->globalPos(), we->delta(), we->buttons(), we->modifiers(), we->orientation());
        emit wheelMoved(&dwe);
        break;
    }
    default:
        break;
    }

    return QQuickItem::childMouseEventFilter(item, event);
//    return false;
}

#include "mouseeventlistener.moc"

