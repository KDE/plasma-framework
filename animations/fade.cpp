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

#include "fade_p.h"

#include <QRect>

#include <kdebug.h>

namespace Plasma
{

FadeAnimation::FadeAnimation(QObject *parent)
             : Animation(parent),
               m_startOpacity(0),
               m_targetOpacity(1)
{
}

FadeAnimation::~FadeAnimation()
{
}

void FadeAnimation::setStartOpacity(qreal factor)
{
    m_startOpacity = qBound(qreal(0.0), factor, qreal(1.0));
}

qreal FadeAnimation::startOpacity() const
{
    return m_startOpacity;
}

void FadeAnimation::setTargetOpacity(qreal factor)
{
    m_targetOpacity = qBound(qreal(0.0), factor, qreal(1.0));
}

qreal FadeAnimation::targetOpacity() const
{
    return m_targetOpacity;
}

void FadeAnimation::setWidgetToAnimate(QGraphicsWidget *widget)
{
    if (widget == widgetToAnimate()) {
        return;
    }

    Animation::setWidgetToAnimate(widget);
    if (widget) {
        //widget->setOpacity(m_startOpacity);
    }

    if(m_anim.data()) {
        delete m_anim.data();
        m_anim.clear();
    }

    QPropertyAnimation *anim = new QPropertyAnimation(widget, "opacity", widget);
    anim->setStartValue(startOpacity());
    anim->setEndValue(targetOpacity());

    m_anim = anim;
}

void FadeAnimation::updateState(QAbstractAnimation::State oldState, QAbstractAnimation::State newState)
{
    QGraphicsWidget *w = widgetToAnimate();
    if (!w) {
        w->setOpacity(startOpacity());
        return;
    }

    if( (oldState == QAbstractAnimation::Stopped) &&
        (newState == QAbstractAnimation::Running)) {
        QPropertyAnimation *anim = m_anim.data();

        anim->setDirection(direction());
        anim->setStartValue(startOpacity());
        anim->setEndValue(targetOpacity());
        anim->start();
    }
}

void FadeAnimation::updateCurrentTime(int currentTime)
{
    m_anim.data()->setCurrentTime(currentTime);

    Animation::updateCurrentTime(currentTime);
}

} //namespace Plasma

