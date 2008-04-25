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

#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <QtGui/QImage>
#include <QtCore/QObject>

#include <plasma/plasma_export.h>

class QGraphicsItem;
class QTimeLine;

namespace Plasma
{

/**
 * @short A system for applying effects to Plasma elements
 */
class PLASMA_EXPORT Animator : public QObject
{
    Q_OBJECT
    Q_ENUMS( Animation )
    Q_ENUMS( CurveShape )
    Q_ENUMS( Movement )

public:

    enum Animation
    {
        AppearAnimation = 0 /*<< Animate the appearance of an element */,
        DisappearAnimation /*<< Animate the disappearance of an element */,
        ActivateAnimation /*<< When something is activated or launched, such as an app icon being clicked */
    };

    enum CurveShape
    {
        EaseInCurve = 0,
        EaseOutCurve,
        EaseInOutCurve,
        LinearCurve
    };

    enum Movement
    {
        SlideInMovement = 0,
        SlideOutMovement,
        FastSlideInMovement,
        FastSlideOutMovement
    };

    /**
     * Singleton accessor
     **/
    static Animator* self();

    explicit Animator(QObject * parent = 0);
    ~Animator();

    Q_INVOKABLE void animateItem(QGraphicsItem* item, Animation anim);
    Q_INVOKABLE void moveItem(QGraphicsItem* item, Movement movement, const QPoint &destination);

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
     *
     * @return an id that can be used to identify this animation.
     */
    Q_INVOKABLE int customAnimation(int frames, int duration, Animator::CurveShape curve,
                                       QObject* receiver, const char* method);

    /**
     * Stops a custom animation. Note that it is not necessary to call
     * this on object destruction, as custom animations associated with
     * a given QObject are cleaned up automatically on QObject destruction.
     *
     * @arg id the id of the animation as returned by customAnimation
     */
    Q_INVOKABLE void stopCustomAnimation(int id);

    Q_INVOKABLE int animateElement(QGraphicsItem *obj, Animation);
    Q_INVOKABLE void stopElementAnimation(int id);
    Q_INVOKABLE void setInitialPixmap(int id, const QPixmap &pixmap);
    Q_INVOKABLE QPixmap currentPixmap(int id);

    /**
     * Can be used to query if there are other animations happening. This way
     * heavy operations can be delayed until all animations are finished.
     * @return true if there are animations going on.
     * @since 4.1
     */
    Q_INVOKABLE bool isAnimating() const;

Q_SIGNALS:
    void animationFinished(QGraphicsItem *item, Plasma::Animator::Animation anim);
    void movementFinished(QGraphicsItem *item);
    void elementAnimationFinished(int id);
    void customAnimationFinished(int id);

protected:
    void timerEvent(QTimerEvent *event);

protected Q_SLOTS:
    void animatedItemDestroyed(QObject*);
    void movingItemDestroyed(QObject*);
    void animatedElementDestroyed(QObject*);
    void customAnimReceiverDestroyed(QObject*);

private:
    void init();

    class Private;
    Private * const d;
};

} // namespace Plasma

#endif

