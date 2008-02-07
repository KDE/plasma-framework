/*
 *   Copyright 2007 by Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
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

#include "layoutitem.h"

#include <KDebug>

#include "layout.h"

namespace Plasma
{

class LayoutItem::Private
{
    public:
        Private()
            : layout(0),
              managingLayout(0)
        {
        }

        ~Private() {}

        Layout* layout;
        Layout* managingLayout;
};


LayoutItem::LayoutItem()
    : d(new Private())
{
}

LayoutItem::~LayoutItem()
{
    if (d->managingLayout) {
        d->managingLayout->removeItem(this);
    }

    delete d->layout;
    delete d;
}

QGraphicsItem* LayoutItem::graphicsItem() 
{
    return 0;
}

bool LayoutItem::hasHeightForWidth() const
{
    return false;
}

qreal LayoutItem::heightForWidth(qreal w) const
{
    Q_UNUSED (w);
    return 0.0;
}

bool LayoutItem::hasWidthForHeight() const
{
    return false;
}

qreal LayoutItem::widthForHeight(qreal h) const
{
    Q_UNUSED (h);
    return 0.0;
}

void LayoutItem::setLayout(Layout* layout)
{
    if (d->layout == layout) {
        return;
    }

    delete d->layout;
    d->layout = layout;

    if (layout) {
        layout->setParent(this);
    }
}

void LayoutItem::unsetLayout()
{
    d->layout = 0;
}

Layout* LayoutItem::layout() const
{
    return d->layout;
}

void LayoutItem::setManagingLayout(Layout* layout)
{
    if (layout == d->managingLayout) {
        return;
    }

    if (d->managingLayout) {
        d->managingLayout->removeItem(this);
    }

    d->managingLayout = layout;
    managingLayoutChanged();
}

void LayoutItem::unsetManagingLayout(Layout* layout)
{
    if (d->managingLayout == layout) {
        d->managingLayout = 0;
    }
    managingLayoutChanged();
}

void LayoutItem::managingLayoutChanged()
{
}

Layout* LayoutItem::managingLayout() const
{
    return d->managingLayout;
}

}
