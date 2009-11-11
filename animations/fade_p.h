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
 * @file This file contains the definition for the Fade effect.
 */

#ifndef PLASMA_ANIMATIONS_FADE_H
#define PLASMA_ANIMATIONS_FADE_H

#include <plasma/animations/animation.h>
#include <plasma/plasma_export.h>

namespace Plasma
{

class FadeAnimationPrivate;

/**
 * @class Fade plasma/animations/fade.h
 * @short Fade effect
 *
 * Effect that slowly transforms the opacity of the object to the given value.
 */
class FadeAnimation : public Animation
{
    Q_OBJECT
    Q_PROPERTY(qreal startOpacity READ startOpacity WRITE setStartOpacity)
    Q_PROPERTY(qreal targetOpacity READ targetOpacity WRITE setTargetOpacity)

public:
    FadeAnimation(QObject *parent = 0);
    virtual ~FadeAnimation();

    qreal startOpacity() const;
    void setStartOpacity(qreal);

    qreal targetOpacity() const;
    void setTargetOpacity(qreal);

    void setWidgetToAnimate(QGraphicsWidget *widget);

protected:
    virtual QAbstractAnimation* render(QObject* parent = 0);

private:
    QWeakPointer<QGraphicsWidget> m_widget;
    qreal m_startOpacity;
    qreal m_targetOpacity;
};

}

#endif
