/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *                 2007 Alexis Ménard <darktears31@gmail.com>
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

#ifndef PLASMA_ANIMATOR_H
#define PLASMA_ANIMATOR_H

#include <QtGui/QImage>
#include <QtCore/QObject>
#include <QtCore/QAbstractAnimation>
#include <QtCore/QEasingCurve>

#include <plasma/plasma_export.h>

class QGraphicsItem;
class QGraphicsWidget;
class QTimeLine;

namespace Plasma
{

class AnimatorPrivate;
class Animation;

/**
 * @class Animator plasma/animator.h <Plasma/Animator>
 *
 * @short A system for applying effects to Plasma elements
 */
class PLASMA_EXPORT Animator : public QObject
{
    Q_OBJECT
    Q_ENUMS(Animation)
    Q_ENUMS(CurveShape)

public:
    enum Animation {
        AppearAnimation = 0, /*<< Animate the appearance of an element */
        DisappearAnimation,  /*<< Animate the disappearance of an element */
        ActivateAnimation,    /*<< When something is activated or launched,
                                such as an app icon being clicked */
        FadeAnimation, /*<< Can be used for both fade in and out */
        GrowAnimation, /*<< Grow animated object geometry */
        PulseAnimation, /*<< Pulse animated object (opacity/geometry/scale) */
        RotationAnimation, /*<< Rotate an animated object */
        RotationStackedAnimation, /*<< for flipping one object with another */
        SlideAnimation, /*<< Move the position of animated object */
        GeometryAnimation, /*<< Geometry animation*/
        ZoomAnimation, /*<<Zoom animation */
        PixmapTransitionAnimation, /*<< Transition between two pixmaps*/
        WaterAnimation /*<< Water animation using ripple effect */,
        LastAnimation = 1024
    };

    enum CurveShape {
        EaseInCurve = 0,
        EaseOutCurve,
        EaseInOutCurve,
        LinearCurve,
        PendularCurve
    };

    explicit Animator(QObject *parent = 0);

    /**
     * Factory to build new animation objects. To control their behavior,
     * check \ref AbstractAnimation properties.
     * @since 4.4
     **/
    static Plasma::Animation *create(Animator::Animation type, QObject *parent = 0);

    /**
     * Factory to build new animation objects from Javascript files. To control their behavior,
     * check \ref AbstractAnimation properties.
     * @since 4.5
     **/
    static Plasma::Animation *create(const QString &animationName, QObject *parent = 0);

    /**
     * Factory to build new custom easing curves.
     * @since 4.5
     */
    static QEasingCurve create(Animator::CurveShape type);

private:
    AnimatorPrivate * const d;
};

} // namespace Plasma

#endif

