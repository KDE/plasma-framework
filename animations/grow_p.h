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
 * @file This file contains the definition for the Grow effect.
 */

#ifndef PLASMA_ANIMATIONS_GROW_H
#define PLASMA_ANIMATIONS_GROW_H

#include <plasma/animations/animation.h>
#include <plasma/plasma_export.h>

namespace Plasma
{

/**
 * @class Grow plasma/animations/grow.h
 * @short Grow effect
 *
 * Effect that grows any QGraphicsWidget by a multiple given in the
 * constructor. The center of the object stays in place while the sides grow.
 *
 */
class GrowAnimation : public Animation
{
    Q_OBJECT
    Q_PROPERTY(qreal factor READ factor WRITE setFactor)

public:

    GrowAnimation(qreal factor = 2);
    virtual ~GrowAnimation(){};

    qreal factor() const;

    void setFactor(const qreal factor);


protected:
    virtual QAbstractAnimation* render(QObject* parent = 0);

private:
    qreal m_animFactor;
};

}

#endif
