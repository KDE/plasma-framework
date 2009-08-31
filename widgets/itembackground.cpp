/***************************************************************************
 *   Copyright 2009 by Alessandro Diaferia <alediaferia@gmail.com>         *
 *   Copyright 2009 by Marco Martin <notmart@gmail.com>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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
#include <QStyleOptionGraphicsItem>

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

    ItemBackground *q;
    QGraphicsItem *target;
    Plasma::FrameSvg *frameSvg;
    QRectF oldGeometry;
    QRectF newGeometry;
    int animId;
    qreal opacity;
    bool fading;
    bool fadeIn;
    bool immediate;
};

ItemBackground::ItemBackground(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      d(new ItemBackgroundPrivate(this))
{
    d->frameSvg = new Plasma::FrameSvg(this);
    d->animId = 0;
    d->opacity = 1;
    d->fading = false;
    d->fadeIn = false;
    d->immediate = false;
    setContentsMargins(0, 0, 0, 0);

    d->frameSvg->setImagePath("widgets/viewitem");
    d->frameSvg->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    d->frameSvg->setCacheAllRenderedFrames(true);
    d->frameSvg->setElementPrefix("hover");

    setAcceptedMouseButtons(0);
    setZValue(-800);
}

ItemBackground::~ItemBackground()
{
    delete d;
}

void ItemBackground::setTarget(const QRectF &newGeometry)
{
    qreal left, top, right, bottom;
    d->frameSvg->getMargins(left, top, right, bottom);

    d->oldGeometry = geometry();
    d->newGeometry = newGeometry.adjusted(-left, -top, right, bottom);

    if (!isVisible()) {
        setGeometry(d->newGeometry);
        return;
    }

    QGraphicsWidget *pw = parentWidget();
    if (pw) {
        d->newGeometry = d->newGeometry.intersected(QRectF(QPointF(0,0), pw->size()));
    }

    if (d->animId != 0) {
        Plasma::Animator::self()->stopCustomAnimation(d->animId);
    }

    d->fading = false;
    d->opacity = 1;
    d->animId = Plasma::Animator::self()->customAnimation(
        15, 250, Plasma::Animator::EaseInOutCurve, this, "animationUpdate");
}

void ItemBackground::setTargetItem(QGraphicsItem *target)
{
    if (d->target && d->target != target) {
        d->target->removeSceneEventFilter(this);

        QObject *obj = 0;
        if (target->isWidget()) {
            obj = static_cast<QGraphicsWidget*>(target);
        } else {
            obj = dynamic_cast<QObject *>(target);
        }

        if (obj) {
            disconnect(obj, 0, obj, 0);
        }
    }

    if (target) {
        QRectF rect = target->boundingRect();
        rect.moveTopLeft(target->pos());
        setTarget(rect);

        if (d->target != target) {
            d->target = target;

            d->target->installSceneEventFilter(this);

            QObject *obj = 0;
            if (target->isWidget()) {
                obj = static_cast<QGraphicsWidget*>(target);
            } else {
                obj = dynamic_cast<QObject *>(target);
            }

            if (obj) {
                connect(obj, SIGNAL(destroyed(QObject*)), this, SLOT(targetDestroyed(QObject*)));
            }
        }
    } else {
        d->target = 0;
        hide();
    }
}

bool ItemBackground::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    if (watched == d->target) {
        if (event->type() == QEvent::GraphicsSceneResize ||
            event->type() == QEvent::GraphicsSceneMove) {
            setTargetItem(d->target);
        }
    }

    return false;
}

QVariant ItemBackground::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (d->immediate) {
        return value;
    }

    if (change == ItemVisibleChange) {
        bool visible = value.toBool();
        bool retVisible = visible;
        if (visible == isVisible() || d->animId == 0) {
            retVisible = true;
        }
        d->fading = true;
        d->fadeIn = visible;

        if (d->animId != 0) {
            Plasma::Animator::self()->stopCustomAnimation(d->animId);
        }

        d->animId = Plasma::Animator::self()->customAnimation(
                  10, 250, Plasma::Animator::EaseInCurve, this, "animationUpdate");
        return retVisible;
    }
    return value;
}

void ItemBackground::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    if (d->frameSvg->frameSize() != option->rect.size()) {
        d->frameSvg->resizeFrame(option->rect.size());
    }

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

void ItemBackgroundPrivate::animationUpdate(qreal progress)
{
    if (progress == 1) {
        animId = 0;
    }

    if (fading) {
        opacity = fadeIn?progress:1-progress;
        if (!fadeIn && qFuzzyCompare(opacity+1, (qreal)1.0)) {
            immediate = true;
            q->hide();
            immediate = false;
        }
    } else {
        q->setGeometry(oldGeometry.x() + (newGeometry.x() - oldGeometry.x()) * progress,
                       oldGeometry.y() + (newGeometry.y() - oldGeometry.y()) * progress,
                       oldGeometry.width() + (newGeometry.width() - oldGeometry.width()) * progress,
                       oldGeometry.height() + (newGeometry.height() - oldGeometry.height()) * progress);
    }

    q->update();
}

void ItemBackgroundPrivate::targetDestroyed(QObject*)
{
    q->setTargetItem(0);
}

}

#include "itembackground.moc"


