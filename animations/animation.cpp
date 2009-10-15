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

Animation::Animation(QObject* parent)
    : m_parent(parent),
      m_duration(250)
{
}

Animation::~Animation(){}

void Animation::setDuration(int duration){
    m_duration = duration;
}

int Animation::duration() const {
    return m_duration;
}

void Animation::start(){
    QAbstractAnimation* anim = getQtAnimation(m_parent);
    if (anim){
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

QAbstractAnimation* Animation::getQtAnimation(QObject* parent){

    //check if .setObject() was done
    if (!getAnimatedObject()){
        kDebug() << "Object not set.";
        return NULL;
    }

    //check which parent to use
    if (parent){
        return render(parent);
    } else {
        return render(m_parent);
    }

}

int Animation::getDuration() {

    return m_duration;

}

} //namespace Plasma

#include <../animation.moc>
