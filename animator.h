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
    Q_ENUMS(Movement)

public:

    enum Animation {
        /* TODO: should we map older animations to new ones? */
        AppearAnimation = 0, /*<< Animate the appearance of an element */
        DisappearAnimation,  /*<< Animate the disappearance of an element */
        ActivateAnimation,    /*<< When something is activated or launched,
                                such as an app icon being clicked */
        /* TODO: change the names of animation classes */
        FadeAnimation, /*<< Can be used for both fade in and out */
        GrowAnimation, /*<< Grow animated object geometry */
        PulseAnimation, /*<< Pulse animated object (opacity/geometry/scale) */
        RotationAnimation, /*<< Rotate an animated object */
        RotationStackedAnimation, /*<< for flipping one object with another */
        SlideAnimation, /*<< Move the position of animated object */
        GeometryAnimation, /*<< Geometry animation*/
        ZoomAnimation /*<<Zoom animation */
    };

    enum CurveShape {
        EaseInCurve = 0,
        EaseOutCurve,
        EaseInOutCurve,
        LinearCurve
    };

    enum Movement {
        SlideInMovement = 0,
        SlideOutMovement,
        FastSlideInMovement,
        FastSlideOutMovement
    };

    /**
     * Singleton accessor
     **/
    static KDE_DEPRECATED Animator *self();

    /**
     * Factory to build new animation objects. To control their behavior,
     * check \ref AbstractAnimation properties.
     **/
    static Plasma::Animation *create(Animator::Animation type, QObject *parent = 0);

    /**
     * Starts a standard animation on a QGraphicsItem.
     *
     * @arg item the item to animate in some fashion
     * @arg anim the type of animation to perform
     * @return the id of the animation
     * @deprecated use new Animator API with Qt Kinetic
     **/
    KDE_DEPRECATED Q_INVOKABLE int animateItem(QGraphicsItem *item,Animation anim);

    /**
     * Stops an item animation before the animation is complete.
     * Note that it is not necessary to call
     * this on normal completion of the animation.
     *
     * @arg id the id of the animation as returned by animateItem
     * @deprecated use new Animator API with Qt Kinetic
     */
    KDE_DEPRECATED Q_INVOKABLE void stopItemAnimation(int id);

    /**
     * Starts a standard animation on a QGraphicsItem.
     *
     * @arg item the item to animate in some fashion
     * @arg anim the type of animation to perform
     * @return the id of the animation
     * @deprecated use new Animator API with Qt Kinetic
     **/
    KDE_DEPRECATED Q_INVOKABLE int moveItem(QGraphicsItem *item, Movement movement, const QPoint &destination);

    /**
     * Stops an item movement before the animation is complete.
     * Note that it is not necessary to call
     * this on normal completion of the animation.
     *
     * @arg id the id of the animation as returned by moveItem
     * @deprecated use new Animator API with Qt Kinetic
     */
    KDE_DEPRECATED Q_INVOKABLE void stopItemMovement(int id);

    /**
     * Starts a custom animation, preventing the need to create a timeline
     * with its own timer tick.
     *
     * @arg frames the number of frames this animation should persist for
     * @arg duration the length, in milliseconds, the animation will take
     * @arg curve the curve applied to the frame rate
     * @arg receive the object that will handle the actual animation
     * @arg method the method name of slot to be invoked on each update.
     *             It must take a qreal. So if the slot is animate(qreal),
     *             pass in "animate" as the method parameter.
     *             It has an optional integer paramenter that takes an
     *             integer that reapresents the animation id, useful if
     *             you want to manage multiple animations with a sigle slot
     *
     * @return an id that can be used to identify this animation.
     * @deprecated use new Animator API with Qt Kinetic
     */
    KDE_DEPRECATED Q_INVOKABLE int customAnimation(int frames, int duration,
        Animator::CurveShape curve, QObject *receiver, const char *method);

    /**
     * Stops a custom animation. Note that it is not necessary to call
     * this on object destruction, as custom animations associated with
     * a given QObject are cleaned up automatically on QObject destruction.
     *
     * @arg id the id of the animation as returned by customAnimation
     * @deprecated use new Animator API with Qt Kinetic
     */
    KDE_DEPRECATED Q_INVOKABLE void stopCustomAnimation(int id);

    KDE_DEPRECATED Q_INVOKABLE int animateElement(QGraphicsItem *obj, Animation);
    KDE_DEPRECATED Q_INVOKABLE void stopElementAnimation(int id);
    KDE_DEPRECATED Q_INVOKABLE void setInitialPixmap(int id, const QPixmap &pixmap);
    KDE_DEPRECATED Q_INVOKABLE QPixmap currentPixmap(int id);

    /**
     * Can be used to query if there are other animations happening. This way
     * heavy operations can be delayed until all animations are finished.
     * @return true if there are animations going on.
     * @since 4.1
     * @deprecated use new Animator API with Qt Kinetic
     */
    KDE_DEPRECATED Q_INVOKABLE bool isAnimating() const;

    /**
     * Register a widget as a scrolling widget.
     * The widget will get animate scrolling with mouse dragging and mouse wheel.
     * It must provide
     * scrollValue, viewportGeometry and pageSize properties
     *
     * @param widget the widget that offers a scrolling behaviour
     * @since 4.4
     */
    void registerScrollingManager(QGraphicsWidget *widget);

    /**
     * unregister the scrolling manager of a certain widget
     *
     * @param widget the widget we don't want no longer animated
     * @since 4.4
     */
    void unregisterScrollingManager(QGraphicsWidget *widget);

Q_SIGNALS:
    void animationFinished(QGraphicsItem *item, Plasma::Animator::Animation anim);
    void movementFinished(QGraphicsItem *item);
    void elementAnimationFinished(int id);
    void customAnimationFinished(int id);
    void scrollStateChanged(QGraphicsWidget *widget, QAbstractAnimation::State newState,
            QAbstractAnimation::State oldState);

protected:
    void timerEvent(QTimerEvent *event);

private:
    friend class AnimatorSingleton;
    explicit Animator(QObject * parent = 0);
    ~Animator();

    Q_PRIVATE_SLOT(d, void animatedItemDestroyed(QObject*))
    Q_PRIVATE_SLOT(d, void movingItemDestroyed(QObject*))
    Q_PRIVATE_SLOT(d, void animatedElementDestroyed(QObject*))
    Q_PRIVATE_SLOT(d, void customAnimReceiverDestroyed(QObject*))
    Q_PRIVATE_SLOT(d, void scrollStateChanged(QAbstractAnimation::State,
                QAbstractAnimation::State))

    friend class AnimatorPrivate;
    AnimatorPrivate * const d;
};

} // namespace Plasma

#endif

