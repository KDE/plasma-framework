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

#include "easinganimation_p.h"
#include <kdebug.h>

namespace Plasma
{

EasingAnimation::EasingAnimation(QObject *parent)
    : Animation(parent)
{
}

void EasingAnimation::updateCurrentTime(int currentTime)
{
    updateEffectiveTime(easingCurve().valueForProgress(currentTime / qreal(qMax(1, duration()))) * duration());
}

} // namespace Plasma

#include "easinganimation_p.moc"

