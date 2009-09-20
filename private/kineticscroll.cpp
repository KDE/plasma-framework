/////////////////////////////////////////////////////////////////////////
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
 * - write a factory to animation/state object
 * - move timer based code to an implementation of the factory
 * - write another implementation using QPropertyAnimation Qt 4.6
 * - generalize code concerning viewport and other parameters
 * - swap the 'magic numbers' for consts
 * - consider if direct access to control state/velocity variables should
 * be done directly or using functions
 */

namespace Plasma
{

class KineticScrollingPrivate
{
public:
    KineticScrollingPrivate(): mScrollVelocity(0), timerID(0),
                               overshoot(40), bounceFlag(0), hasOvershoot(true)
    { }

    void count()
    {
        t = QTime::currentTime();
        t.start();
    }

    unsigned int elapsed()
    {
        return t.restart();
    }

    void verticalScroll(int value)
    {
        widget->setPos(QPoint(0, -value*10));
    }

    void horizontalScroll(int value)
    {
        widget->setPos(QPoint(-value*10, 0));
    }


    unsigned int timeDelta;
    qreal scrollVelocity;
    QPoint movement;
    QPoint kinMovement;

    qreal mScrollVelocity;
    enum { None, Up, Down };
    int timerID, overshoot, direction;
    QPoint cposition, minimalPos, maximumPos;
    char bounceFlag;
    bool hasOvershoot;

    QGraphicsWidget *widget;
    QGraphicsWidget *scrollingWidget;

protected:
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

QPoint KineticScrolling::movement()
{
    return d->movement;
}

QPoint KineticScrolling::kinMovement()
{
    return d->kinMovement;
}

qreal KineticScrolling::duration()
{
    return d->timeDelta;
}

void KineticScrolling::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    doneOvershoot();

    Q_UNUSED(event);
    d->count();
    d->scrollVelocity = 0;
    d->movement = QPoint(0, 0);
    d->kinMovement = QPoint(0, 0);
}

void KineticScrolling::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPoint temp = event->lastPos().toPoint() - event->pos().toPoint();
    if (!temp.isNull()) {
        d->movement = temp;
        d->kinMovement += temp;
    }
    /* After */
    setKineticScrollValue(movement());
}

void KineticScrolling::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    d->timeDelta = d->elapsed();
    QPoint temp = event->lastPos().toPoint() - event->pos().toPoint();
    if (!temp.isNull())
        d->kinMovement += temp;

    if (d->timeDelta > 600) {
        if (d->kinMovement.y() > 0)
            d->kinMovement.setY(3);
        else
            d->kinMovement.setY(-3);
    }

    /* The after */
    if (d->cposition.y() > 0) {
        d->direction = KineticScrollingPrivate::Up;
    } else if (abs(d->cposition.y()) >
               abs(d->scrollingWidget->size().height() - d->widget->size().height()))  {
        d->direction = KineticScrollingPrivate::Down;
    } else { 
        d->direction = KineticScrollingPrivate::None;
        d->mScrollVelocity = d->kinMovement.x() + d->kinMovement.y();
        startAnimationTimer(30);
        return;
    }

    d->mScrollVelocity = 5;
    killTimer(d->timerID);
    startAnimationTimer(50);
}

void KineticScrolling::wheelReleaseEvent(QGraphicsSceneWheelEvent *event)
{

    mousePressEvent(0);

    /* Core */
    d->timeDelta = d->elapsed();
    int temp = event->delta();
    temp *= -0.5;
    d->kinMovement.setY(kinMovement().y() + temp);

    /* After */
    d->mScrollVelocity = kinMovement().y();
    startAnimationTimer(30);

}

void KineticScrolling::startAnimationTimer(int interval)
{
    if (d->timerID) {
        killTimer(d->timerID);
        d->timerID = 0;
    }

    d->timerID = QObject::startTimer(interval);
}

void KineticScrolling::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    if (d->direction == KineticScrollingPrivate::None) {
        d->mScrollVelocity *= 0.9;
        if (qAbs(d->mScrollVelocity) < 5.0) {
            if (d->timerID)
                killTimer(d->timerID);

        }
        setKineticScrollValue( QPoint(d->mScrollVelocity, d->mScrollVelocity));
    } else {
        if ((d->timerID != 0) && (d->direction != KineticScrollingPrivate::None)) {
            bounceTimer();
        }
    }										
}

void KineticScrolling::setKineticScrollValue(QPoint value)
{

    d->minimalPos.setY((-(d->widget->size().height())) + d->scrollingWidget->size().height()
                 -(d->overshoot));
    d->minimalPos.setX((-(d->widget->size().width())) + d->scrollingWidget->size().width());

    d->minimalPos.setY(qMin(d->overshoot, d->minimalPos.y()));
    d->minimalPos.setX(qMin( 0, d->minimalPos.x()));

    d->maximumPos = d->widget->pos().toPoint() - value;

    d->cposition.setY(qBound(d->minimalPos.y(), d->maximumPos.y(), d->overshoot));
    d->cposition.setX( qBound(d->minimalPos.x(), d->maximumPos.x(), 0));

    d->widget->setPos(d->cposition);

    if ((d->cposition.y() == d->overshoot) && d->hasOvershoot) {
        d->direction = KineticScrollingPrivate::Up;
        killTimer(d->timerID);
        d->mScrollVelocity = 5;
        startAnimationTimer(30);

    } else if ((d->cposition.y() == d->minimalPos.y()) && d->hasOvershoot) {
        d->direction = KineticScrollingPrivate::Down;
        killTimer(d->timerID);
        d->mScrollVelocity = 5;
        startAnimationTimer(30);

    } else
        d->direction = KineticScrollingPrivate::None;
}

void KineticScrolling::bounceTimer()
{
    int delta = 5;
    if ((d->direction == KineticScrollingPrivate::Up) && (d->bounceFlag == 0))
        delta = -5;
    else if ((d->direction == KineticScrollingPrivate::Up) && (d->bounceFlag == 1))
        delta = -2;
    else if ((d->direction == KineticScrollingPrivate::Down) && (d->bounceFlag == 0))
        delta = 5;
    else if ((d->direction == KineticScrollingPrivate::Down) && (d->bounceFlag == 1))
        delta = 2;

    d->cposition.setY(d->cposition.y() + delta);
    d->widget->setPos(d->cposition);

    if ((d->direction == KineticScrollingPrivate::Up) && (d->cposition.y() < 0)) {
        if (!d->bounceFlag) {
            d->cposition.setY(d->overshoot/4);
            d->bounceFlag = 1;
            d->widget->setPos(d->cposition);
            return;
        } else {
            d->bounceFlag = 0;
            d->cposition = QPoint(d->cposition.x(), 0);
            doneOvershoot() ;
        }

    } else if ((d->direction == KineticScrollingPrivate::Down) &&
                ((d->cposition.y() - d->overshoot) > d->minimalPos.y())) {
        if (!d->bounceFlag) {
            d->cposition.setY(d->minimalPos.y() + d->overshoot/4);
            d->bounceFlag = 1;
            d->widget->setPos(d->cposition);
            return;
        } else {
            d->bounceFlag = 0;
            d->cposition.setY(d->minimalPos.y() + d->overshoot);
            doneOvershoot();
        }
    }
}

void KineticScrolling::doneOvershoot(void)
{
    d->direction = KineticScrollingPrivate::None;
    d->mScrollVelocity = 0;
    killTimer(d->timerID);
    d->timerID = 0;
}

void KineticScrolling::setWidgets(QGraphicsWidget *widget,
                                  QGraphicsWidget *scrolling)
{
    d->widget = widget;
    d->scrollingWidget = scrolling;

    if(d->widget->size().height() <= d->scrollingWidget->size().height()) {
        d->hasOvershoot = false;
        d->overshoot = 0;	
    }

    d->widget->installEventFilter(this);
}

bool KineticScrolling::eventFilter(QObject *watched, QEvent *event)
{
    const int threashold = 10;
    if (!d->widget) {
        return false;
    }

    if (watched == d->widget && event->type() == QEvent::GraphicsSceneResize) {
        if(d->widget->size().height() <= d->scrollingWidget->size().height()+threashold) {
            d->hasOvershoot = false;
            d->overshoot = 0;
        } else {
            d->hasOvershoot = true;
            d->overshoot = 40;
        }
    }
    return false;
}

} // namespace Plasma

