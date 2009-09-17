/***********************************************************************/
/* kineticscroll.h 						       */
/* 								       */
/* Copyright(C) 2009 Igor Trindade Oliveira <igor.oliveira@indt.org.br>*/
/* Copyright(C) 2009 Adenilson Cavalcanti <adenilson.silva@idnt.org.br>*/
/* 								       */
/* This library is free software; you can redistribute it and/or       */
/* modify it under the terms of the GNU Lesser General Public	       */
/* License as published by the Free Software Foundation; either	       */
/* version 2.1 of the License, or (at your option) any later version.  */
/*   								       */
/* This library is distributed in the hope that it will be useful,     */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of      */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   */
/* Lesser General Public License for more details.		       */
/*  								       */
/* You should have received a copy of the GNU Lesser General Public    */
/* License along with this library; if not, write to the Free Software */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA       */
/* 02110-1301  USA						       */
/***********************************************************************/
#ifndef PLASMA_KINETICSCROLLING_H
#define PLASMA_KINETICSCROLLING_H

class QGraphicsSceneMouseEvent;
class QGraphicsWidget;
#include <QObject>

namespace Plasma
{

class KineticScrollingPrivate;

class KineticScrolling: public QObject
{
    public:
        KineticScrolling();
	~KineticScrolling();
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        void wheelReleaseEvent(QGraphicsSceneWheelEvent *event);
        void setWidgets(QGraphicsWidget *widget, QGraphicsWidget *scrolling);

    private:
        KineticScrollingPrivate *d;
        void timerEvent(QTimerEvent *event);
        void bounceTimer();
        qreal duration();
        int movement();
	int kinMovement();
        void startAnimationTimer(int interval);
        void doneOvershut(void);

public Q_SLOTS:
    void setKineticScrollValue(int value);

};

}

#endif
