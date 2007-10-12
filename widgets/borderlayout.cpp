/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "borderlayout.h"

namespace Plasma {

class BorderLayout::Private {
public:
    BorderLayout * q;

    explicit Private(BorderLayout * parent = 0)
        : q(parent)
    {
        sizes[LeftPositioned] = -1;
        sizes[RightPositioned] = -1;
        sizes[TopPositioned] = -1;
        sizes[BottomPositioned] = -1;
        sizes[CenterPositioned] = -1;

    }

    virtual ~Private()
    {
    }

    QMap< Position, LayoutItem * > itemPositions;
    QRectF geometry;
    QMap< Position, qreal > sizes;
};



BorderLayout::BorderLayout(LayoutItem * parent) :
    Layout(parent), d(new Private(this))
{
    if (parent) {
        parent->setLayout(this);
    }
}

BorderLayout::~BorderLayout()
{
    delete d;
}

Qt::Orientations BorderLayout::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
}

QRectF BorderLayout::geometry() const
{
    return d->geometry;
}

void BorderLayout::setGeometry(const QRectF& geometry)
{
    if (!geometry.isValid() || geometry.isEmpty()) {
        return;
    }

    d->geometry = geometry;

    invalidate();
}

void BorderLayout::invalidate()
{
    QRectF geometry = d->geometry;
    geometry.setTopLeft(geometry.topLeft() + QPointF(margin(), margin()));
    geometry.setBottomRight(geometry.bottomRight() - QPointF(margin(), margin()));

    QPointF origin = geometry.topLeft();
    qreal top, bottom, left, right;
    top    = (d->sizes[TopPositioned] >= 0)    ? d->sizes[TopPositioned]    : 0;
    left   = (d->sizes[LeftPositioned] >= 0)   ? d->sizes[LeftPositioned]   : 0;
    bottom = geometry.height() - ((d->sizes[BottomPositioned] >= 0) ? d->sizes[BottomPositioned] : 0);
    right  = geometry.width()  - ((d->sizes[RightPositioned] >= 0)  ? d->sizes[RightPositioned]  : 0);

    if (d->itemPositions[TopPositioned] /*&& d->itemPositions[TopPositioned]->isVisible()*/) {
        top = (d->sizes[TopPositioned] >= 0) ? d->sizes[TopPositioned] : d->itemPositions[TopPositioned]->sizeHint().height();
        d->itemPositions[TopPositioned]->setGeometry(QRectF(origin, QSizeF(
                geometry.width(), top)));
        top += spacing();
    }

    if (d->itemPositions[BottomPositioned] /*&& d->itemPositions[BottomPositioned]->isVisible()*/) {
        bottom = (d->sizes[BottomPositioned] >= 0) ? d->sizes[BottomPositioned]
                : d->itemPositions[BottomPositioned]->sizeHint().height();
        d->itemPositions[BottomPositioned]->setGeometry(QRectF(origin + QPointF(0,
                geometry.height() - bottom), QSizeF(geometry.width(),
                bottom)));
        bottom = geometry.height() - bottom - spacing();
    }

    if (d->itemPositions[LeftPositioned] /*&& d->itemPositions[LeftPositioned]->isVisible()*/) {
        left = (d->sizes[LeftPositioned] >= 0) ? d->sizes[LeftPositioned] : d->itemPositions[LeftPositioned]->sizeHint().width();
        d->itemPositions[LeftPositioned]->setGeometry(QRectF(origin + QPointF(0, top),
                QSizeF(left, bottom - top)));
        left += spacing();
    }

    if (d->itemPositions[RightPositioned] /*&& d->itemPositions[RightPositioned]->isVisible()*/) {
        right = (d->sizes[RightPositioned] >= 0) ? d->sizes[RightPositioned] : d->itemPositions[RightPositioned]->sizeHint().width();
        d->itemPositions[RightPositioned]->setGeometry(QRectF(origin + QPointF(
                geometry.width() - right, top), QSizeF(right, bottom - top)));
        right = geometry.width() - right - spacing();
    }

    if (d->itemPositions[CenterPositioned] /*&& d->itemPositions[CenterPositioned]->isVisible()*/) {
        d->itemPositions[CenterPositioned]->setGeometry(QRectF(
                origin + QPointF(left, top), QSizeF(right - left, bottom - top)));
    }

}

QSizeF BorderLayout::sizeHint() const
{
    qreal hintHeight = 0.0;
    qreal hintWidth = 0.0;

    if (d->itemPositions[TopPositioned] /*&& d->itemPositions[TopPositioned]->isVisible()*/) {
        hintHeight += d->itemPositions[TopPositioned]->sizeHint().height();
    }

    if (d->itemPositions[BottomPositioned] /*&& d->itemPositions[BottomPositioned]->isVisible()*/) {
        hintHeight += d->itemPositions[BottomPositioned]->sizeHint().height();
    }

    if (d->itemPositions[LeftPositioned] /*&& d->itemPositions[LeftPositioned]->isVisible()*/) {
        hintWidth += d->itemPositions[LeftPositioned]->sizeHint().width();
    }

    if (d->itemPositions[RightPositioned] /*&& d->itemPositions[RightPositioned]->isVisible()*/) {
        hintWidth += d->itemPositions[RightPositioned]->sizeHint().width();
    }

    if (d->itemPositions[CenterPositioned] /*&& d->itemPositions[CenterPositioned]->isVisible()*/) {
        hintHeight += d->itemPositions[CenterPositioned]->sizeHint().height();
        hintWidth += d->itemPositions[CenterPositioned]->sizeHint().width();
    }

    return QSizeF(hintWidth + 2 * margin(), hintHeight + 2 * margin());
}

void BorderLayout::addItem(Plasma::LayoutItem * item)
{
    BorderLayout::addItem (item, CenterPositioned);
}

void BorderLayout::addItem(Plasma::LayoutItem * item, Position position)
{
    d->itemPositions[position] = item;
    update();
}

void BorderLayout::removeItem(LayoutItem * item)
{
    QMutableMapIterator< Position, Plasma::LayoutItem * > i(d->itemPositions);
    while (i.hasNext()) {
        i.next();
        if (i.value() == item) {
            i.remove();
        }
    }
    update();
}

int BorderLayout::count() const
{
    int count = 0;
    foreach (Plasma::LayoutItem * i, d->itemPositions) {
        if (i) {
            ++count;
        }
    }
    return count;
}

int BorderLayout::indexOf(LayoutItem * item) const
{
    int count = 0;
    foreach (Plasma::LayoutItem * i, d->itemPositions) {
        if (i) {
            if (item == i) {
                return count;
            }
            ++count;
        }
    }
    return -1;
}

LayoutItem * BorderLayout::itemAt(int index) const
{
    int count = 0;
    foreach (Plasma::LayoutItem * i, d->itemPositions) {
        if (i) {
            if (index == count) {
                return i;
            }
            count++;

        }
    }

    return 0;
}

Plasma::LayoutItem * BorderLayout::takeAt(int i)
{
    Plasma::LayoutItem * item = itemAt(i);
    removeItem(item);
    return item;
}

void BorderLayout::setSize(qreal size, Position border)
{
    d->sizes[border] = size;
    update();
}

void BorderLayout::setAutoSize(Position border)
{
    d->sizes[border] = -1;
    update();
}

qreal BorderLayout::size(Position border)
{
    if (border == CenterPositioned) {
        return -1;
    }
    return d->sizes[border];
}

}
