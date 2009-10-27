////////////////////////////////////////////////////////////////////////
// kineticscroll.cpp                                                   //
//                                                                     //
// Copyright(C) 2009 Igor Trindade Oliveira <igor.oliveira@indt.org.br>//
// Copyright(C) 2009 Adenilson Cavalcanti <adenilson.silva@idnt.org.br>//
//                                                                     //
// This library is free software; you can redistribute it and/or       //
// modify it under the terms of the GNU Lesser General Public          //
// License as published by the Free Software Foundation; either        //
// version 2.1 of the License, or (at your option) any later version.  //
//                                                                     //
// This library is distributed in the hope that it will be useful,     //
// but WITHOUT ANY WARRANTY; without even the implied warranty of      //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   //
// Lesser General Public License for more details.                     //
//                                                                     //
// You should have received a copy of the GNU Lesser General Public    //
// License along with this library; if not, write to the Free Software //
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA       //
// 02110-1301  USA                                                     //
/////////////////////////////////////////////////////////////////////////
#include <QtCore/qglobal.h>
#include <QtCore/qmetatype.h>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QTime>
#include <QDebug>
#include <QGraphicsWidget>
#include <QPoint>

#include <kdebug.h>

#include "kineticscroll_p.h"

/* TODO:
 * - implement horizontal scrolling
 * - implement bouncing animation (not sure if it adds in usability
 * in plasma-netbook).
 * - Merge QPropertyAnimation code when KDE migrates to Qt 4.6
 * (http://repo.or.cz/w/kineticlist.git)
 * - swap the 'magic numbers' for consts
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

    KineticScrollingPrivate()
        : timerID(0),
        overshoot(20),
        bounceFlag(0),
        hasOvershoot(true),
        parent(0),
        friction(0.8),
        forwardingEvent(false)
    {
        maximum = 100 + overshoot;
        minimum = -overshoot;
    }

    void count()
    {
        t = QTime::currentTime();
    }

    void applyFriction()
    {
        kinMovement *= friction;
    }

    void syncViewportRect()
    {
        contentsSize = parent->property("contentsSize").toSizeF();
        viewportGeometry = parent->property("viewportGeometry").toRectF();
        //TODO: overshoot and friction will have to be independent horizontal and vertical
        overshoot = 20*(1.0 - viewportGeometry.height()/contentsSize.height());
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

    unsigned int timeDelta;
    qreal scrollVelocity;
    QPointF kinMovement;


    int timerID, overshoot, direction;
    QPointF cposition;
    char bounceFlag;
    bool hasOvershoot;
    QGraphicsWidget *parent;
    QRectF viewportGeometry;
    QSizeF contentsSize;
    int maximum, minimum;
    qreal friction;
    bool forwardingEvent;

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

QPointF KineticScrolling::kinMovement()
{
    return d->kinMovement;
}

void KineticScrolling::duration()
{
    d->timeDelta = d->t.msecsTo(QTime::currentTime());
}

void KineticScrolling::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    d->syncViewportRect();
    d->cposition = -d->parent->property("scrollPosition").value<QPointF>();

    if (d->cposition == QPointF(0, 0) &&
        d->viewportGeometry.width() + 2 >= d->contentsSize.width() &&
        d->viewportGeometry.height() + 2 >= d->contentsSize.height()) {
        event->ignore();
    }

    doneOvershoot();
    Q_UNUSED(event);
    d->count();
    d->scrollVelocity = 0;
    d->kinMovement = QPoint(0, 0);
}

void KineticScrolling::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF temp = event->lastPos().toPoint() - event->pos().toPoint();

    if (!temp.isNull()) {
        d->kinMovement += temp;
    }
    /* After */
    setKineticScrollValue(d->kinMovement);
}

void KineticScrolling::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    duration();
    /* slow down a bit */
    d->kinMovement /= 4;

    if (d->timeDelta > 600) {
        if (d->kinMovement.y() > 0) {
            d->kinMovement.setY(6);
        } else {
            d->kinMovement.setY(-6);
        }
    }

    d->direction = KineticScrollingPrivate::None;
    startAnimationTimer(30);
}

void KineticScrolling::wheelReleaseEvent(QGraphicsSceneWheelEvent *event)
{
    doneOvershoot();
    d->syncViewportRect();
    if (d->direction == KineticScrollingPrivate::None) {
        mousePressEvent(0);
        duration();

        int temp = -event->delta();

        if ((event->delta() < 0 && d->canScroll(KineticScrollingPrivate::Down)) ||
            (event->delta() > 0 && d->canScroll(KineticScrollingPrivate::Up))){
            d->kinMovement.setY(kinMovement().y() + temp);
        } else if (!d->canScroll(KineticScrollingPrivate::Down) &&
                   !d->canScroll(KineticScrollingPrivate::Up)) {
            if ((event->delta() < 0 && d->canScroll(KineticScrollingPrivate::Right)) ||
                   (event->delta() > 0 && d->canScroll(KineticScrollingPrivate::Left))) {
                d->kinMovement.setX(kinMovement().x() + temp);
            }
        } else {
            event->ignore();
            return;
        }

        startAnimationTimer(30);
    }

}

void KineticScrolling::startAnimationTimer(int interval)
{
    if (d->timerID) {
        killTimer(d->timerID);
        d->timerID = 0;
    }

    d->syncViewportRect();

    d->timerID = QObject::startTimer(interval);
}

void KineticScrolling::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    d->cposition = -d->parent->property("scrollPosition").value<QPointF>();

    if (d->direction == KineticScrollingPrivate::None) {

        if (qAbs(d->kinMovement.y()) < 5.0 &&
            qAbs(d->kinMovement.x()) < 5.0 ) {

            d->kinMovement = QPoint(0,0);

            if (d->canScroll(KineticScrollingPrivate::Down) && !d->canScroll(KineticScrollingPrivate::Up)) {
                if (d->cposition.y() > 2) {
                    d->kinMovement.setY(-2);
                }
            } else if (d->canScroll(KineticScrollingPrivate::Up) && !d->canScroll(KineticScrollingPrivate::Down)) {
                if (d->cposition.y() + d->contentsSize.height() < d->viewportGeometry.bottom()) {
                    d->kinMovement.setY(1.0);
                }
            }

            if (d->canScroll(KineticScrollingPrivate::Right) && !d->canScroll(KineticScrollingPrivate::Left)) {
                if (d->cposition.x() > 2) {
                    d->kinMovement.setX(-2);
                }
            } else if (d->canScroll(KineticScrollingPrivate::Left) && !d->canScroll(KineticScrollingPrivate::Right)) {
                if (d->cposition.x() + d->contentsSize.width() < d->viewportGeometry.right()) {
                    d->kinMovement.setX(2);
                }
            }

            if (!d->kinMovement.isNull()) {
                d->parent->setProperty("scrollPosition", -(d->cposition
                                        + d->kinMovement));
                return;
            }
            if (d->timerID) {
                killTimer(d->timerID);
            }

        } else {
            d->applyFriction();
        }

        setKineticScrollValue(d->kinMovement);

    } else {
        /* TODO: call bouncer */
    }
}

void KineticScrolling::setKineticScrollValue(QPointF value)
{
    if (d->viewportGeometry.isEmpty()) {
        d->kinMovement = QPoint();
        return;
    }

    QPointF scrollPosition = -d->parent->property("scrollPosition").value<QPointF>();

    QPointF movement = d->cposition - value;

    if (movement.x() > d->overshoot*2 ||
        movement.x() + d->contentsSize.width() < d->viewportGeometry.right() - d->overshoot*2) {
        movement.setX(scrollPosition.x());
    }
    if (movement.y() > d->overshoot*2 ||
        movement.y() + d->contentsSize.height() < d->viewportGeometry.bottom() - d->overshoot*2) {
        movement.setY(scrollPosition.y());
    }
    d->parent->setProperty("scrollPosition", -movement);


}

void KineticScrolling::bounceTimer()
{
    d->applyFriction();
    qreal movement = d->kinMovement.y();
    d->cposition.setY(d->parent->property("verticalScrollValue").value<qreal>());
    movement += d->cposition.y();

    if ((d->direction == KineticScrollingPrivate::Down) &&
        (d->cposition.y() > 100)) {
        d->parent->setProperty("verticalScrollValue", movement);

    } else if ((d->direction == KineticScrollingPrivate::Up) &&
        (d->cposition.y() < 0)) {
        d->parent->setProperty("verticalScrollValue", movement);

    } else {
        doneOvershoot();
    }

}

void KineticScrolling::doneOvershoot(void)
{
    d->direction = KineticScrollingPrivate::None;
    d->kinMovement.setY(0);
    killTimer(d->timerID);
    d->timerID = 0;
}

void KineticScrolling::setWidget(QGraphicsWidget *parent)
{
    if (d->parent) {
        d->parent->removeEventFilter(this);
    }

    setParent(parent);

    d->parent = parent;

    if (parent) {
        d->parent->installEventFilter(this);
    }
    /* TODO: add a new property in plasma::ScrollWidget 'hasOvershoot' */
}

bool KineticScrolling::eventFilter(QObject *watched, QEvent *event)
{
    if (d->forwardingEvent) {
        return false;
    }

    bool notBlocked = true;
    if (d->parent && d->parent->scene()) {
        d->forwardingEvent = true;
        notBlocked = d->parent->scene()->sendEvent(d->parent, event);
        d->forwardingEvent = false;
    }

   if (!notBlocked ||
       ((event->type() != QEvent::GraphicsSceneMousePress && event->isAccepted()) &&
       (event->type() != QEvent::GraphicsSceneWheel && event->isAccepted()))) {
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
    case QEvent::GraphicsSceneWheel:
        wheelReleaseEvent(we);
        break;
    default:
        break;
    }

    return true;
}

} // namespace Plasma

