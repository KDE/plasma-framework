/* Copyright (C)  2009  Adenilson Cavalcanti <cavalcantii@gmail.com>
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

#ifndef PLASMA_ANIMATIONS_PULSE_H
#define PLASMA_ANIMATIONS_PULSE_H

#include <plasma/animations/animation.h>
#include <plasma/plasma_export.h>

namespace Plasma
{

class PulseAnimationPrivate;

class PulseAnimation : public Animation
{
    Q_OBJECT

public:
    PulseAnimation(QObject *parent = 0);
    ~PulseAnimation();

    void updateGeometry(QRectF updated, qreal zCoordinate = 0, qreal scale = 1.5);
    void setWidgetToAnimate(QGraphicsWidget *widget);

public Q_SLOTS:
    void resetPulser();

protected:
    virtual QAbstractAnimation* render(QObject* parent = 0);
    void setCopy();

private:

    void createAnimation(qreal _duration = 500, qreal _scale = 1.5);

    PulseAnimationPrivate *d;
};

}

#endif






