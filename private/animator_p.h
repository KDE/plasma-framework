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

#ifndef ANIMATOR_P_H
#define ANIMATOR_P_H

#include <QHash>
#include <QPixmap>
#include <QSet>
#include <QTime>
#include <QTimeLine>

class QGraphicsItem;

namespace Plasma
{

class AnimationDriver;
class KineticScrolling;

struct AnimationState
{
    QGraphicsItem *item;
    QObject *qobj;
    Animator::Animation animation;
    Animator::CurveShape curve;
    int interval;
    int currentInterval;
    int frames;
    int currentFrame;
    int id;
};

struct ElementAnimationState
{
    QGraphicsItem *item;
    QObject *qobj;
    Animator::CurveShape curve;
    Animator::Animation animation;
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
    Animator::CurveShape curve;
    Animator::Movement movement;
    int interval;
    int currentInterval;
    int frames;
    int currentFrame;
    QPoint start;
    QPoint destination;
    int id;
};

struct CustomAnimationState
{
    Animator::CurveShape curve;
    int frames;
    int currentFrame;
    int frameInterval;
    int interval;
    int currentInterval;
    int id;
    QObject *receiver;
    char *slot;
};

class Animator;

class AnimatorPrivate
{
    public:
        AnimatorPrivate(Animator *parent);
        ~AnimatorPrivate();

        qreal calculateProgress(int time, int duration, Animator::CurveShape curve);
        void performAnimation(qreal amount, const AnimationState *state);
        void performMovement(qreal amount, const MovementState *state);

        void init(Animator *q);
        void cleanupStates();
        void animatedItemDestroyed(QObject*);
        void movingItemDestroyed(QObject*);
        void animatedElementDestroyed(QObject*);
        void customAnimReceiverDestroyed(QObject*);

        void scrollStateChanged(QAbstractAnimation::State newState,
                QAbstractAnimation::State oldState);

        Animator *q;
        AnimationDriver *driver;
        int animId;
        int timerId;
        QTime time;
        QTimeLine timeline;

        // active items
        QHash<QGraphicsItem *, AnimationState *> animatedItems;
        QHash<QGraphicsItem *, MovementState *> movingItems;
        QHash<int, ElementAnimationState *> animatedElements;
        QHash<int, CustomAnimationState *> customAnims;

        // items to cull
        QSet<AnimationState *> animatedItemsToDelete;
        QSet<MovementState *> movingItemsToDelete;
        QSet<ElementAnimationState *> animatedElementsToDelete;
        QSet<CustomAnimationState *> customAnimsToDelete;

        QHash<QGraphicsWidget *, KineticScrolling *> scrollingManagers;

        static void mapAnimation(Animator::Animation from, Animator::Animation to);
        static void mapAnimation(Animator::Animation from, const QString &to);

        static QHash<Animator::Animation, Animator::Animation> s_stockAnimMappings;
        static QHash<Animator::Animation, QString> s_loadableAnimMappings;
};

}

#endif

