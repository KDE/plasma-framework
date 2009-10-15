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
 * @file This file contains the definition for the Expand effect.
 */

#ifndef PLASMA_ANIMATIONS_Expand_H
#define PLASMA_ANIMATIONS_Expand_H

#include <plasma/animations/animation.h>
#include <plasma/plasma_export.h>

namespace Plasma
{

/**
 * @class Expand plasma/animations/expand.h
 * @short Expand effect
 * 
 * Effect that grows the object a specific distance in a given direction, on
 * one side. The object doesn't "move" except for the side that is expanding.
 */
class PLASMA_EXPORT ExpandAnimation : public Animation
{
    Q_OBJECT

public:
    ExpandAnimation(AnimationDirection direction, int distance);
    virtual ~ExpandAnimation(){};

protected:
    virtual QAbstractAnimation* render(QObject* parent = 0);

private:
    AnimationDirection m_direction;
    int m_distance;

};

}

#endif
