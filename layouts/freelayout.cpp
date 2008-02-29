/*
 *   Copyright 2007 by Robert Knight <robertknight@gmail.com>
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

#include "freelayout.h"

#include <KDebug>

namespace Plasma
{

class FreeLayout::Private
{
public:
    QList<QGraphicsLayoutItem*> children;
};

FreeLayout::FreeLayout(QGraphicsLayoutItem *parent)
    : QGraphicsLayout(parent),
      d(new Private)
{
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding,QSizePolicy::DefaultType);
}

FreeLayout::~FreeLayout()
{
    delete d;
}

void FreeLayout::addItem(QGraphicsLayoutItem *item)
{
    if (d->children.contains(item)) {
        return;
    }

    d->children << item;
}

void FreeLayout::removeItem(QGraphicsLayoutItem *item)
{
    if (!item) {
        return;
    }

    d->children.removeAll(item);
}

int FreeLayout::indexOf(QGraphicsLayoutItem *item) const
{
    return d->children.indexOf(item);
}

QGraphicsLayoutItem * FreeLayout::itemAt(int i) const
{
    return d->children[i];
}

int FreeLayout::count() const
{
    return d->children.count();
}

QGraphicsLayoutItem * FreeLayout::takeAt(int i)
{
    return d->children.takeAt(i);
}

void FreeLayout::removeAt(int i)
{
    d->children.removeAt(i);
}

void FreeLayout::relayout()
{
    foreach (QGraphicsLayoutItem *child , d->children) {
        if (child->geometry().size() != child->effectiveSizeHint(Qt::PreferredSize)) {
            const QSizeF newSize = child->effectiveSizeHint(Qt::PreferredSize).expandedTo(minimumSize()).boundedTo(maximumSize());
            child->setGeometry(QRectF(child->geometry().topLeft(), newSize));
        }
    }
}

QRectF FreeLayout::geometry() const
{
    if (parentLayoutItem()) {
        return parentLayoutItem()->geometry();
    }

    return QRectF(QPointF(0, 0), maximumSize());
}

void FreeLayout::setGeometry(const QRectF &geom)
{
    if (!geom.isValid() || geom == geometry()) {
        return;
    }

//     QRectF newGeom = geom;
// 
//     if (d->parent && !dynamic_cast<QGraphicsLayout*>(d->parent)) {
//         newGeom = d->parent->adjustToMargins(newGeom);
//         //kDebug() << "parent rect is" << d->parent->topLeft() << d->parent->size()
//         //         << "and we are" << geometry() << "but aiming for"
//         //         << newGeom << "from" << geom;
//     }
// 
//     d->pos = newGeom.topLeft();
    setPreferredSize(geom.size());
    // TODO: respect minimum and maximum sizes: is it possible?
    //setSize(newGeom.size().expandedTo(minimumSize()).boundedTo(maximumSize()));

    //kDebug() << "geometry is now" << geometry();
    invalidate();
}

QSizeF FreeLayout::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    if (parentLayoutItem()) {
        //kDebug() << "returning size hint from freelayout of" <<  parent()->geometry().size();
        return parentLayoutItem()->geometry().size();
    }

    //kDebug() << "returning size hint from freelayout of" << maximumSize();
    return maximumSize();
}

}

