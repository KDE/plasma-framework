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
};

class Phase::Private
{
    public:

        Private()
            : animator(0)
        {
        }

        ~Private()
        {
            // delete animator; Animator is a QObject
            // TimeLine's are parented to us, and we don't own the items
        }

        Animator* animator;

        //TODO: eventually perhaps we should allow multiple animations simulataneously
        //      which would imply changing this to a QMap<QGraphicsItem*, QList<QTimeLine*> >
        //      and really making the code fun ;)
        QMap<QGraphicsItem*, QTimeLine*> theAnimated;
        QMap<QTimeLine*, AnimationState> animations;
};

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

    delete it.value();
    d->animations.erase(d->animations.find(it.value()));
    d->theAnimated.erase(it);
}

void Phase::animate(QGraphicsItem* item, Animation animation)
{
    QMap<QGraphicsItem*, QTimeLine*>::iterator it = d->theAnimated.find(item);

    if (it != d->theAnimated.end()) {
        delete it.value();
        d->animations.erase(d->animations.find(it.value()));
    }

    int frames = 0;
    switch (animation) {
        case Appear:
            frames = d->animator->appearFrames();
            break;
        case Disappear:
            frames = d->animator->disappearFrames();
            break;
        case Activate:
            frames = d->animator->activateFrames();
            break;
        case FrameAppear:
            frames = d->animator->activateFrames();
            break;
    }

    if (frames < 1) {
        return;
    }

    QTimeLine* timeLine = new QTimeLine(300, this);
    timeLine->setFrameRange(0, frames);

    AnimationState state;
    state.item = item;
    state.animation = animation;
    d->animations[timeLine] = state;
    d->theAnimated[item] = timeLine;
    connect(timeLine, SIGNAL(frameChanged(int)), this, SLOT(advanceFrame(int)));
    connect(timeLine, SIGNAL(finished()), this, SLOT(animationComplete()));
}

void Phase::advanceFrame(int frame)
{
    QTimeLine* timeLine = dynamic_cast<QTimeLine*>(sender());

    if (!timeLine) {
        return;
    }

    QMap<QTimeLine*, AnimationState>::iterator it = d->animations.find(timeLine);

    if (it == d->animations.end()) {
        return;
    }

    AnimationState state = it.value();

    switch (state.animation) {
        case Appear:
            d->animator->appear(frame, state.item);
            break;
        case Disappear:
            d->animator->disappear(frame, state.item);
            break;
        case Activate:
            d->animator->activate(frame, state.item);
            break;
        case FrameAppear:
            d->animator->frameAppear(frame, state.item, QRegion()); //FIXME: what -is- the frame region?
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

    QMap<QGraphicsItem*, QTimeLine*>::iterator animIt = d->theAnimated.find(it.value().item);

    if (animIt != d->theAnimated.end()) {
        d->theAnimated.erase(animIt);
    }

    d->animations.erase(it);
    delete tl;
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
