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

#include "fade.h"

#include <QRect>
#include <QtGui/QGraphicsOpacityEffect>

#include <kdebug.h>

namespace Plasma
{

FadeAnimation::FadeAnimation(QObject *parent)
             : Animation(parent),
               m_animFactor(0.5)
{
}

FadeAnimation::~FadeAnimation()
{
}

void FadeAnimation::setFactor(qreal factor)
{
    m_animFactor = qBound(qreal(0.0), factor, qreal(1.0));
}

qreal FadeAnimation::factor() const
{
    return m_animFactor;
}

void FadeAnimation::setWidgetToAnimate(QGraphicsWidget *widget)
{
    QGraphicsOpacityEffect *effect = m_opacityEffect.data();
    delete effect;

    Animation::setWidgetToAnimate(widget);

    if (widget) {
        effect = new QGraphicsOpacityEffect(widget);
        effect->setOpacity(qreal(1.0));
        widget->setGraphicsEffect(effect);
        m_opacityEffect = effect;
    }
}

QAbstractAnimation* FadeAnimation::render(QObject* parent)
{
    //create animation
    QPropertyAnimation* anim = new QPropertyAnimation(m_opacityEffect.data(), "opacity", parent);
    anim->setStartValue(qreal(1.0));
    anim->setEndValue(m_animFactor);
    anim->setDuration(duration());

    return anim;
}

} //namespace Plasma

