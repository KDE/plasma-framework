/*
 *   Copyright 2009 Mehmet Ali Akmanalp <makmanalp@wpi.edu>
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
 * @file This file contains the definition for the Fade effect.
 */

#ifndef PLASMA_ANIMATIONS_FADE_P_H
#define PLASMA_ANIMATIONS_FADE_P_H

#include <plasma/animations/easinganimation_p.h>
#include <plasma/plasma_export.h>

namespace Plasma
{

/**
 * @class Fade plasma/animations/fade.h
 * @short Fade effect
 *
 * Effect that slowly transforms the opacity of the object from a starting
 * value to a target value. The range is 0 (full translucent) to 1 (full
 * opaque).
 */
class FadeAnimation : public EasingAnimation
{
    Q_OBJECT
    Q_PROPERTY(qreal startOpacity READ startOpacity WRITE setStartOpacity)
    Q_PROPERTY(qreal targetOpacity READ targetOpacity WRITE setTargetOpacity)

public:
    /** Default constructor */
    explicit FadeAnimation(QObject *parent = 0);

    /** Destructor */
    virtual ~FadeAnimation();

    /**
     * Access start opacity of the target widget.
     *
     * You can set both a start and an end opacity for an widget when
     * using this animation class. See \ref setStartOpacity.
     * @return The opacity (range is 0 to 1).
     */
    qreal startOpacity() const;
    /**
     * Set the start opacity of the target widget.
     *
     * See also \ref targetOpacity.
     * @param qreal The opacity (range is 0 to 1).
     */
    void setStartOpacity(qreal);

    /**
     * Access final opacity of the target widget.
     *
     * You can set both a start and an end opacity for an widget when
     * using this animation class. See \ref setTargetOpacity.
     * @return The opacity (range is 0 to 1).
     */
    qreal targetOpacity() const;
    /**
     * Set the final opacity of the target widget.
     *
     * See also \ref startOpacity.
     * @param qreal The opacity (range is 0 to 1).
     */
    void setTargetOpacity(qreal);

protected:
    void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState);
    void updateEffectiveTime(int currentTime);

private:
    /** Initial opacity */
    qreal m_startOpacity;
    /** Final opacity */
    qreal m_targetOpacity;
};

}

#endif // PLASMA_ANIMATIONS_FADE_P_H
