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
#include <QGraphicsSceneMouseEvent>
#include <QTime>
#include <QDebug>
#include <QGraphicsWidget>
#include <QPoint>

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
    KineticScrollingPrivate(): timerID(0),
                               overshoot(20), bounceFlag(0), hasOvershoot(true),
                               friction(0.8)
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
        qreal tmp(kinMovement.y());
        tmp *= friction;
        kinMovement.setY(tmp);
    }

    unsigned int timeDelta;
    qreal scrollVelocity;
    QPointF kinMovement;

    enum { None, Up, Down };
    int timerID, overshoot, direction;
    QPointF cposition;
    char bounceFlag;
    bool hasOvershoot;
    QObject *parent;
    QRectF geo;
    int maximum, minimum;
    qreal friction;

    QTime t;
};


KineticScrolling::KineticScrolling(): d(0)
{
    d = new KineticScrollingPrivate;
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
    duration();
    QPointF temp = event->pos().toPoint() - event->lastPos().toPoint();
    if (!temp.isNull()) {
        d->kinMovement += temp;
        /* Not so fast baby! */
        d->kinMovement *= 0.3;
    }

    if (d->timeDelta > 600) {
        if (d->kinMovement.y() > 0)
            d->kinMovement.setY(3);
        else
            d->kinMovement.setY(-3);
    }

    d->direction = KineticScrollingPrivate::None;
    startAnimationTimer(30);
}

void KineticScrolling::wheelReleaseEvent(QGraphicsSceneWheelEvent *event)
{
    doneOvershoot();
    d->geo = d->parent->property("viewport").toRectF();
    if (d->direction == KineticScrollingPrivate::None) {
        mousePressEvent(0);
        duration();
        /* scroll down is negative in pixels and we want 6% */
        int temp = event->delta();
        if (temp < 0)
            temp = d->geo.height() * 0.07;
        else
            temp = d->geo.height() * -0.07;

        d->kinMovement.setY(kinMovement().y() + temp);
        startAnimationTimer(30);
    }

}

void KineticScrolling::startAnimationTimer(int interval)
{
    if (d->timerID) {
        killTimer(d->timerID);
        d->timerID = 0;
    }

    d->geo = d->parent->property("viewport").toRectF();
    d->timerID = QObject::startTimer(interval);
}

void KineticScrolling::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    d->cposition.setY(d->parent->property("verticalScrollValue").value<qreal>());

    if (d->direction == KineticScrollingPrivate::None) {
        if ((qAbs(d->kinMovement.y()) < 5.0)) {
            if ((d->cposition.y() > 100) ||
                (d->cposition.y() < 0)) {
                if (d->cposition.y() < 0)
                    d->kinMovement.setY(2);
                else
                    d->kinMovement.setY(-2);

                d->parent->setProperty("verticalScrollValue", d->cposition.y()
                                       + d->kinMovement.y());
                return;
            }
            if (d->timerID) {
                killTimer(d->timerID);
            }

        } else
            d->applyFriction();

        setKineticScrollValue(d->kinMovement);

    } else {
        /* TODO: call bouncer */
    }
}

void KineticScrolling::setKineticScrollValue(QPointF value)
{
    if (!(d->geo.height())) {
        d->kinMovement.setY(0);
        return;
    }

    qreal movement = (100 * value.y())/int(d->geo.height());
    qreal final;

    movement += d->cposition.y();

    if (movement > d->maximum) {
        d->kinMovement.setY(-(d->overshoot));
    } else if (movement < d->minimum) {
        d->kinMovement.setY(d->overshoot);
    } else {
        final = qBound((qreal)d->minimum, movement, (qreal)d->maximum);
        d->parent->setProperty("verticalScrollValue", final);
    }

    /* TODO: use 'ScrollWidget::HorizontalScrollValue */
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
    d->parent = parent;
    /* TODO: add a new property in plasma::ScrollWidget 'hasOvershoot' */
}

} // namespace Plasma

