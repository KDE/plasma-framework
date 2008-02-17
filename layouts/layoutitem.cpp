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
              managingLayout(0),
              leftMargin(0),
              rightMargin(0),
              topMargin(0),
              bottomMargin(0)
        {
        }

        ~Private() {}

        Layout* layout;
        Layout* managingLayout;
        QSizeF size;
        qreal leftMargin;
        qreal rightMargin;
        qreal topMargin;
        qreal bottomMargin;
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

QPointF LayoutItem::topLeft() const
{
    return QPointF(0, 0);
}

Layout* LayoutItem::managingLayout() const
{
    return d->managingLayout;
}

qreal LayoutItem::margin(Plasma::MarginEdge edge) const
{
    switch (edge) {
    case LeftMargin:
        return d->leftMargin;
        break;
    case RightMargin:
        return d->rightMargin;
        break;
    case TopMargin:
        return d->topMargin;
        break;
    case BottomMargin:
        return d->bottomMargin;
        break;
    }

    return 0;
}

QRectF LayoutItem::adjustToMargins(const QRectF &rect) const
{
    QRectF r(rect);

    if (r.x() <= d->leftMargin) {
        r.setX(d->leftMargin);
    }

    if (r.y() <= d->topMargin) {
        r.setY(d->topMargin);
    }

    QPointF tl = topLeft();
    qreal maxWidth = d->size.width() + tl.x() - d->leftMargin - d->rightMargin;
    if (r.width() > maxWidth) {
        r.setWidth(maxWidth);
    }

    qreal maxHeight = d->size.height() + tl.y() - d->topMargin - d->bottomMargin;
    if (r.height() > maxHeight) {
        r.setHeight(maxHeight);
    }

    return r;
}

void LayoutItem::setMargin(Plasma::MarginEdge edge, qreal m)
{
    switch (edge) {
    case LeftMargin:
        d->leftMargin = m;
        break;
    case RightMargin:
        d->rightMargin = m;
        break;
    case TopMargin:
        d->topMargin = m;
        break;
    case BottomMargin:
        d->bottomMargin = m;
        break;
    }

    if (d->layout) {
        d->layout->setGeometry(d->layout->geometry());
    }
}

void LayoutItem::setMargin(qreal m)
{
    setMargins(m, m, m, m);
}

void LayoutItem::setMargins(qreal left, qreal top, qreal right, qreal bottom)
{
    d->leftMargin = left;
    d->rightMargin = right;
    d->topMargin = top;
    d->bottomMargin = bottom;

    if (d->layout) {
        d->layout->setGeometry(d->layout->geometry());
    }
}

QSizeF LayoutItem::size() const
{
    return d->size;
}

void LayoutItem::setSize(const QSizeF &size)
{
    d->size = size;
}

}
