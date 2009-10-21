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

class FadeAnimationPrivate
{
public:
    FadeAnimationPrivate()
    {
    }

    ~FadeAnimationPrivate()
    {
    }

    QGraphicsOpacityEffect *opacityEffect;
};

FadeAnimation::FadeAnimation(qreal factor)
             : d(new FadeAnimationPrivate()),
               m_animFactor(qBound(qreal(0.0), factor, qreal(1.0)))
{
}

FadeAnimation::~FadeAnimation()
{
    delete d;
}

void FadeAnimation::setWidgetToAnimate(QGraphicsWidget *widget)
{
    QGraphicsWidget *m_widget = widgetToAnimate();
    if (m_widget) {
        QGraphicsEffect *effect = m_widget->graphicsEffect();
        if (effect && dynamic_cast<QGraphicsOpacityEffect *>(effect)) {
            effect->deleteLater();
        }
    }

    Animation::setWidgetToAnimate(widget);

    d->opacityEffect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(d->opacityEffect);
}

QAbstractAnimation* FadeAnimation::render(QObject* parent)
{

    //create animation
    QPropertyAnimation* anim = new QPropertyAnimation(d->opacityEffect, "opacity", parent);
    anim->setEndValue(m_animFactor);
    anim->setDuration(duration());

    //QObject::connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));

    return anim;
}

} //namespace Plasma

