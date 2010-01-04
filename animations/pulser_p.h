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

/**
 * @file This file contains the definition for the Pulse effect.
 */

#ifndef PLASMA_ANIMATIONS_PULSER_P_H
#define PLASMA_ANIMATIONS_PULSER_P_H

#include <plasma/animations/animation.h>
#include <plasma/plasma_export.h>

namespace Plasma
{

class ShadowFake;

/**
 * @class PulseAnimation plasma/animations/pulser_p.h
 * @short Pulse effect
 *
 * Effect that pulses a shadow copy of any QGraphicsWidget making
 * it more translucent and bigger along the time until it vanishes.
 */
class PulseAnimation : public Animation
{
    Q_OBJECT

public:
    /** Default Constructor */
    PulseAnimation(QObject *parent = 0);
    /** Destructor */
    ~PulseAnimation();

public Q_SLOTS:
    /**
     * Resets the shadow widget to its initial state (full translucent
     * and with same geometry as the target widget). It is executed
     * when the animation is over.
     */
    void resetPulser();

protected:
    void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);
    void updateCurrentTime(int currentTime);
    void setCopy();

private:
    /** Zvalue (tipically -1 than the target widget) */
    qreal m_zvalue;

    /** Original widget scale */
    qreal m_scale;

    /** Opacity of shadow widget (full translucent) */
    qreal m_opacity;

    /** Target scale of shadow widget (default is 1.5x the animated
     * widget scale).
     */
    qreal m_endScale;

    /** The shadow copy (it really is a QGraphicsWidget with a pixmap
     * copy of the original widget).
     */
    ShadowFake *m_under;
};

}

#endif // PLASMA_ANIMATIONS_PULSER_P_H






