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
     * Parent owner object to use in generated animations.
     */
    QObject* m_parent;

    /**
     * Duration of the animation. Default is 1000ms.
     */
    int m_duration;

};

Animation::Animation(QObject* parent)
    : d(new AnimationPrivate)
{
    d->m_parent = parent;
    d->m_duration = 250;

}

Animation::~Animation()
{
    delete d;
}

void Animation::setDuration(int duration)
{
    d->m_duration = duration;
}

void Animation::start()
{
    QAbstractAnimation* anim = toQAbstractAnimation(d->m_parent);
    if (anim) {
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

QAbstractAnimation* Animation::toQAbstractAnimation(QObject* parent)
{
    //check if .setObject() was done
    if (!widgetToAnimate()) {
        kDebug() << "Object not set.";
        return NULL;
    }

    //check which parent to use
    if (parent) {
        return render(parent);
    } else {
        return render(d->m_parent);
    }

}

int Animation::duration() const
{
    return d->m_duration;
}

} //namespace Plasma

#include <../animation.moc>
