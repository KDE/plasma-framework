/*
 *   Copyright 2010 Aaron Seigo <aseigo@kde.org>
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

#ifndef PLASMA_EASINGANIMATION_H
#define PLASMA_EASINGANIMATION_H

#include "animation.h"

namespace Plasma
{


class EasingAnimation : public Animation
{
    Q_OBJECT

public:
    explicit EasingAnimation(QObject *parent = 0);

protected:
    virtual void updateEffectiveTime(int currentTime) = 0;

private:
    void updateCurrentTime(int currentTime);
};

} // namespace Plasma

#endif // PLASMA_EASINGANIMATION_H

