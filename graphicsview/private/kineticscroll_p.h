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

#ifndef PLASMA_KINETICSCROLLING_H
#define PLASMA_KINETICSCROLLING_H

class QGraphicsSceneMouseEvent;
class QGraphicsWidget;
class QGraphicsSceneWheelEvent;
class QKeyEvent;
#include <QObject>
#include <QPointF>
#include <QtCore/QAbstractAnimation>


namespace Plasma
{

class KineticScrollingPrivate;

class KineticScrolling: public QObject
{
    Q_OBJECT
public:
    KineticScrolling(QGraphicsWidget *parent);
    ~KineticScrolling();
    void setWidget(QGraphicsWidget *parent);
    void stop();

Q_SIGNALS:
    void stateChanged(QAbstractAnimation::State newState,
            QAbstractAnimation::State oldState);

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void wheelReleaseEvent(QGraphicsSceneWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    KineticScrollingPrivate *d;
    void resetAnimation(QPointF finalPosition, int duration);
    void setScrollValue(QPointF value);
    QPointF thresholdPosition(QPointF value) const;
    void duration();

private Q_SLOTS:
    void overshoot();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

};

}

#endif
