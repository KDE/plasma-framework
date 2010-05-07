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

#include "animator.h"
#include "private/animator_p.h"

#include <QGraphicsItem>
#include <QTimeLine>
#include <QTimerEvent>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kglobalsettings.h>

#include "animationdriver.h"
#include "private/kineticscroll_p.h"

namespace Plasma
{

static const int MIN_TICK_RATE_INT = 10;
static const qreal MIN_TICK_RATE = 10;

AnimatorPrivate::AnimatorPrivate(Animator *parent)
    : q(parent),
      driver(0),
      animId(0),
      timerId(0)
{
}

AnimatorPrivate::~AnimatorPrivate()
{
    cleanupStates();
    qDeleteAll(animatedItems);
    qDeleteAll(animatedElements);
    qDeleteAll(movingItems);

    QMutableHashIterator<int, CustomAnimationState*> it(customAnims);
    while (it.hasNext()) {
        it.next();
        delete[] it.value()->slot;
        delete it.value();
        it.remove();
    }

    // Animator is a QObject
    // and we don't own the items
}

qreal AnimatorPrivate::calculateProgress(int time, int duration, Animator::CurveShape curve)
{
    if (!(KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects)) {
        return qreal(1.0);
    }

    timeline.setCurveShape(static_cast<QTimeLine::CurveShape>(curve));
    timeline.setDuration(duration);
    qreal progress = timeline.valueForTime(time);
    return progress;
}

void AnimatorPrivate::performAnimation(qreal amount, const AnimationState *state)
{
    /* TODO: write new animations to replace this.
     */
    switch (state->animation) {
        case Animator::AppearAnimation:
            driver->itemAppear(amount, state->item);
            break;
        case Animator::DisappearAnimation:
            driver->itemDisappear(amount, state->item);
            if (amount >= 1) {
                state->item->hide();
            }
            break;
        case Animator::ActivateAnimation:
            driver->itemActivated(amount, state->item);
            break;
        default:
            kDebug() << "Unsupported animation type.";

    }
}

void AnimatorPrivate::performMovement(qreal amount, const MovementState *state)
{
    switch (state->movement) {
        case Animator::SlideInMovement:
        case Animator::FastSlideInMovement:
            //kDebug() << "performMovement, SlideInMovement";
            driver->itemSlideIn(amount, state->item, state->start, state->destination);
            break;
        case Animator::SlideOutMovement:
        case Animator::FastSlideOutMovement:
            //kDebug() << "performMovement, SlideOutMovement";
            driver->itemSlideOut(amount, state->item, state->start, state->destination);
            break;
    }
}

void AnimatorPrivate::scrollStateChanged(QAbstractAnimation::State newState,
        QAbstractAnimation::State oldState)
{
    KineticScrolling *scroll = qobject_cast<KineticScrolling*>(q->sender());
    if (!scroll) {
        kDebug() << "Could not find KineticScrolling object";
        return;
    }

    emit q->scrollStateChanged(scrollingManagers.key(scroll), newState, oldState);
}

class AnimatorSingleton
{
    public:
        Animator self;
};

K_GLOBAL_STATIC(AnimatorSingleton, privateSelf)

Animator *Animator::self()
{
    return &privateSelf->self;
}

Animator::Animator(QObject *parent)
    : QObject(parent),
      d(new AnimatorPrivate(this))
{
    d->init(this);
}

Animator::~Animator()
{
    delete d;
}

void AnimatorPrivate::animatedItemDestroyed(QObject *o)
{
    //kDebug() << "testing for" << (void*)o;
    QMutableHashIterator<QGraphicsItem*, AnimationState*> it(animatedItems);
    while (it.hasNext()) {
        it.next();
        //kDebug() << "comparing against" << it.value()->qobj;
        if (it.value()->qobj == o) {
            kDebug() << "found deleted animated item";
            if (timerId) {
                animatedItemsToDelete.insert(it.value());
            } else {
                delete it.value();
            }

            it.remove();
        }
    }
}

void AnimatorPrivate::movingItemDestroyed(QObject *o)
{
    QMutableHashIterator<QGraphicsItem*, MovementState*> it(movingItems);
    while (it.hasNext()) {
        it.next();
        if (it.value()->qobj == o) {
            if (timerId) {
                movingItemsToDelete.insert(it.value());
            } else {
                delete it.value();
            }

            it.remove();
        }
    }
}

void AnimatorPrivate::animatedElementDestroyed(QObject *o)
{
    QMutableHashIterator<int, ElementAnimationState*> it(animatedElements);
    while (it.hasNext()) {
        it.next();
        if (it.value()->qobj == o) {
            if (timerId) {
                animatedElementsToDelete.insert(it.value());
            } else {
                delete it.value();
            }

            it.remove();
        }
    }
}

void AnimatorPrivate::customAnimReceiverDestroyed(QObject *o)
{
    QMutableHashIterator<int, CustomAnimationState*> it(customAnims);
    while (it.hasNext()) {
        if (it.next().value()->receiver == o) {
            if (timerId) {
                customAnimsToDelete.insert(it.value());
            } else {
                delete[] it.value()->slot;
                delete it.value();
            }

            it.remove();
        }
    }
}

int Animator::animateItem(QGraphicsItem *item, Animation animation)
{
     //kDebug();
    // get rid of any existing animations on this item.
    QHash<QGraphicsItem*, AnimationState*>::iterator it = d->animatedItems.find(item);
    if (it != d->animatedItems.end()) {
        if (d->timerId) {
            d->animatedItemsToDelete.insert(it.value());
        } else {
            delete it.value();
        }

        d->animatedItems.erase(it);
    }

    int frames = d->driver->animationFps(animation);

    if (frames < 1) {
        // evidently this animator doesn't have an implementation
        // for this Animation
        return -1;
    }

    int duration = d->driver->animationDuration(animation);

    AnimationState *state = new AnimationState;
    state->id = ++d->animId;
    state->item = item;
    state->animation = animation;
    state->curve = d->driver->animationCurve(animation);
    state->frames = qMax(1.0, frames * (duration / 1000.0));
    state->currentFrame = 0;
    state->interval = d->driver->animationDuration(animation) / qreal(state->frames);
    state->interval = qMax(MIN_TICK_RATE_INT, state->interval - (state->interval % MIN_TICK_RATE_INT));
    state->currentInterval = state->interval;
    state->qobj = dynamic_cast<QObject*>(item);

    if (state->qobj) {
        //kDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!! got us an object!";
        disconnect(state->qobj, SIGNAL(destroyed(QObject*)),
                   this, SLOT(animatedItemDestroyed(QObject*)));
        connect(state->qobj, SIGNAL(destroyed(QObject*)),
                this, SLOT(animatedItemDestroyed(QObject*)));
    }

    d->animatedItems[item] = state;
    d->performAnimation(0, state);

    if (!d->timerId) {
        d->timerId = startTimer(MIN_TICK_RATE);
        d->time.restart();
    }

    return state->id;
}

int Animator::moveItem(QGraphicsItem *item, Movement movement, const QPoint &destination)
{
     //kDebug();
     QHash<QGraphicsItem*, MovementState*>::iterator it = d->movingItems.find(item);
     if (it != d->movingItems.end()) {
         if (d->timerId) {
             d->movingItemsToDelete.insert(it.value());
         } else {
             delete it.value();
         }

         d->movingItems.erase(it);
     }

     int frames = d->driver->movementAnimationFps(movement);
     if (frames <= 1) {
          // evidently this animator doesn't have an implementation
          // for this Animation
          return -1;
     }

     MovementState *state = new MovementState;
     state->id = ++d->animId;
     state->destination = destination;
     state->start = item->pos().toPoint();
     state->item = item;
     state->movement = movement;
     state->curve = d->driver->movementAnimationCurve(movement);
     int duration = d->driver->movementAnimationDuration(movement);
     state->frames = qMax(1.0, frames * (duration / 1000.0));
     state->currentFrame = 0;
     state->interval = duration / qreal(state->frames);
     state->interval = qMax(MIN_TICK_RATE_INT, state->interval - (state->interval % MIN_TICK_RATE_INT));
//     state->interval = (state->interval / MIN_TICK_RATE) * MIN_TICK_RATE;
//     kDebug() << "interval of" << state->interval << state->frames << duration << frames;
     state->currentInterval = state->interval;
     state->qobj = dynamic_cast<QObject*>(item);

     if (state->qobj) {
        disconnect(state->qobj, SIGNAL(destroyed(QObject*)), this, SLOT(movingItemDestroyed(QObject*)));
        connect(state->qobj, SIGNAL(destroyed(QObject*)), this, SLOT(movingItemDestroyed(QObject*)));
     }

     d->movingItems[item] = state;
     d->performMovement(0, state);

     if (!d->timerId) {
          d->timerId = startTimer(MIN_TICK_RATE);
          d->time.restart();
     }

     return state->id;
}

int Animator::customAnimation(int frames, int duration, Animator::CurveShape curve,
                              QObject *receiver, const char *slot)
{
    if (frames < 1 || duration < 1 || !receiver || !slot) {
        return -1;
    }

    CustomAnimationState *state = new CustomAnimationState;
    state->id = ++d->animId;
    state->frames = frames;
    state->currentFrame = 0;
    state->curve = curve;
    state->frameInterval = qMax(qreal(1.0), duration / qreal(state->frames));
    state->interval = qMax(MIN_TICK_RATE_INT, state->frameInterval - (state->frameInterval % MIN_TICK_RATE_INT));
    state->currentInterval = state->interval;
    state->receiver = receiver;
    state->slot = qstrdup(slot);

    d->customAnims[state->id] = state;

    disconnect(receiver, SIGNAL(destroyed(QObject*)),
               this, SLOT(customAnimReceiverDestroyed(QObject*)));
    connect(receiver, SIGNAL(destroyed(QObject*)),
            this, SLOT(customAnimReceiverDestroyed(QObject*)));

    // try with only progress as argument
    if (!QMetaObject::invokeMethod(receiver, slot, Q_ARG(qreal, 0))) {
        //try to pass also the animation id
        QMetaObject::invokeMethod(receiver, slot, Q_ARG(qreal, 0), Q_ARG(int, state->id));
    }

    if (!d->timerId) {
        d->timerId = startTimer(MIN_TICK_RATE);
        d->time.restart();
    }

    return state->id;
}

void Animator::stopCustomAnimation(int id)
{
    QHash<int, CustomAnimationState*>::iterator it = d->customAnims.find(id);
    if (it != d->customAnims.end()) {
        if (d->timerId) {
            d->customAnimsToDelete.insert(it.value());
        } else {
            delete[] it.value()->slot;
            delete it.value();
        }

        d->customAnims.erase(it);
    }
    //kDebug() << "stopCustomAnimation(AnimId " << id << ") done";
}

void Animator::stopItemAnimation(int id)
{
    QMutableHashIterator<QGraphicsItem*, AnimationState*> it(d->animatedItems);
    while (it.hasNext()) {
        it.next();
        if (it.value()->id == id) {
            if (d->timerId) {
                d->animatedItemsToDelete.insert(it.value());
            } else {
                delete it.value();
            }

            it.remove();
            return;
        }
    }
}

void Animator::stopItemMovement(int id)
{
    QMutableHashIterator<QGraphicsItem*, MovementState*> it(d->movingItems);
    while (it.hasNext()) {
        it.next();
        if (it.value()->id == id) {
            if (d->timerId) {
                d->movingItemsToDelete.insert(it.value());
            } else {
                delete it.value();
            }

            it.remove();
            return;
        }
    }
}

int Animator::animateElement(QGraphicsItem *item, Animation animation)
{
    //kDebug() << "startElementAnimation(AnimId " << animation << ")";
    int frames = d->driver->elementAnimationFps(animation);
    int duration = d->driver->animationDuration(animation);

    ElementAnimationState *state = new ElementAnimationState;
    state->item = item;
    state->curve = d->driver->elementAnimationCurve(animation);
    state->animation = animation;
    state->frames = qMax(1.0, frames * (duration / 1000.0));
    state->currentFrame = 0;
    state->interval = duration / qreal(state->frames);
    state->interval = qMax(MIN_TICK_RATE_INT, state->interval - (state->interval % MIN_TICK_RATE_INT));
    state->currentInterval = state->interval;
    state->id = ++d->animId;
    state->qobj = dynamic_cast<QObject*>(item);

    if (state->qobj) {
        disconnect(state->qobj, SIGNAL(destroyed(QObject*)),
                   this, SLOT(animatedElementDestroyed(QObject*)));
        connect(state->qobj, SIGNAL(destroyed(QObject*)),
                this, SLOT(animatedElementDestroyed(QObject*)));
    }

    //kDebug() << "animateElement " << animation << ", interval: "
    //         << state->interval << ", frames: " << state->frames;
    bool needTimer = true;
    if (state->frames < 1) {
        state->frames = 1;
        state->currentFrame = 1;
        needTimer = false;
    }

    d->animatedElements[state->id] = state;

    //kDebug() << "startElementAnimation(AnimId " << animation << ") returning " << state->id;
    if (needTimer && !d->timerId) {
        // start a 20fps timer;
        d->timerId = startTimer(MIN_TICK_RATE);
        d->time.restart();
    }
    return state->id;
}

void Animator::stopElementAnimation(int id)
{
    QHash<int, ElementAnimationState*>::iterator it = d->animatedElements.find(id);
    if (it != d->animatedElements.end()) {
        if (d->timerId) {
            d->animatedElementsToDelete.insert(it.value());
        } else {
            delete it.value();
        }

        d->animatedElements.erase(it);
    }
    //kDebug() << "stopElementAnimation(AnimId " << id << ") done";
}

void Animator::setInitialPixmap(int id, const QPixmap &pixmap)
{
    QHash<int, ElementAnimationState*>::iterator it = d->animatedElements.find(id);

    if (it == d->animatedElements.end()) {
        kDebug() << "No entry found for id " << id;
        return;
    }

    it.value()->pixmap = pixmap;
}

QPixmap Animator::currentPixmap(int id)
{
    QHash<int, ElementAnimationState*>::const_iterator it = d->animatedElements.constFind(id);

    if (it == d->animatedElements.constEnd()) {
        //kDebug() << "Animator::currentPixmap(" << id << ") found no entry for it!";
        return QPixmap();
    }

    ElementAnimationState *state = it.value();
    qreal progress = d->calculateProgress(state->currentFrame * state->interval,
                                          state->frames *  state->interval,
                                          state->curve);
    //kDebug() << "Animator::currentPixmap(" << id <<   " at " << progress;

    switch (state->animation) {
        case AppearAnimation:
            return d->driver->elementAppear(progress, state->pixmap);
            break;
        case DisappearAnimation:
            return d->driver->elementDisappear(progress, state->pixmap);
            break;
        case ActivateAnimation:
            break;
        default:
            kDebug() << "Unsupported animation type.";

    }

    return state->pixmap;
}

bool Animator::isAnimating() const
{
    return (!d->animatedItems.isEmpty() ||
            !d->movingItems.isEmpty() ||
            !d->animatedElements.isEmpty() ||
            !d->customAnims.isEmpty());
}

void Animator::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != d->timerId) {
        QObject::timerEvent(event);
        return;
    }

    Q_UNUSED(event)
    bool animationsRemain = false;
    int elapsed = MIN_TICK_RATE;
    if (d->time.elapsed() > elapsed) {
        elapsed = d->time.elapsed();
    }
    d->time.restart();
    //kDebug() << "timeEvent, elapsed time: " << elapsed;

    foreach (AnimationState *state, d->animatedItems) {
        if (d->animatedItemsToDelete.contains(state)) {
            continue;
        }

        if (state->currentInterval <= elapsed) {
            // we need to step forward!
            state->currentFrame +=
                (KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects) ?
                qMax(1, elapsed / state->interval) : state->frames - state->currentFrame;

            if (state->currentFrame < state->frames) {
                qreal progress = d->calculateProgress(state->currentFrame * state->interval,
                                                      state->frames *  state->interval,
                                                      state->curve);
                d->performAnimation(progress, state);
                state->currentInterval = state->interval;
                animationsRemain = true;
            } else {
                d->performAnimation(1, state);
                d->animatedItems.erase(d->animatedItems.find(state->item));
                emit animationFinished(state->item, state->animation);
                d->animatedItemsToDelete.insert(state);
            }
        } else {
            state->currentInterval -= elapsed;
            animationsRemain = true;
        }
    }

    foreach (MovementState *state, d->movingItems) {
        if (d->movingItemsToDelete.contains(state)) {
            continue;
        }

        if (state->currentInterval <= elapsed) {
            // we need to step forward!
            state->currentFrame +=
                (KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects) ?
                qMax(1, elapsed / state->interval) : state->frames - state->currentFrame;

            if (state->currentFrame < state->frames) {
                //kDebug() << "movement";
                qreal progress = d->calculateProgress(state->currentFrame * state->interval,
                                                      state->frames *  state->interval,
                                                      state->curve);
                d->performMovement(progress, state);
                animationsRemain = true;
            } else {
                //kDebug() << "movement";
                d->performMovement(1, state);
                d->movingItems.erase(d->movingItems.find(state->item));
                emit movementFinished(state->item);
                d->movingItemsToDelete.insert(state);
            }
        } else {
            state->currentInterval -= elapsed;
            animationsRemain = true;
        }
    }

    foreach (ElementAnimationState *state, d->animatedElements) {
        if (d->animatedElementsToDelete.contains(state)) {
            continue;
        }

        if (state->currentFrame == state->frames) {
            //kDebug() << "skipping" << state->id << "as it is already at frame"
            //         << state->currentFrame << "of" << state->frames;
            // since we keep element animations around until they are
            // removed, we will end up with finished animations in the queue;
            // just skip them
            continue;
        }

        if (state->currentInterval <= elapsed) {
            // we need to step forward!
            /*kDebug() << "stepping forwards element anim " << state->id
                       << " from " << state->currentFrame
                       << " by " << qMax(1, elapsed / state->interval) << " to "
                       << state->currentFrame + qMax(1, elapsed / state->interval) << endl;*/
            state->currentFrame +=
                (KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects) ?
                qMax(1, elapsed / state->interval) : state->frames - state->currentFrame;

            state->item->update();
            if (state->currentFrame < state->frames) {
                state->currentInterval = state->interval;
                animationsRemain = true;
            } else {
                d->animatedElements.remove(state->id);
                emit elementAnimationFinished(state->id);
                d->animatedElementsToDelete.insert(state);
            }
        } else {
            state->currentInterval -= elapsed;
            animationsRemain = true;
        }
    }

    foreach (CustomAnimationState *state, d->customAnims) {
        if (d->customAnimsToDelete.contains(state)) {
            continue;
        }

        if (state->currentInterval <= elapsed) {
            // advance the frame
            state->currentFrame +=
                (KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects) ?
                qMax(1, elapsed / state->frameInterval) : state->frames - state->currentFrame;
            /*kDebug() << "custom anim for" << state->receiver
                       << "to slot" << state->slot
                       << "with interval of" << state->interval
                       << "at frame" << state->currentFrame;*/

            if (state->currentFrame < state->frames) {
                //kDebug () << "not the final frame";
                state->currentInterval = state->interval;
                animationsRemain = true;
                // signal the object
                // try with only progress as argument
                qreal progress = d->calculateProgress(state->currentFrame * state->interval,
                                                      state->frames *  state->interval,
                                                      state->curve);
                if (!QMetaObject::invokeMethod(state->receiver, state->slot, Q_ARG(qreal, progress))) {
                //if fails try to add the animation id
                    QMetaObject::invokeMethod(state->receiver, state->slot, Q_ARG(qreal, progress),
                                              Q_ARG(int, state->id));
                }
            } else {
                if (!QMetaObject::invokeMethod(state->receiver, state->slot, Q_ARG(qreal, 1))) {
                    QMetaObject::invokeMethod(state->receiver, state->slot, Q_ARG(qreal, 1), Q_ARG(int, state->id));
                }
                d->customAnims.erase(d->customAnims.find(state->id));
                emit customAnimationFinished(state->id);
                d->customAnimsToDelete.insert(state);
            }
        } else {
            state->currentInterval -= elapsed;
            animationsRemain = true;
        }
    }

    if (!animationsRemain && d->timerId) {
        killTimer(d->timerId);
        d->timerId = 0;
    }

    d->cleanupStates();
}

void AnimatorPrivate::init(Animator *q)
{
    //FIXME: usage between different applications?
    KConfig c("plasmarc");
    KConfigGroup cg(&c, "Animator");
    QString pluginName = cg.readEntry("driver", "default");

    if (!pluginName.isEmpty()) {
        QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(pluginName);
        KService::List offers = KServiceTypeTrader::self()->query("Plasma/Animator", constraint);

        if (!offers.isEmpty()) {
            QString error;

            KPluginLoader plugin(*offers.first());

            if (Plasma::isPluginVersionCompatible(plugin.pluginVersion())) {
                driver = offers.first()->createInstance<Plasma::AnimationDriver>(q, QVariantList(), &error);
            }

            if (!driver) {
                kDebug() << "Could not load requested animator "
                         << offers.first() << ". Error given: " << error;
            }
        }
    }

    if (!driver) {
        driver = new AnimationDriver(q);
    }
}

void AnimatorPrivate::cleanupStates()
{
    /*
    kDebug() << animatedItemsToDelete.count() << animatedElementsToDelete.count()
             << movingItemsToDelete.count() << customAnimsToDelete.count();
             */
    qDeleteAll(animatedItemsToDelete);
    animatedItemsToDelete.clear();
    qDeleteAll(animatedElementsToDelete);
    animatedElementsToDelete.clear();
    qDeleteAll(movingItemsToDelete);
    movingItemsToDelete.clear();

    QSetIterator<CustomAnimationState*> it(customAnimsToDelete);
    while (it.hasNext()) {
        CustomAnimationState *state = it.next();
        delete[] state->slot;
        delete state;
    }
    customAnimsToDelete.clear();
}

void Animator::registerScrollingManager(QGraphicsWidget *widget)
{
    if (!d->scrollingManagers.contains(widget)) {
        KineticScrolling *scroll = new KineticScrolling(widget);
        d->scrollingManagers.insert(widget, scroll);
        connect(scroll,
                SIGNAL(stateChanged(QAbstractAnimation::State, QAbstractAnimation::State)), this,
                SLOT(scrollStateChanged(QAbstractAnimation::State, QAbstractAnimation::State)));
    }
}

void Animator::unregisterScrollingManager(QGraphicsWidget *widget)
{
    if (d->scrollingManagers.contains(widget)) {
        disconnect(d->scrollingManagers.value(widget),
                SIGNAL(stateChanged(QAbstractAnimation::State, QAbstractAnimation::State)), this,
                SLOT(scrollStateChanged(QAbstractAnimation::State, QAbstractAnimation::State)));
        d->scrollingManagers.value(widget)->deleteLater();
        d->scrollingManagers.remove(widget);
    }
}


} // namespace Plasma

