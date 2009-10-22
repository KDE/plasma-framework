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

#include "animation.h"

#include <QMapIterator>
#include <QObject>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>

#include <kdebug.h>
#include <kglobalsettings.h>

namespace Plasma
{

class AnimationPrivate {

public:
    /**
     * Duration of the animation. Default is 250ms.
     */
    int duration;
};

Animation::Animation(QObject* parent)
    : AbstractAnimation(parent),
      d(new AnimationPrivate)
{
    d->duration = 250;
}

Animation::~Animation()
{
    delete d;
}

void Animation::setDuration(int duration)
{
    d->duration = duration;
}

QAbstractAnimation* Animation::toQAbstractAnimation(QObject* parent)
{
    //check if the widget to animate was set already
    if (!widgetToAnimate()) {
        kDebug() << "Object not set.";
        return NULL;
    }

    //check which parent to use
    if (parent) {
        return render(parent);
    } else {
        return render(this->parent());
    }
}

int Animation::duration() const
{
    return d->duration;
}

} //namespace Plasma

#include <../animation.moc>
