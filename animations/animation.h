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
 * @file This file contains the abstract base class for all singular
 * animations.
 */

#ifndef PLASMA_ANIMATION_H
#define PLASMA_ANIMATION_H

#include <QGraphicsWidget>
#include <QObject>
#include <QPropertyAnimation>
#include <QAbstractAnimation>
#include <plasma/plasma_export.h>
#include <plasma/plasma.h>

namespace Plasma
{

class AnimationPrivate;

/**
 * @brief Abstract representation of a single animation.
 * @since 4.4
 */
class PLASMA_EXPORT Animation : public QAbstractAnimation
{

    Q_OBJECT
    Q_PROPERTY(int duration READ duration WRITE setDuration)
    Q_PROPERTY(QEasingCurve::Type easingCurveType READ easingCurveType WRITE setEasingCurveType)
    Q_PROPERTY(QGraphicsWidget *widgetToAnimate READ widgetToAnimate WRITE setWidgetToAnimate)

public:

    QAbstractAnimation::Direction direction() const;

    /**
     * Get the animation duration.
     * @return duration in ms.
     */
    int duration() const;

    /* FIXME: find a better place and name for it. */
    enum Reference{
        Center,
        Up,
        Down,
        Left,
        Right
    };

    explicit Animation(QObject* parent = 0);
    virtual ~Animation() = 0;

    /**
     * Set the widget on which the animation is to be performed.
     * @arg receiver The QGraphicsWidget to be animated.
     */
    virtual void setWidgetToAnimate(QGraphicsWidget* receiver);

    /**
     * The widget that the animation will be performed upon
     */
    QGraphicsWidget *widgetToAnimate();

    /**
     * Set the animation easing curve type
     */
    void setEasingCurveType(QEasingCurve::Type easingCurve);

    /**
     * Get the animation easing curve type
     */
    QEasingCurve::Type easingCurveType() const;

public Q_SLOTS:
    void start(QAbstractAnimation::DeletionPolicy policy = KeepWhenStopped);
    /**
     * Each individual class must override this function to place their main
     * functionality. This function must take the values from the constructor,
     * do the appropriate calculations, and return a corresponding
     * QPropertyAnimation initialized with the given parent.
     */
    virtual QAbstractAnimation* render(QObject* parent = 0) = 0;



protected:

    /**
     * Change the animation duration. Default is 1000ms.
     * @arg duration The new duration of the animation.
     */
    virtual void setDuration(int duration = 250);

    void updateDirection(QAbstractAnimation::Direction direction);

    void updateCurrentTime(int currentTime);

private:
    AnimationPrivate *const d;

};

} //namespace Plasma

#endif
