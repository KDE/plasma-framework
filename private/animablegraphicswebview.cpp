/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2010 Davide Bettio <davide.bettio@kdemail.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "animablegraphicswebview_p.h"

#include <QtGui/QApplication>
#include <QGestureEvent>
#include <QPinchGesture>
#include <QtWebKit/QWebFrame>

#include <qwebpage.h>
#include <kglobalsettings.h>

using namespace Plasma;

AnimableGraphicsWebView::AnimableGraphicsWebView(QGraphicsItem * parent)
    : GraphicsWebViewBase(parent)
{
    setAcceptTouchEvents(true);
    m_dragToScroll = false;
}

void AnimableGraphicsWebView::setDragToScroll(bool enable)
{
    m_dragToScroll = enable;
}

bool AnimableGraphicsWebView::dragToScroll() const
{
    return m_dragToScroll;
}

QSizeF AnimableGraphicsWebView::contentsSize() const
{
    if (!page()) {
        return QSizeF();
    } else {
        return page()->mainFrame()->contentsSize();
    }
}

void AnimableGraphicsWebView::setScrollPosition(const QPointF &position)
{
    if (!page()) {
        return;
    } else {
        page()->mainFrame()->setScrollPosition(position.toPoint());
    }
}

QPointF AnimableGraphicsWebView::scrollPosition() const
{
    if (!page() || !page()->mainFrame()) {
        return QPointF();
    } else {
        return page()->mainFrame()->scrollPosition();
    }
}

qreal AnimableGraphicsWebView::zoom() const
{
    return GraphicsWebViewBase::zoomFactor();
}

void AnimableGraphicsWebView::setZoom(const qreal zoom)
{
    GraphicsWebViewBase::setZoomFactor(zoom);
}

QRectF AnimableGraphicsWebView::viewportGeometry() const
{
    QRectF result;
    if (!page()) {
        return result;
    } else {
        return page()->mainFrame()->geometry();
    }
}

void AnimableGraphicsWebView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!page()) {
        GraphicsWebViewBase::mouseMoveEvent(event);
        return;
    }

    if (m_dragToScroll) {
        return;
    }

    QMouseEvent me(QEvent::MouseMove, event->pos().toPoint(), event->button(),
            event->buttons(), event->modifiers());
    page()->event(&me);

    if (me.isAccepted()) {
        event->accept();
    }
}

void AnimableGraphicsWebView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!page()) {
        GraphicsWebViewBase::mousePressEvent(event);
        return;
    }

    m_lastScrollPosition = scrollPosition();
    setFocus();

    QMouseEvent me(QEvent::MouseButtonPress, event->pos().toPoint(),
            event->button(), event->buttons(), event->modifiers());

    page()->event(&me);
    if (me.isAccepted() && !m_dragToScroll) {
        event->accept();
    }
}

void AnimableGraphicsWebView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!page()) {
        GraphicsWebViewBase::mouseReleaseEvent(event);
        return;
    }

    QMouseEvent me(QEvent::MouseButtonRelease, event->pos().toPoint(),
            event->button(),event->buttons(), event->modifiers());

    if (!m_dragToScroll || (scrollPosition() - m_lastScrollPosition).manhattanLength() < KGlobalSettings::dndEventDelay()) {
        page()->event(&me);
    }

    if (me.isAccepted() && !m_dragToScroll) {
        event->accept();
    }
}

void AnimableGraphicsWebView::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (!page()) {
        GraphicsWebViewBase::wheelEvent(event);
        return;
    }

    QWheelEvent we(event->pos().toPoint(), event->delta(), event->buttons(),
            event->modifiers(), event->orientation());
    page()->event(&we);

    event->setAccepted(!m_dragToScroll);
}

qreal AnimableGraphicsWebView::scrollPositionX() const
{
    return page()->mainFrame()->scrollPosition().x();
}

void AnimableGraphicsWebView::setScrollPositionX(qreal position)
{
    QPointF pt(position, scrollPositionY());
    setScrollPosition(pt);
}

qreal AnimableGraphicsWebView::scrollPositionY() const
{
    return page()->mainFrame()->scrollPosition().y();
}

void AnimableGraphicsWebView::setScrollPositionY(qreal position)
{
    QPointF pt(scrollPositionX(), position);
    setScrollPosition(pt);
}


#include "animablegraphicswebview_p.moc"

