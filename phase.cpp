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

#include "phase.h"

#include <QGraphicsItem>

#include <KConfig>
#include <KConfigGroup>
#include <KService>
#include <KServiceTypeTrader>
#include <KGlobalSettings>

#include "animator.h"

namespace Plasma
{

static const qreal MIN_TICK_RATE = 40;

struct AnimationState
{
    QGraphicsItem *item;
    QObject *qobj;
    Phase::Animation animation;
    Phase::CurveShape curve;
    int interval;
    int currentInterval;
    int frames;
    int currentFrame;
};

struct ElementAnimationState
{
    QGraphicsItem *item;
    QObject *qobj;
    Phase::CurveShape curve;
    Phase::ElementAnimation animation;
    int interval;
    int currentInterval;
    int frames;
    int currentFrame;
    int id;
    QPixmap pixmap;
};

struct MovementState
{
    QGraphicsItem *item;
    QObject *qobj;
    Phase::CurveShape curve;
    Phase::Movement movement;
    int interval;
    int currentInterval;
    int frames;
    int currentFrame;
    QPoint start;
    QPoint destination;
};

struct CustomAnimationState
{
    Phase::CurveShape curve;
    int frames;
    int currentFrame;
    int interval;
    int currentInterval;
    Phase::AnimId id;
    QObject* receiver;
    char* slot;
};

class Phase::Private
{
    public:

        Private()
            : animator(0),
              animId(0),
              timerId(0)
        {
        }

        ~Private()
        {
            qDeleteAll(animatedItems);
            qDeleteAll(animatedElements);
            qDeleteAll(movingItems);

            QMutableMapIterator<AnimId, CustomAnimationState*> it(customAnims);
            while (it.hasNext()) {
                delete it.value()->slot;
                delete it.value();
                it.remove();
            }

            // Animator is a QObject
            // and we don't own the items
        }

        qreal calculateProgress(int frames, int currentFrame)
        {
            if (!(KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects)) {
                return qreal(1.0);
            }

            qreal progress = frames;
            progress = currentFrame / progress;
            progress = qMin(qreal(1.0), qMax(qreal(0.0), progress));
            return progress;
        }

        void performAnimation(qreal amount, const AnimationState* state)
        {
            switch (state->animation) {
                case Phase::Appear:
                    animator->itemAppear(amount, state->item);
                    break;
                case Phase::Disappear:
                    animator->itemDisappear(amount, state->item);
                    if (amount >= 1) {
                        state->item->hide();
                    }
                    break;
                case Phase::Activate:
                    animator->itemActivated(amount, state->item);
                    break;
            }
        }

        void performMovement(qreal amount, const MovementState* state)
        {
            switch (state->movement) {
                case Phase::SlideIn:
                case Phase::FastSlideIn:
                    //kDebug() << "performMovement, SlideIn";
                    animator->itemSlideIn(amount, state->item, state->start, state->destination);
                    break;
                case Phase::SlideOut:
                case Phase::FastSlideOut:
                    //kDebug() << "performMovement, SlideOut";
                    animator->itemSlideOut(amount, state->item, state->start, state->destination);
                    break;
            }
        }

        Animator* animator;
        int animId;
        int timerId;
        QTime time;

        //TODO: eventually perhaps we should allow multiple animations simulataneously
        //      which would imply changing this to a QMap<QGraphicsItem*, QList<QTimeLine*> >
        //      and really making the code fun ;)
        QMap<QGraphicsItem*, AnimationState*> animatedItems;
        QMap<QGraphicsItem*, MovementState*> movingItems;
        QMap<AnimId, ElementAnimationState*> animatedElements;
        QMap<AnimId, CustomAnimationState*> customAnims;
};

class PhaseSingleton
{
    public:
        Phase self;
};

K_GLOBAL_STATIC( PhaseSingleton, privateSelf )

Phase* Phase::self()
{
    return &privateSelf->self;
}


Phase::Phase(QObject * parent)
    : QObject(parent),
      d(new Private)
{
    init();
}

Phase::~Phase()
{
    delete d;
}

void Phase::animatedItemDestroyed(QObject* o)
{
    //kDebug() << "testing for" << (void*)o;
    QMutableMapIterator<QGraphicsItem*, AnimationState*> it(d->animatedItems);
    while (it.hasNext()) {
        it.next();
        //kDebug() << "comparing against" << it.value()->qobj;
        if (it.value()->qobj == o) {
            kDebug() << "found deleted animated item";
            delete it.value();
            it.remove();
        }
    }
}

void Phase::movingItemDestroyed(QObject* o)
{
    QMutableMapIterator<QGraphicsItem*, MovementState*> it(d->movingItems);
    while (it.hasNext()) {
        it.next();
        if (it.value()->qobj == o) {
            delete it.value();
            it.remove();
        }
    }
}

void Phase::animatedElementDestroyed(QObject* o)
{
    QMutableMapIterator<AnimId, ElementAnimationState*> it(d->animatedElements);
    while (it.hasNext()) {
        it.next();
        if (it.value()->qobj == o) {
            delete it.value();
            it.remove();
        }
    }
}

void Phase::customAnimReceiverDestroyed(QObject* o)
{
    QMutableMapIterator<AnimId, CustomAnimationState*> it(d->customAnims);
    while (it.hasNext()) {
        if (it.next().value()->receiver == o) {
            delete it.value()->slot;
            delete it.value();
            it.remove();
        }
    }
}

void Phase::animateItem(QGraphicsItem* item, Animation animation)
{
     //kDebug();
    // get rid of any existing animations on this item.
    //TODO: shoudl we allow multiple anims per item?
    QMap<QGraphicsItem*, AnimationState*>::iterator it = d->animatedItems.find(item);
    if (it != d->animatedItems.end()) {
        delete it.value();
        d->animatedItems.erase(it);
    }

    int frames = d->animator->animationFramesPerSecond(animation);

    if (frames < 1) {
        // evidently this animator doesn't have an implementation
        // for this Animation
        return;
    }

    AnimationState* state = new AnimationState;
    state->item = item;
    state->animation = animation;
    state->curve = d->animator->curve(animation);
    //TODO: variance in times based on the value of animation
    state->frames = frames / 3;
    state->currentFrame = 0;
    state->interval = d->animator->animationDuration(animation) / state->frames;
    state->interval = (state->interval / MIN_TICK_RATE) * MIN_TICK_RATE;
    state->currentInterval = state->interval;
    state->qobj = dynamic_cast<QObject*>(item);

     if (state->qobj) {
         //kDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!! got us an object!";
         disconnect(state->qobj, SIGNAL(destroyed(QObject*)), this, SLOT(animatedItemDestroyed(QObject*)));
         connect(state->qobj, SIGNAL(destroyed(QObject*)), this, SLOT(animatedItemDestroyed(QObject*)));
     }

    d->animatedItems[item] = state;
    d->performAnimation(0, state);

    if (!d->timerId) {
        d->timerId = startTimer(MIN_TICK_RATE);
        d->time.restart();
    }
}

void Phase::moveItem(QGraphicsItem* item, Movement movement, const QPoint &destination)
{
     //kDebug();
     QMap<QGraphicsItem*, MovementState*>::iterator it = d->movingItems.find(item);
     if (it != d->movingItems.end()) {
          delete it.value();
          d->movingItems.erase(it);
     }

     int frames = d->animator->movementAnimationFramesPerSecond(movement);
     if (frames <= 1) {
          // evidently this animator doesn't have an implementation
          // for this Animation
          return;
     }

     MovementState* state = new MovementState;
     state->destination = destination;
     state->start = item->pos().toPoint();
     state->item = item;
     state->movement = movement;
     state->curve = d->animator->curve(movement);
     //TODO: variance in times based on the value of animation
     state->frames = frames / 2;
     state->currentFrame = 0;
     state->interval = d->animator->duration(movement) / state->frames;
     state->interval = (state->interval / MIN_TICK_RATE) * MIN_TICK_RATE;
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
}

Phase::AnimId Phase::customAnimation(int frames, int duration, Phase::CurveShape curve,
                                     QObject* receiver, const char* slot)
{
    if (frames < 1 || duration < 1 || !receiver || !slot) {
        return -1;
    }

    CustomAnimationState *state = new CustomAnimationState;
    state->id = ++d->animId;
    state->frames = frames;
    state->currentFrame = 0;
    state->curve = curve;
    state->interval = duration / qreal(state->frames);
    state->interval = qMax( 1, state->interval );
    state->interval = (state->interval / MIN_TICK_RATE) * MIN_TICK_RATE;
    state->currentInterval = state->interval;
    state->receiver = receiver;
    state->slot = qstrdup(slot);

    d->customAnims[state->id] = state;

    disconnect(receiver, SIGNAL(destroyed(QObject*)),
               this, SLOT(customAnimReceiverDestroyed(QObject*)));
    connect(receiver, SIGNAL(destroyed(QObject*)),
            this, SLOT(customAnimReceiverDestroyed(QObject*)));

    QMetaObject::invokeMethod(receiver, slot, Q_ARG(qreal, 0));

    if (!d->timerId) {
        d->timerId = startTimer(MIN_TICK_RATE);
        d->time.restart();
    }

    return state->id;
}

void Phase::stopCustomAnimation(AnimId id)
{
    QMap<AnimId, CustomAnimationState*>::iterator it = d->customAnims.find(id);
    if (it != d->customAnims.end()) {
        delete [] it.value()->slot;
        delete it.value();
        d->customAnims.erase(it);
    }
    //kDebug() << "stopCustomAnimation(AnimId " << id << ") done";
}

Phase::AnimId Phase::animateElement(QGraphicsItem *item, ElementAnimation animation)
{
    //kDebug() << "startElementAnimation(AnimId " << animation << ")";
    ElementAnimationState *state = new ElementAnimationState;
    state->item = item;
    state->curve = d->animator->curve(animation);
    state->animation = animation;
    //TODO: variance in times based on the value of animation
    state->frames = d->animator->animationFramesPerSecond(animation) / 5;
    state->currentFrame = 0;
    state->interval = d->animator->duration(animation) / state->frames;
    state->interval = (state->interval / MIN_TICK_RATE) * MIN_TICK_RATE;
    state->currentInterval = state->interval;
    state->id = ++d->animId;
    state->qobj = dynamic_cast<QObject*>(item);

     if (state->qobj) {
         disconnect(state->qobj, SIGNAL(destroyed(QObject*)), this, SLOT(animatedElementDestroyed(QObject*)));
         connect(state->qobj, SIGNAL(destroyed(QObject*)), this, SLOT(animatedElementDestroyed(QObject*)));
     }

    //kDebug() << "animateElement " << animation << ", interval: " << state->interval << ", frames: " << state->frames;
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
        //TODO: should be started at the maximum frame rate needed only?
        d->timerId = startTimer(MIN_TICK_RATE);
        d->time.restart();
    }
    return state->id;
}

void Phase::stopElementAnimation(AnimId id)
{
    QMap<AnimId, ElementAnimationState*>::iterator it = d->animatedElements.find(id);
    if (it != d->animatedElements.end()) {
        delete it.value();
        d->animatedElements.erase(it);
    }
    //kDebug() << "stopElementAnimation(AnimId " << id << ") done";
}

void Phase::setAnimationPixmap(AnimId id, const QPixmap &pixmap)
{
    QMap<AnimId, ElementAnimationState*>::iterator it = d->animatedElements.find(id);

    if (it == d->animatedElements.end()) {
        kDebug() << "Phase::setAnimationPixmap(" << id << ") found no entry for it!";
        return;
    }

    it.value()->pixmap = pixmap;
}

QPixmap Phase::animationResult(AnimId id)
{
    QMap<AnimId, ElementAnimationState*>::const_iterator it = d->animatedElements.find(id);

    if (it == d->animatedElements.constEnd()) {
        //kDebug() << "Phase::animationResult(" << id << ") found no entry for it!";
        return QPixmap();
    }

    ElementAnimationState* state = it.value();
    qreal progress = state->frames;
    //kDebug() << "Phase::animationResult(" << id <<   " at " << progress;
    progress = state->currentFrame / progress;
    progress = qMin(qreal(1.0), qMax(qreal(0.0), progress));
    //kDebug() << "Phase::animationResult(" << id <<   " at " << progress;

    switch (state->animation) {
        case ElementAppear:
            return d->animator->elementAppear(progress, state->pixmap);
            break;
        case ElementDisappear:
            return d->animator->elementDisappear(progress, state->pixmap);
            break;
    }

    return state->pixmap;
}

bool Phase::isAnimating() const
{
    return (!d->animatedItems.isEmpty() ||
            !d->movingItems.isEmpty() ||
            !d->animatedElements.isEmpty() ||
            !d->customAnims.isEmpty());
}

void Phase::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)
    bool animationsRemain = false;
    int elapsed = MIN_TICK_RATE;
    if (d->time.elapsed() > elapsed) {
        elapsed = d->time.elapsed();
    }
    d->time.restart();
    //kDebug() << "timeEvent, elapsed time: " << elapsed;

    foreach (AnimationState* state, d->animatedItems) {
        if (state->currentInterval <= elapsed) {
            // we need to step forward!
            state->currentFrame += (KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects) ?
                                   qMax(1, elapsed / state->interval) : state->frames - state->currentFrame;

            if (state->currentFrame < state->frames) {
                qreal progress = d->calculateProgress(state->frames, state->currentFrame);
                d->performAnimation(progress, state);
                state->currentInterval = state->interval;
                //TODO: calculate a proper interval based on the curve
                state->interval *= 1 - progress;
                animationsRemain = true;
            } else {
                d->performAnimation(1, state);
                d->animatedItems.erase(d->animatedItems.find(state->item));
                emit animationComplete(state->item, state->animation);
                delete state;
            }
        } else {
            state->currentInterval -= elapsed;
            animationsRemain = true;
        }
    }

    foreach (MovementState* state, d->movingItems) {
        if (state->currentInterval <= elapsed) {
            // we need to step forward!
            state->currentFrame += (KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects) ?
                                   qMax(1, elapsed / state->interval) : state->frames - state->currentFrame;

            if (state->currentFrame < state->frames) {
                d->performMovement(d->calculateProgress(state->frames, state->currentFrame), state);
                //TODO: calculate a proper interval based on the curve
                state->currentInterval = state->interval;
                animationsRemain = true;
            } else {
                d->performMovement(1, state);
                d->movingItems.erase(d->movingItems.find(state->item));
                emit movementComplete(state->item);
                delete state;
            }
        } else {
            state->currentInterval -= elapsed;
            animationsRemain = true;
        }
    }

    foreach (ElementAnimationState* state, d->animatedElements) {
        if (state->currentFrame == state->frames) {
            //kDebug() << "skipping" << state->id << "as its already at frame" << state->currentFrame << "of" << state->frames;
            // since we keep element animations around until they are
            // removed, we will end up with finished animations in the queue;
            // just skip them
            //TODO: should we move them to a separate QMap?
            continue;
        }

        if (state->currentInterval <= elapsed) {
            // we need to step forward!
            /*kDebug() << "stepping forwards element anim " << state->id << " from " << state->currentFrame
                    << " by " << qMax(1, elapsed / state->interval) << " to "
                    << state->currentFrame + qMax(1, elapsed / state->interval) << endl;*/
            state->currentFrame += (KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects) ?
                                   qMax(1, elapsed / state->interval) : state->frames - state->currentFrame;
            if (state->currentFrame < state->frames) {
                state->currentInterval = state->interval;
                //TODO: calculate a proper interval based on the curve
                state->interval *= 1 - d->calculateProgress(state->frames, state->currentFrame);
                animationsRemain = true;
            } else {
                d->animatedElements.remove(state->id);
                emit elementAnimationComplete(state->id);
                delete state;
            }
        } else {
            state->currentInterval -= elapsed;
            animationsRemain = true;
        }
    }

    foreach (CustomAnimationState *state, d->customAnims) {
        if (state->currentInterval <= elapsed) {
            // advance the frame
            state->currentFrame += (KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects) ?
                                   qMax(1, elapsed / state->interval) : state->frames - state->currentFrame;
            /*kDebug() << "custom anim for" << state->receiver << "to slot" << state->slot
                     << "with interval of" << state->interval << "at frame" << state->currentFrame;*/

            if (state->currentFrame < state->frames) {
                //kDebug () << "not the final frame";
                //TODO: calculate a proper interval based on the curve
                state->currentInterval = state->interval;
                animationsRemain = true;
                // signal the object
                QMetaObject::invokeMethod(state->receiver, state->slot,
                                          Q_ARG(qreal,
                                                d->calculateProgress(state->frames, state->currentFrame)));
            } else {
                QMetaObject::invokeMethod(state->receiver, state->slot, Q_ARG(qreal, 1));
                d->customAnims.erase(d->customAnims.find(state->id));
                emit customAnimationComplete(state->id);
                delete [] state->slot;
                delete state;
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
}

void Phase::init()
{
    KConfig c("plasmarc");
    KConfigGroup cg(&c, "Phase");
    QString pluginName = cg.readEntry("animator", "default");

    if (!pluginName.isEmpty()) {
        QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(pluginName);
        KService::List offers = KServiceTypeTrader::self()->query("Plasma/Animator", constraint);

        if (!offers.isEmpty()) {
            QString error;
            d->animator = offers.first()->createInstance<Plasma::Animator>(0, QVariantList(), &error);
            if (!d->animator) {
                kDebug() << "Could not load requested animator " << offers.first() << ". Error given: " << error;
            }
        }
    }

    if (!d->animator) {
        d->animator = new Animator(this);
    }
}

} // namespace Plasma

#include <phase.moc>
