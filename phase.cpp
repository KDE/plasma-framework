/*
 *   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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
#include <QTimeLine>

#include <KConfig>
#include <KConfigGroup>
#include <KService>
#include <KServiceTypeTrader>

#include "animator.h"

namespace Plasma
{

struct AnimationState
{
    QGraphicsItem* item;
    Phase::Animation animation;
    int frames;
};

struct ElementAnimationState
{
    QGraphicsItem* item;
    QTimeLine* timeline;
    Phase::ElementAnimation animation;
    int frames;
    int currentFrame;
    int id;
    QPixmap pixmap;
};

class Phase::Private
{
    public:

        Private()
            : animator(0),
              animId(0)
        {
        }

        ~Private()
        {
            // delete animator; Animator is a QObject
            // TimeLine's are parented to us, and we don't own the items
        }

        Animator* animator;
        int animId;

        //TODO: eventually perhaps we should allow multiple animations simulataneously
        //      which would imply changing this to a QMap<QGraphicsItem*, QList<QTimeLine*> >
        //      and really making the code fun ;)
        QMap<QGraphicsItem*, QTimeLine*> theAnimated;
        QMap<QTimeLine*, AnimationState> animations;
        QMap<Phase::AnimId, ElementAnimationState> theAnimatedElements;
        QMap<QTimeLine*, Phase::AnimId> elementAnimations;
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

void Phase::appletDestroyed(QObject* o)
{
    QGraphicsItem* item = dynamic_cast<QGraphicsItem*>(o);

    if (!item) {
        return;
    }

    QMap<QGraphicsItem*, QTimeLine*>::iterator it = d->theAnimated.find(item);

    if (it == d->theAnimated.end()) {
        return;
    }

    d->animations.erase(d->animations.find(it.value()));
    d->theAnimated.erase(it);
    delete it.value();
}

void Phase::animate(QGraphicsItem* item, Animation animation)
{
    QMap<QGraphicsItem*, QTimeLine*>::iterator it = d->theAnimated.find(item);

    if (it != d->theAnimated.end()) {
        delete it.value();
        d->animations.erase(d->animations.find(it.value()));
    }

    //TODO: allow the animator to define this?
    QTimeLine::CurveShape curveShape = QTimeLine::EaseInOutCurve;
    int frames = d->animator->frameCount(animation);

    if (frames < 1) {
        return;
    }

    AnimationState state;
    state.item = item;
    state.animation = animation;
    //TODO: variance in times based on the value of animation
    state.frames = frames / 3;

    //TODO: variance in times based on the value of animation
    QTimeLine* timeLine = new QTimeLine(333, this);
    timeLine->setFrameRange(0, state.frames);
    timeLine->setCurveShape(curveShape);

    d->animations[timeLine] = state;
    d->theAnimated[item] = timeLine;
    connect(timeLine, SIGNAL(frameChanged(int)), this, SLOT(advanceFrame(int)));
    connect(timeLine, SIGNAL(finished()), this, SLOT(animationComplete()));
    timeLine->start();
    advanceFrame(0, timeLine);
}

void Phase::advanceFrame(int frame)
{
    QTimeLine* timeLine = dynamic_cast<QTimeLine*>(sender());

    if (!timeLine) {
        //kDebug() << "Phase::advanceFrame found no timeLine!" << endl;
        return;
    }

    advanceFrame(frame, timeLine);
}

void Phase::advanceFrame(int frame, QTimeLine* timeLine)
{
    QMap<QTimeLine*, AnimationState>::iterator it = d->animations.find(timeLine);

    if (it == d->animations.end()) {
        //kDebug() << "Phase::advanceFrame found no entry in animations!" << endl;
        return;
    }

    AnimationState state = it.value();
    qreal progress = state.frames;
    progress = frame / progress;

    switch (state.animation) {
        case Appear:
            d->animator->appear(progress, state.item);
            break;
        case Disappear:
            d->animator->disappear(progress, state.item);
            break;
        case Activate:
            d->animator->activate(progress, state.item);
            break;
        case FrameAppear:
            d->animator->frameAppear(progress, state.item, QRegion()); //FIXME: what -is- the frame region?
            break;
    }
}

void Phase::animationComplete()
{
    QTimeLine* tl = dynamic_cast<QTimeLine*>(sender());

    if (!tl) {
        return;
    }

    QMap<QTimeLine*, AnimationState>::iterator it = d->animations.find(tl);

    if (it == d->animations.end()) {
        return;
    }

    AnimationState state = it.value();

    switch (state.animation) {
        case Appear:
            d->animator->appearCompleted(state.item);
            break;
        case Disappear:
            d->animator->disappearCompleted(state.item);
            break;
        case Activate:
            d->animator->activateCompleted(state.item);
            break;
        case FrameAppear:
            d->animator->frameAppearCompleted(state.item, QRegion()); //FIXME: what -is- the frame region?
            break;
    }

    QMap<QGraphicsItem*, QTimeLine*>::iterator animIt = d->theAnimated.find(state.item);

    if (animIt != d->theAnimated.end()) {
        d->theAnimated.erase(animIt);
    }

    d->animations.erase(it);
    tl->deleteLater();
}

void Phase::render(QGraphicsItem* item, QImage& image, RenderOp op)
{
    Q_UNUSED(item);
    switch (op) {
        case RenderBackground:
            d->animator->renderBackground(image);
            break;
    }
}

Phase::AnimId Phase::startElementAnimation(QGraphicsItem *item, ElementAnimation animation)
{
    //kDebug() << "startElementAnimation(AnimId " << animation << ")" << endl;
    //TODO: allow the animator to define this?
    QTimeLine::CurveShape curveShape = QTimeLine::EaseInOutCurve;

    ElementAnimationState state;
    state.item = item;
    state.animation = animation;
    //TODO: variance in times based on the value of animation
    state.frames = d->animator->elementFrameCount(animation) / 3;
    state.currentFrame = 0;
    state.id = ++d->animId;

    if (state.frames < 1) {
        state.frames = 1;
        state.currentFrame = 1;
        state.timeline = 0;
    } else {
        //TODO: variance in times based on the value of animation
        state.timeline = new QTimeLine(200, this);
        state.timeline->setFrameRange(0, state.frames);
        state.timeline->setCurveShape(curveShape);
        connect(state.timeline, SIGNAL(frameChanged(int)), this, SLOT(advanceElementFrame(int)));
        connect(state.timeline, SIGNAL(finished()), this, SLOT(elementAnimationComplete()));
        state.timeline->start();
    }

    d->elementAnimations[state.timeline] = state.id;
    d->theAnimatedElements[state.id] = state;

    //TODO: this is a bit wasteful; perhaps we should pass in state itself?
    advanceElementFrame(0, state.id);

    //kDebug() << "startElementAnimation(AnimId " << animation << ") returning " << state.id << endl;
    return state.id;
}

void Phase::stopElementAnimation(AnimId id)
{
    //kDebug() << "stopElementAnimation(AnimId " << id << ")" << endl;
    QMap<AnimId, ElementAnimationState>::iterator it = d->theAnimatedElements.find(id);

    if (it == d->theAnimatedElements.end()) {
        return;
    }


    if (it.value().timeline) {
        d->elementAnimations.erase(d->elementAnimations.find(it.value().timeline));
        //delete it.value().timeline;
        it.value().timeline = 0;
    }

    d->theAnimatedElements.erase(it);
    //kDebug() << "stopElementAnimation(AnimId " << id << ") done" << endl;
}

void Phase::setAnimationPixmap(AnimId id, const QPixmap &pixmap)
{
    QMap<AnimId, ElementAnimationState>::iterator it = d->theAnimatedElements.find(id);

    if (it == d->theAnimatedElements.end()) {
        kDebug() << "Phase::setAnimationPixmap(" << id << ") found no entry for it!" << endl;
        return;
    }

    it.value().pixmap = pixmap;
}

QPixmap Phase::animationResult(AnimId id)
{
    QMap<AnimId, ElementAnimationState>::const_iterator it = d->theAnimatedElements.find(id);

    if (it == d->theAnimatedElements.constEnd()) {
        kDebug() << "Phase::animationResult(" << id << ") found no entry for it!" << endl;
        return QPixmap();
    }

    qreal progress = it.value().frames;
    //kDebug() << "Phase::animationResult(" << id <<   " at " << progress  << endl;
    progress = it.value().currentFrame / progress;
    //kDebug() << "Phase::animationResult(" << id <<   " at " << progress  << endl;

    switch (it.value().animation) {
        case ElementAppear:
            return d->animator->elementAppear(progress, it.value().pixmap);
            break;
        case ElementDisappear:
            return d->animator->elementDisappear(progress, it.value().pixmap);
            break;
    }

    return it.value().pixmap;
}

void Phase::advanceElementFrame(int frame)
{
    QTimeLine* timeline = dynamic_cast<QTimeLine*>(sender());

    if (!timeline) {
        kDebug() << "Phase::advanceElementFrame found no timeLine!" << endl;
        return;
    }

    QMap<QTimeLine*, Phase::AnimId>::const_iterator it = d->elementAnimations.find(timeline);

    if (it == d->elementAnimations.constEnd()) {
        kDebug() << "Phase::advanceElementFrame found no entry in animations!" << endl;
        return;
    }

    advanceElementFrame(frame, it.value());
}

void Phase::advanceElementFrame(int frame, AnimId id)
{

    QMap<Phase::AnimId, ElementAnimationState>::iterator it2 = d->theAnimatedElements.find(id);

    if (it2 == d->theAnimatedElements.end()) {
        kDebug() << "Phase::advanceElementFrame found no entry in animations!" << endl;
        return;
    }

    it2.value().currentFrame = frame;
    it2.value().item->update();
}

void Phase::elementAnimationComplete()
{
    QTimeLine* tl = dynamic_cast<QTimeLine*>(sender());

    if (!tl) {
        return;
    }

    QMap<QTimeLine*, Phase::AnimId>::iterator it = d->elementAnimations.find(tl);

    if (it == d->elementAnimations.end()) {
        return;
    }
    //kDebug() << "elementAnimationComplete() " << it.value() << endl;

    QMap<Phase::AnimId, ElementAnimationState>::iterator it2 = d->theAnimatedElements.find(it.value());
    if (it2 != d->theAnimatedElements.end()) {
        it2.value().timeline = 0;
        it2.value().currentFrame = it2.value().frames;
        it2.value().item->update();
    }

    d->elementAnimations.erase(it);
    tl->deleteLater();
}

void Phase::init()
{
    KConfig c("plasmarc");
    KConfigGroup cg(&c, "Phase");
    QString pluginName = cg.readEntry("animator", QString());

    if (!pluginName.isEmpty()) {
        QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(pluginName);
        KService::List offers = KServiceTypeTrader::self()->query("Plasma/Animator", constraint);

        if (!offers.isEmpty()) {
             d->animator = KService::createInstance<Plasma::Animator>(offers.first(), 0, QStringList());
        }
    }

    if (!d->animator) {
        d->animator = new Animator(this);
    }
}

} // namespace Plasma

#include <phase.moc>
