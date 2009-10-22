/*
 *   Copyright 2009 Aaron J. Seigo <aseigo@kde.org>
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

#ifndef PLASMA_ANIMATIONS_PAUSE_H
#define PLASMA_ANIMATIONS_PAUSE_H

#include <plasma/animations/animation.h>
#include <plasma/plasma_export.h>

namespace Plasma
{

/**
 * @class PauseAnimation
 * @short Provides a pause in animations
 */
class PauseAnimation : public Animation
{
    Q_OBJECT

public:
    PauseAnimation(QObject *parent = 0);

protected:
    virtual QAbstractAnimation* render(QObject* parent = 0);
};

}

#endif
