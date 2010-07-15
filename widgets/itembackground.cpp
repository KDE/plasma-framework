/***************************************************************************
 *   Copyright 2009 by Alessandro Diaferia <alediaferia@gmail.com>         *
 *   Copyright 2009 by Marco Martin <notmart@gmail.com>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2, or       *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/
#include "itembackground.h"

#include <QPainter>
#include <QTimer>
#include <QStyleOptionGraphicsItem>

#include <QPropertyAnimation>

#include <kdebug.h>

#include <plasma/framesvg.h>
#include <plasma/animator.h>
#include <plasma/theme.h>
namespace Plasma
{

class ItemBackgroundPrivate
{
public:
    ItemBackgroundPrivate(ItemBackground *parent)
        : q(parent),
          target(0)
    {}

    void animationUpdate(qreal progress);
    void targetDestroyed(QObject*);
    void frameSvgChanged();
    void refreshCurrentTarget();

    ItemBackground * const q;
    QGraphicsItem *target;
    Plasma::FrameSvg *frameSvg;
    QRectF oldGeometry;
    QRectF newGeometry;
    QPropertyAnimation *anim;
    qreal opacity;
    bool fading;
    bool fadeIn;
    bool immediate;
};

ItemBackground::ItemBackground(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      d(new ItemBackgroundPrivate(this))
{
    setCacheMode(DeviceCoordinateCache);
    setFlag(ItemIsMovable, false);
    setFlag(ItemIsSelectable, false);
    setFlag(ItemIsFocusable, false);

    setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);

    d->frameSvg = new Plasma::FrameSvg(this);
    d->anim = new QPropertyAnimation(this, "animationUpdate", this);
    d->anim->setStartValue(0);
    d->anim->setEndValue(1);
    d->opacity = 1;
    d->fading = false;
    d->fadeIn = false;
    d->immediate = false;

    d->frameSvg->setImagePath("widgets/viewitem");
    d->frameSvg->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    d->frameSvg->setCacheAllRenderedFrames(true);
    d->frameSvg->setElementPrefix("hover");

    qreal l, t, r, b;
    d->frameSvg->getMargins(l, t, r, b);
    setContentsMargins(l, t, r, b);

    connect(d->frameSvg, SIGNAL(repaintNeeded()), this, SLOT(frameSvgChanged()));

    setAcceptedMouseButtons(0);
    setZValue(-800);
}

ItemBackground::~ItemBackground()
{
    delete d;
}

QRectF ItemBackground::target() const
{
    return d->newGeometry;
}

void ItemBackground::setTarget(const QRectF &newGeometry)
{
    d->oldGeometry = geometry();
    d->newGeometry = newGeometry;

    if (!isVisible() && (!d->target || !d->target->isVisible())) {
        setGeometry(d->newGeometry);
        targetReached(newGeometry);
        if (d->target) {
            emit targetItemReached(d->target);
        }
        return;
    }

    QGraphicsWidget *pw = parentWidget();
    if (pw) {
        d->newGeometry = d->newGeometry.intersected(QRectF(QPointF(0,0), pw->size()));
    }

    if (d->anim->state() != QAbstractAnimation::Stopped) {
        d->anim->stop();
    }

    if (d->target && d->target->isVisible() && !isVisible()) {
        setZValue(d->target->zValue()-1);
        setGeometry(newGeometry);
        d->oldGeometry = newGeometry;
        show();
    } else {
        d->fading = false;
        d->opacity = 1;
        d->anim->start();
    }

}

void ItemBackground::setTargetItem(QGraphicsItem *target)
{
    if (d->target && d->target != target) {
        QObject *obj = 0;
        if (d->target->isWidget()) {
            obj = static_cast<QGraphicsWidget*>(d->target);
            obj->removeEventFilter(this);
        } else {
            d->target->removeSceneEventFilter(this);
            obj = dynamic_cast<QObject *>(d->target);
        }

        if (obj) {
            disconnect(obj, 0, this, 0);
        }
    }

    if (!target) {
        hide();
    }

    bool newTarget = (d->target != target);
    d->target = target;
    if (target) {
        setZValue(target->zValue() - 1);
        if (parentItem() != target->parentItem()) {
            QTransform t = transform();
            setTransform(QTransform());
            QRectF geom = mapToScene(geometry()).boundingRect();
            setGeometry(mapFromScene(geom).boundingRect());
            setTransform(t);
        }

        QRectF rect = target->boundingRect();
        rect.moveTopLeft(mapToParent(mapFromScene(target->mapToScene(QPointF(0, 0)))));

        setTarget(rect);


        if (newTarget) {
            QObject *obj = 0;
            if (target->isWidget()) {
                obj = static_cast<QGraphicsWidget*>(target);
                obj->installEventFilter(this);
            } else {
                d->target->installSceneEventFilter(this);
                obj = dynamic_cast<QObject *>(target);
            }

            if (obj) {
                connect(obj, SIGNAL(destroyed(QObject*)), this, SLOT(targetDestroyed(QObject*)));
            }
        }
    }
}

QGraphicsItem *ItemBackground::targetItem() const
{
    return d->target;
}

bool ItemBackground::eventFilter(QObject *watched, QEvent *event)
{
    QGraphicsWidget *targetWidget = static_cast<QGraphicsWidget *>(d->target);
    if (watched == targetWidget) {
        if (event->type() == QEvent::GraphicsSceneResize ||
            event->type() == QEvent::GraphicsSceneMove) {
            // We need to wait for the parent widget to resize...
            QTimer::singleShot(0, this, SLOT(refreshCurrentTarget()) );
        } else if (event->type() == QEvent::Show) {
            setTargetItem(targetWidget);
        }
    }

    return false;
}

bool ItemBackground::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    if (watched == d->target) {
        if (event->type() == QEvent::GraphicsSceneMove) {
            QTimer::singleShot(0, this, SLOT(refreshCurrentTarget()) );
        }
    }

    return false;
}

void ItemBackground::resizeEvent(QGraphicsSceneResizeEvent *)
{
    d->frameSvg->resizeFrame(size());
}

QVariant ItemBackground::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (d->immediate) {
        return value;
    }

    if (change == ItemVisibleChange) {
        bool visible = value.toBool();
        bool retVisible = visible;
        if (visible == isVisible() || d->anim->state() == QAbstractAnimation::Stopped) {
            retVisible = true;
        }
        d->fading = true;
        d->fadeIn = visible;

        if (d->anim->state() != QAbstractAnimation::Stopped) {
            d->anim->stop();
        }

        d->anim->setDuration(250);
        d->anim->start();

        return retVisible;

    }
    return value;
}

void ItemBackground::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    if (qFuzzyCompare(d->opacity, (qreal)1.0)) {
        d->frameSvg->paintFrame(painter, option->rect.topLeft());
    } else if (qFuzzyCompare(d->opacity+1, (qreal)1.0)) {
        return;
    } else {
        QPixmap framePix = d->frameSvg->framePixmap();
        QPainter bufferPainter(&framePix);
        bufferPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        bufferPainter.fillRect(framePix.rect(), QColor(0, 0, 0, 255 * d->opacity));
        bufferPainter.end();
        painter->drawPixmap(framePix.rect(), framePix, framePix.rect());
    }
}

void ItemBackground::setAnimationUpdate(qreal progress)
{
    d->animationUpdate(progress);
}

qreal ItemBackground::animationUpdate() const
{
    return d->opacity;
}

void ItemBackgroundPrivate::animationUpdate(qreal progress)
{
    if (progress == 1) {
        if ((!fading) || (fadeIn)) {
            emit q->targetReached(newGeometry);
            if (target) {
                emit q->targetItemReached(target);
            }
        }
    }

    if (fading) {
        opacity = fadeIn?progress:1-progress;
        if (!fadeIn && qFuzzyCompare(opacity+1, (qreal)1.0)) {
            immediate = true;
            q->hide();
            immediate = false;
        }
    } else if (oldGeometry != newGeometry) {
        q->setGeometry(oldGeometry.x() + (newGeometry.x() - oldGeometry.x()) * progress,
                       oldGeometry.y() + (newGeometry.y() - oldGeometry.y()) * progress,
                       oldGeometry.width() + (newGeometry.width() - oldGeometry.width()) * progress,
                       oldGeometry.height() + (newGeometry.height() - oldGeometry.height()) * progress);
    }

    q->update();
    emit q->animationStep(progress);
}

void ItemBackgroundPrivate::targetDestroyed(QObject*)
{
    target = 0;
    q->setTargetItem(0);
}

void ItemBackgroundPrivate::frameSvgChanged()
{
    qreal l, t, r, b;
    frameSvg->getMargins(l, t, r, b);
    q->setContentsMargins(l, t, r, b);
    q->update();
    emit q->appearanceChanged();
}

void ItemBackgroundPrivate::refreshCurrentTarget()
{
    q->setTargetItem(target);
}

} // Plasma namespace


#include "itembackground.moc"


