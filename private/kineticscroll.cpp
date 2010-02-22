/*
    Copyright (C) 2009 Igor Trindade Oliveira <igor.oliveira@indt.org.br>
    Copyright (C) 2009 Adenilson Cavalcanti <adenilson.silva@idnt.org.br>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kineticscroll_p.h"

#include <QtCore/qglobal.h>
#include <QtCore/qmetatype.h>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QTime>
#include <QGraphicsWidget>
#include <QPoint>
#include <QPropertyAnimation>
#include <QCursor>

#include <kdebug.h>

/* TODO:
 * - clean up the code(remove duplicated code, constify)
 * - port to Plasma::Animator
 */

namespace Plasma
{

class KineticScrollingPrivate
{
public:
    enum Direction {
        None,
        Up,
        Down,
        Left,
        Right
    };

    enum Gesture {
        GestureNone = 0,
        GestureUndefined,
        GestureScroll,
        GestureZoom
    };

    KineticScrollingPrivate()
        : overshoot(20),
        hasOvershoot(true),
        parent(0),
        forwardingEvent(false),
        multitouchGesture(GestureNone)
    {
    }

    void count()
    {
        t = QTime::currentTime();
    }

    void syncViewportRect()
    {
        contentsSize = parent->property("contentsSize").toSizeF();
        viewportGeometry = parent->property("viewportGeometry").toRectF();
    }
    
    bool canScroll(Direction direction, bool hasOvershoot = false) const
    {
        QPointF scrollPosition = -parent->property("scrollPosition").value<QPointF>();
        int offset = (hasOvershoot?overshoot*2:0);

        switch (direction) {
        case Up:
            return (scrollPosition.y() < offset);
        case Down:
            return (scrollPosition.y() + contentsSize.height() + offset >= viewportGeometry.bottom());
        case Left:
            return (scrollPosition.x() < offset);
        case Right:
            return (scrollPosition.x() + contentsSize.width() + offset >= viewportGeometry.right());
        default:
            return true;
        }
    }


    QPointF kinMovement;

    enum BounceStatus {
        Running,
        Finished
    };

    BounceStatus bounceStatus;

    QPropertyAnimation *scrollAnimation;

    int overshoot;
    QPointF cposition;
    bool hasOvershoot;
    QGraphicsWidget *parent;
    QRectF viewportGeometry;
    QSizeF contentsSize;
    QPointF maximum, minimum;
    bool forwardingEvent;
    Gesture multitouchGesture;

    unsigned int timeDelta;
    QTime t;
};


KineticScrolling::KineticScrolling(QGraphicsWidget *parent)
    : d(new KineticScrollingPrivate)
{
    setWidget(parent);
}

KineticScrolling::~KineticScrolling()
{
    delete d;
}

void KineticScrolling::duration( )
{
    d->timeDelta = d->t.msecsTo(QTime::currentTime());
}

void KineticScrolling::overshoot()
{
    QPointF scrollPosition = -d->parent->property("scrollPosition").value<QPointF>();

    if (!d->canScroll(KineticScrollingPrivate::Down) &&
        !d->canScroll(KineticScrollingPrivate::Up)) {
        return;
    }

    if (d->bounceStatus != KineticScrollingPrivate::Running) {
        if ((d->cposition.y() > 0 ) || (d->cposition.y() <= d->minimum.y() + d->overshoot)) {
            QPointF finalPosition;
            d->scrollAnimation->setEasingCurve( QEasingCurve::OutBounce );

            if (d->cposition.y() > 0) {
                finalPosition = QPointF(scrollPosition.x(), 0);
            } else {
                finalPosition = QPointF(d->cposition.x(),
                        -d->contentsSize.height( ) + d->parent->size().height());
            }

            resetAnimation(-finalPosition, 900);
            d->bounceStatus = KineticScrollingPrivate::Running;
        }
    } else {
        d->bounceStatus = KineticScrollingPrivate::Finished;
        d->scrollAnimation->setEasingCurve(QEasingCurve::OutCirc);
    }
}

void KineticScrolling::setScrollValue(QPointF value)
{
    const QPointF pos = thresholdPosition(value);
    QPointF posf(-pos);
    d->parent->setProperty("scrollPosition", posf);

    if ((pos.y() == d->overshoot) || (pos.y() == d->minimum.y())) {
        overshoot();
    }
}

QPointF KineticScrolling::thresholdPosition(QPointF value) const
{
    d->minimum.setX(-d->contentsSize.width() + d->viewportGeometry.width());
    d->minimum.setY(-d->contentsSize.height() + d->viewportGeometry.height()
        -d->overshoot);

    d->minimum.setY(qMin((qreal)d->overshoot, d->minimum.y()));
    d->maximum = value;

    if(d->minimum.x() >= 0) {
        d->cposition.setX(value.x());
    } else {
        d->cposition.setX(qBound(d->minimum.x(), d->maximum.x(), qreal(0)));
    }

    if((-d->contentsSize.height() + d->viewportGeometry.height() - d->overshoot) >= 0) {
        d->cposition.setY(value.y());
    } else {
        d->cposition.setY(qBound(d->minimum.y(), d->maximum.y(), qreal(d->overshoot)));
    }

    return d->cposition;
}

void KineticScrolling::resetAnimation(QPointF finalPosition, int duration)
{
    if (d->scrollAnimation->state() != QAbstractAnimation::Stopped) {
        d->scrollAnimation->stop();
    }

    d->cposition = -finalPosition;
    QPointF tmpPosition = d->parent->property("scrollPosition").value<QPointF>();
    d->scrollAnimation->setStartValue(tmpPosition);

    tmpPosition = finalPosition;

    d->scrollAnimation->setEndValue(tmpPosition);
    d->scrollAnimation->setDuration(duration);
    d->scrollAnimation->start();

}

void KineticScrolling::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (d->scrollAnimation->state() != QAbstractAnimation::Stopped) {
        d->scrollAnimation->stop();
    }

    d->syncViewportRect();
    d->cposition = -d->parent->property("scrollPosition").value<QPointF>();

    d->count();
    d->kinMovement = QPointF(0,0);
    d->scrollAnimation->setEasingCurve(QEasingCurve::OutCirc);
}

void KineticScrolling::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF movement = event->lastPos().toPoint() - event->pos().toPoint();
    const QPointF scrollPosition = -d->parent->property("scrollPosition").value<QPointF>();

    if (!movement.isNull()) {
        if ((d->contentsSize.width() < d->viewportGeometry.width()) &&
                (d->contentsSize.height() < d->viewportGeometry.height())) {
            d->kinMovement = QPointF(0, 0);
            movement = QPointF(0, 0);
        } else if (d->contentsSize.height() < d->viewportGeometry.height()) {
            d->kinMovement += QPointF(movement.x(), 0);
            movement.setY(0);
        } else if (d->contentsSize.width() < d->viewportGeometry.width()) {
            d->kinMovement = QPointF(0, movement.y());
            movement.setX(0);
        } else {
            d->kinMovement += movement;
        }
        setScrollValue(scrollPosition - movement);
    }
}

void KineticScrolling::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    if (d->scrollAnimation->state() != QAbstractAnimation::Running) {
        duration();

        if (d->kinMovement != QPointF(0, 0)) {
            const QPointF scrollPosition = -d->parent->property("scrollPosition").toPointF();
            d->kinMovement = QPointF(d->kinMovement.x()*3, d->kinMovement.y()*3);
            const QPointF finalPos = thresholdPosition(scrollPosition - d->kinMovement);
            resetAnimation( -finalPos, d->timeDelta*8 );
        }
    }
}

void KineticScrolling::wheelReleaseEvent(QGraphicsSceneWheelEvent *event)
{
    Q_UNUSED(event);
    d->syncViewportRect();
    d->kinMovement = QPointF(0,0);

    if((event->orientation() == Qt::Vertical) &&
       ((event->delta() < 0) && d->canScroll(KineticScrollingPrivate::Down) ||
       (event->delta() > 0) && d->canScroll(KineticScrollingPrivate::Up))) {
        d->kinMovement.setY(d->kinMovement.y() - event->delta());
    } else if ((event->orientation() == Qt::Vertical) ||
               (!d->canScroll(KineticScrollingPrivate::Down) &&
               !d->canScroll(KineticScrollingPrivate::Up))) {
        if (((event->delta() < 0) &&
            d->canScroll(KineticScrollingPrivate::Right)) ||
            (event->delta() > 0 && d->canScroll(KineticScrollingPrivate::Left))) {
            d->kinMovement.setX(d->kinMovement.x() - event->delta());
        } else {
            event->ignore( );
        }
    } else {
        event->ignore( );
        return;
    }

    const QPointF scrollPosition = -d->parent->property("scrollPosition").value<QPointF>();
    const QPointF pos = scrollPosition - d->kinMovement*2;
    const QPointF finalPos = thresholdPosition(pos);

    d->scrollAnimation->setEasingCurve(QEasingCurve::OutCirc);
    resetAnimation(-finalPos, 900);
}

void KineticScrolling::keyPressEvent(QKeyEvent *event)
{
    const int movement = 30;
    const int duration = 900;

    QPointF scrollPosition = -d->parent->property("scrollPosition").value<QPointF>();
    QPointF finalPos;
    switch (event->key()) {
    case Qt::Key_Left:
        scrollPosition.setX(scrollPosition.x() + movement);
        finalPos = thresholdPosition(scrollPosition);
        resetAnimation(-finalPos, duration);
        break;
    case Qt::Key_Right:
        scrollPosition.setX(scrollPosition.x() - movement);
        finalPos = thresholdPosition(scrollPosition);
        resetAnimation(-finalPos, duration);
        break;
    case Qt::Key_Up:
        scrollPosition.setY(scrollPosition.y() + movement);
        finalPos = thresholdPosition(scrollPosition);
        resetAnimation(-finalPos, duration);
        break;
    case Qt::Key_Down:
        scrollPosition.setY(scrollPosition.y() - movement);
        finalPos = thresholdPosition(scrollPosition);
        resetAnimation(-finalPos, duration);
        break;
    default:
        break;
    }
}

void KineticScrolling::setWidget(QGraphicsWidget *parent)
{
    if (d->parent) {
        d->parent->removeEventFilter(this);
        disconnect(d->scrollAnimation, SIGNAL(finished()), this, SLOT(overshoot()));
        disconnect(d->scrollAnimation,
                SIGNAL(stateChanged(QAbstractAnimation::State, QAbstractAnimation::State)), this,
                SIGNAL(stateChanged(QAbstractAnimation::State, QAbstractAnimation::State)));
        delete d->scrollAnimation;
    }

    setParent(parent);

    d->parent = parent;

    d->scrollAnimation = new QPropertyAnimation(parent, "scrollPosition", parent);
    connect(d->scrollAnimation, SIGNAL(finished()), this, SLOT(overshoot()));
    connect(d->scrollAnimation,
            SIGNAL(stateChanged(QAbstractAnimation::State, QAbstractAnimation::State)), this,
            SIGNAL(stateChanged(QAbstractAnimation::State, QAbstractAnimation::State)));
    d->scrollAnimation->setEasingCurve(QEasingCurve::OutCirc);

    if (parent) {
        d->parent->installEventFilter(this);
    }
    /* TODO: add a new property in plasma::ScrollWidget 'hasOvershoot' */
}

void KineticScrolling::stop()
{
    d->scrollAnimation->stop();
}


bool KineticScrolling::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);
    Q_UNUSED(event);

    if (d->forwardingEvent) {
        return false;
    }

    bool notBlocked = true;
    if (d->multitouchGesture == KineticScrollingPrivate::GestureNone &&
            d->parent && d->parent->scene()) {
        d->forwardingEvent = true;
        notBlocked = d->parent->scene()->sendEvent(d->parent, event);
        d->forwardingEvent = false;
    }

   if (event->type() != QEvent::TouchBegin &&
       event->type() != QEvent::TouchUpdate &&
       event->type() != QEvent::TouchEnd &&
       (!notBlocked ||
       ((event->type() != QEvent::GraphicsSceneMousePress && event->isAccepted()) &&
        (event->type() != QEvent::GraphicsSceneWheel && event->isAccepted())))) {
       return true;
   }

    QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);
    QGraphicsSceneWheelEvent *we = static_cast<QGraphicsSceneWheelEvent *>(event);

    switch (event->type()) {
    case QEvent::GraphicsSceneMousePress:
        mousePressEvent(me);
        break;
    case QEvent::GraphicsSceneMouseRelease:
        mouseReleaseEvent(me);
        break;
    case QEvent::GraphicsSceneMouseMove:
        mouseMoveEvent(me);
        break;
    case QEvent::TouchBegin:
        mousePressEvent(0);
        break;
    case QEvent::TouchUpdate: {
        QList<QTouchEvent::TouchPoint> touchPoints = static_cast<QTouchEvent *>(event)->touchPoints();
        if (touchPoints.count() == 2) {
            const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
            const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();
            const QLineF line0(touchPoint0.lastPos(), touchPoint1.lastPos());
            const QLineF line1(touchPoint0.pos(), touchPoint1.pos());
            const QLineF startLine(touchPoint0.startPos(), touchPoint1.startPos());
            const QPointF point = line1.pointAt(0.5);
            const QPointF lastPoint = line0.pointAt(0.5);

            if (d->multitouchGesture == KineticScrollingPrivate::GestureNone) {
                d->multitouchGesture = KineticScrollingPrivate::GestureUndefined;
            }
            if (d->multitouchGesture == KineticScrollingPrivate::GestureUndefined) {
                const int zoomDistance = qAbs(line1.length() - startLine.length());
                const int dragDistance = (startLine.pointAt(0.5) - point).manhattanLength();

                if (zoomDistance - dragDistance > 30) {
                    d->multitouchGesture = KineticScrollingPrivate::GestureZoom;
                } else if (dragDistance - zoomDistance > 30) {
                    d->multitouchGesture = KineticScrollingPrivate::GestureScroll;
                }
            }

            if (d->multitouchGesture ==  KineticScrollingPrivate::GestureScroll) {
                QGraphicsSceneMouseEvent fakeEvent;
                fakeEvent.setPos(point);
                fakeEvent.setLastPos(lastPoint);
                mouseMoveEvent(&fakeEvent);
            } else if (d->multitouchGesture == KineticScrollingPrivate::GestureZoom) {
                qreal scaleFactor = 1;
                if (line0.length() > 0) {
                    scaleFactor = line1.length() / line0.length();
                }

                qreal zoom = d->parent->property("zoomFactor").toReal();
                d->parent->setProperty("zoomFactor", zoom * scaleFactor);
            }
        }
        break;
    }
    case QEvent::TouchEnd:
        mouseReleaseEvent(0);
        d->multitouchGesture = KineticScrollingPrivate::GestureNone;
        break;

    case QEvent::GraphicsSceneWheel:
        wheelReleaseEvent(we);
        break;
    case QEvent::KeyPress: {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        keyPressEvent(ke);
        break;
    }
    default:
        break;
    }

    return true;
}

} // namespace Plasma
