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
    ItemBackgroundPrivate()
    {}
    
    Plasma::FrameSvg *frameSvg;
    QRectF oldGeometry;
    QRectF newGeometry;
    int animId;
    qreal opacity;
    bool fading;
    bool fadeIn;
};

ItemBackground::ItemBackground(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      d(new ItemBackgroundPrivate)
{
    d->frameSvg = new Plasma::FrameSvg(this);
    d->animId = 0;
    d->opacity = 1;
    d->fading = false;
    d->fadeIn = false;
    setContentsMargins(0, 0, 0, 0);

    d->frameSvg->setImagePath("widgets/viewitem");
    d->frameSvg->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    d->frameSvg->setCacheAllRenderedFrames(true);
    d->frameSvg->setElementPrefix("hover");

    setAcceptedMouseButtons(0);
    setZValue(-800);
}

ItemBackground::~ItemBackground()
{}

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
    QRectF rect = target->boundingRect();
    rect.moveTopLeft(target->pos());
    setTarget(rect);
}

QVariant ItemBackground::itemChange(GraphicsItemChange change, const QVariant &value)
{
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

void ItemBackground::animationUpdate(qreal progress)
{
    if (progress == 1) {
        d->animId = 0;
    }

    if (d->fading) {
        d->opacity = d->fadeIn?progress:1-progress;
        if (!d->fadeIn && qFuzzyCompare(d->opacity+1, (qreal)1.0)) {
            hide();
        }
    } else {
        setGeometry(d->oldGeometry.x() + (d->newGeometry.x() - d->oldGeometry.x()) * progress,
                    d->oldGeometry.y() + (d->newGeometry.y() - d->oldGeometry.y()) * progress,

                    d->oldGeometry.width() + (d->newGeometry.width() - d->oldGeometry.width()) * progress,
                    d->oldGeometry.height() + (d->newGeometry.height() - d->oldGeometry.height()) * progress);
    }
    update();
}

}

