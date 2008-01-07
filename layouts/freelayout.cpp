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

#include <plasma/layouts/layout.h>

namespace Plasma
{

class FreeLayout::Private
{
public:
    QList<LayoutItem*> children;
};

FreeLayout::FreeLayout(LayoutItem *parent)
    : Layout(parent),
      d(new Private)
{
}

FreeLayout::~FreeLayout()
{
    delete d;
}

Qt::Orientations FreeLayout::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
}

void FreeLayout::addItem(LayoutItem *item)
{
    if (d->children.contains(item)) {
        return;
    }

    d->children << item;
    item->setManagingLayout(this);
}

void FreeLayout::removeItem(LayoutItem *item)
{
    if (!item) {
        return;
    }

    d->children.removeAll(item);
    item->unsetManagingLayout(this);
}

int FreeLayout::indexOf(LayoutItem *item) const
{
    return d->children.indexOf(item);
}

LayoutItem * FreeLayout::itemAt(int i) const
{
    return d->children[i];
}

int FreeLayout::count() const
{
    return d->children.count();
}

LayoutItem * FreeLayout::takeAt(int i)
{
    return d->children.takeAt(i);
}

void FreeLayout::relayout()
{
    foreach (LayoutItem *child , d->children) {
        if (child->geometry().size() != child->sizeHint()) {
            QSizeF newSize = child->sizeHint().expandedTo(minimumSize()).boundedTo(maximumSize());
            child->setGeometry(QRectF(child->geometry().topLeft(), newSize));
        }
    }
}

QRectF FreeLayout::geometry() const
{
    if (parent()) {
        return parent()->geometry();
    }

    return QRectF(QPointF(0, 0), maximumSize());
}

QSizeF FreeLayout::sizeHint() const
{
    if (parent()) {
        //kDebug(1209) << "returning size hint from freelayout of" <<  parent()->geometry().size();
        return parent()->geometry().size();
    }

    //kDebug(1209) << "returning size hint from freelayout of" << maximumSize();
    return maximumSize();
}

}

