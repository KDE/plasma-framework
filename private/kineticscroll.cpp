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
#include "kineticscroll_p.h"

/* TODO:
 * - write a factory to animation/state object
 * - move timer based code to an implementation of the factory
 * - write another implementation using QPropertyAnimation Qt 4.6
 * - generalize code concerning viewport and other parameters
 * - swap the 'magic numbers' for consts
 * - consider if direct access to control state/velocity variables should
 * be done directly or using functions
 * - implement horizontal scrolling (simultaneously with vertical)
 */

namespace Plasma
{

class KineticScrollingPrivate
{
public:
    KineticScrollingPrivate(): mScrollVelocity(0), timerID(0),
                               overshoot(40), bounceFlag(0)
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
    int movement;
    int kinMovement;

    qreal mScrollVelocity;
    enum { None, Up, Down };
    int timerID, overshoot, cposition, direction, minimalPos, maximumPos;
    char bounceFlag;

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

int KineticScrolling::movement()
{
    return d->movement;
}

int KineticScrolling::kinMovement()
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
    d->movement = 0;
    d->kinMovement = 0;
}

void KineticScrolling::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    int temp = event->lastPos().y() - event->pos().y();
    if (temp) {
        d->movement = temp;
        d->kinMovement += temp;
    }
    /* After */
    setKineticScrollValue(movement());
}

void KineticScrolling::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    d->timeDelta = d->elapsed();
    int temp = event->lastPos().y() - event->pos().y();
    if (temp)
        d->kinMovement += temp;

    if (d->timeDelta > 600) {
        if (d->kinMovement > 0)
            d->kinMovement = 3;
        else
            d->kinMovement = -3;
    }

    /* The after */
    if (d->cposition  > 0) {
        d->direction = KineticScrollingPrivate::Up;
    } else if (abs(d->cposition) >
               abs(d->scrollingWidget->size().height() - d->widget->size().height()))  {
        d->direction = KineticScrollingPrivate::Down;
    } else {
        d->direction = KineticScrollingPrivate::None;
        d->mScrollVelocity = kinMovement();
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
    d->kinMovement += temp;

    /* After */
    d->mScrollVelocity = kinMovement();
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
        setKineticScrollValue(qRound(d->mScrollVelocity));
    } else {
        if ((d->timerID != 0) && (d->direction != KineticScrollingPrivate::None)) {
            bounceTimer();
        }
    }
}

void KineticScrolling::setKineticScrollValue(int value)
{

    d->minimalPos = (-(d->widget->size().height())) + d->scrollingWidget->size().height()
                 -(d->overshoot);
    d->minimalPos = qMin(d->overshoot, d->minimalPos);
    d->maximumPos = d->widget->pos().y() - value;

    d->cposition = qBound(d->minimalPos, d->maximumPos, d->overshoot);
    d->widget->setPos(0, d->cposition);
    d->verticalScroll(-d->cposition/10);

    if (d->cposition == d->overshoot) {
        d->direction = KineticScrollingPrivate::Up;
        killTimer(d->timerID);
        d->mScrollVelocity = 5;
        startAnimationTimer(30);

    } else if (d->cposition == d->minimalPos) {
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

    d->cposition += delta;
    d->widget->setPos(0, d->cposition);

    if ((d->direction == KineticScrollingPrivate::Up) && (d->cposition < 0)) {
        if (!d->bounceFlag) {
            d->cposition = d->overshoot/4;
            d->bounceFlag = 1;
            d->widget->setPos(0, d->cposition);
            return;
        } else {
            d->bounceFlag = 0;
            d->cposition = 0;
            doneOvershoot() ;
        }

    } else if ((d->direction == KineticScrollingPrivate::Down) &&
                ((d->cposition - d->overshoot) > d->minimalPos)) {
        if (!d->bounceFlag) {
            d->cposition = d->minimalPos + d->overshoot/4;
            d->bounceFlag = 1;
            d->widget->setPos(0, d->cposition);
            return;
        } else {
            d->bounceFlag = 0;
            d->cposition = d->minimalPos + d->overshoot;
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
}

} // namespace Plasma

