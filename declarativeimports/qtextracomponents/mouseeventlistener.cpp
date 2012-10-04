/*
    Copyright 2011 Marco Martin <notmart@gmail.com>

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

#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <QtCore/QTimer>

#include <KDebug>

MouseEventListener::MouseEventListener(QDeclarativeItem *parent)
    : QDeclarativeItem(parent),
    m_pressed(false),
    m_pressAndHoldEvent(0),
    m_lastEvent(0),
    m_containsMouse(false)
{
    qmlRegisterType<KDeclarativeMouseEvent>();
    qmlRegisterType<KDeclarativeWheelEvent>();

    setFiltersChildEvents(true);
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

void MouseEventListener::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_containsMouse = true;
    emit containsMouseChanged(true);
}

void MouseEventListener::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_containsMouse = false;
    emit containsMouseChanged(false);
}

bool MouseEventListener::containsMouse() const
{
    return m_containsMouse;
}

void MouseEventListener::mousePressEvent(QGraphicsSceneMouseEvent *me)
{
    if (m_lastEvent == me) {
        return;
    }

    //FIXME: when a popup window is visible: a click anywhere hides it: but the old qgraphicswidget will continue to think it's under the mouse
    //doesn't seem to be any good way to properly reset this.
    //this msolution will still caused a missed click after the popup is gone, but gets the situation unblocked.
    if (!isUnderMouse()) {
        me->ignore();
        return;
    }

    KDeclarativeMouseEvent dme(me->pos().x(), me->pos().y(), me->screenPos().x(), me->screenPos().y(), me->button(), me->buttons(), me->modifiers());
    m_pressAndHoldEvent = new KDeclarativeMouseEvent(me->pos().x(), me->pos().y(), me->screenPos().x(), me->screenPos().y(), me->button(), me->buttons(), me->modifiers());
    emit pressed(&dme);
    m_pressed = true;
    //delete m_pressAndHoldEvent;
    //m_pressAndHoldEvent = &dme;
    //new KDeclarativeMouseEvent(me->pos().x(), me->pos().y(), me->screenPos().x(), me->screenPos().y(), me->button(), me->buttons(), me->modifiers());;
    QTimer::singleShot(800, this, SLOT(handlePressAndHold()));
}

void MouseEventListener::mouseMoveEvent(QGraphicsSceneMouseEvent *me)
{
    if (m_lastEvent == me) {
        return;
    }

    KDeclarativeMouseEvent dme(me->pos().x(), me->pos().y(), me->screenPos().x(), me->screenPos().y(), me->button(), me->buttons(), me->modifiers());
    emit positionChanged(&dme);
}

void MouseEventListener::mouseReleaseEvent(QGraphicsSceneMouseEvent *me)
{
    if (m_lastEvent == me) {
        return;
    }

    KDeclarativeMouseEvent dme(me->pos().x(), me->pos().y(), me->screenPos().x(), me->screenPos().y(), me->button(), me->buttons(), me->modifiers());
    m_pressed = false;
    emit released(&dme);
}

void MouseEventListener::wheelEvent(QGraphicsSceneWheelEvent *we)
{
    if (m_lastEvent == we) {
        return;
    }

    KDeclarativeWheelEvent dwe(we->pos(), we->screenPos(), we->delta(), we->buttons(), we->modifiers(), we->orientation());
    emit wheelMoved(&dwe);
}

void MouseEventListener::handlePressAndHold()
{
    if (m_pressed) {
        //KDeclarativeMouseEvent dme = m_pressAndHoldEvent;
        //const QPointF myPos = m_pressAndHoldEvent->pos();
        //KDeclarativeMouseEvent dme(myPos.x(), myPos.y(), m_pressAndHoldEvent->screenPos().x(), m_pressAndHoldEvent->screenPos().y(), m_pressAndHoldEvent->button(), m_pressAndHoldEvent->buttons(), m_pressAndHoldEvent->modifiers());
        emit pressAndHold(m_pressAndHoldEvent);
        //delete m_pressAndHoldEvent;
    }
}


bool MouseEventListener::sceneEventFilter(QGraphicsItem *item, QEvent *event)
{
    if (!isEnabled()) {
        return false;
    }

    m_lastEvent = event;

    switch (event->type()) {
    case QEvent::GraphicsSceneMousePress: {
        QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);
        //the parent will receive events in its own coordinates
        const QPointF myPos = item->mapToItem(this, me->pos());
        KDeclarativeMouseEvent dme(myPos.x(), myPos.y(), me->screenPos().x(), me->screenPos().y(), me->button(), me->buttons(), me->modifiers());
        m_pressAndHoldEvent = new KDeclarativeMouseEvent(myPos.x(), myPos.y(), me->screenPos().x(), me->screenPos().y(), me->button(), me->buttons(), me->modifiers());
        //kDebug() << "pressed in sceneEventFilter";
        emit pressed(&dme);
        m_pressed = true;
        //delete m_pressAndHoldEvent;
        //m_pressAndHoldEvent = new KDeclarativeMouseEvent(me->pos().x(), me->pos().y(), me->screenPos().x(), me->screenPos().y(), me->button(), me->buttons(), me->modifiers());;
        //m_pressAndHoldEvent = &dme;
        QTimer::singleShot(800, this, SLOT(handlePressAndHold()));
        break;
    }
    case QEvent::GraphicsSceneMouseMove: {
        QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);
        const QPointF myPos = item->mapToItem(this, me->pos());
        KDeclarativeMouseEvent dme(myPos.x(), myPos.y(), me->screenPos().x(), me->screenPos().y(), me->button(), me->buttons(), me->modifiers());
        //kDebug() << "positionChanged..." << dme.x() << dme.y();
        m_pressAndHoldEvent = new KDeclarativeMouseEvent(myPos.x(), myPos.y(), me->screenPos().x(), me->screenPos().y(), me->button(), me->buttons(), me->modifiers());
        emit positionChanged(&dme);
        break;
    }
    case QEvent::GraphicsSceneMouseRelease: {
        QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);
        const QPointF myPos = item->mapToItem(this, me->pos());
        KDeclarativeMouseEvent dme(myPos.x(), myPos.y(), me->screenPos().x(), me->screenPos().y(), me->button(), me->buttons(), me->modifiers());
        emit released(&dme);
        m_pressed = false;
        break;
    }
    case QEvent::GraphicsSceneWheel: {
        QGraphicsSceneWheelEvent *we = static_cast<QGraphicsSceneWheelEvent *>(event);
        KDeclarativeWheelEvent dwe(we->pos(), we->screenPos(), we->delta(), we->buttons(), we->modifiers(), we->orientation());
        emit wheelMoved(&dwe);
        break;
    }
    default:
        break;
    }

    return QDeclarativeItem::sceneEventFilter(item, event);
}

#include "mouseeventlistener.moc"

