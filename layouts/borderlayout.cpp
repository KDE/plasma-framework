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
        : q(parent),
          spacing(6.0)
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

    QMap< Position, QGraphicsLayoutItem * > itemPositions;
    QMap< Position, qreal > sizes;
    qreal spacing;
};



BorderLayout::BorderLayout(QGraphicsLayoutItem * parent) 
    : QGraphicsLayout(parent),
      d(new Private(this))
{
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding,QSizePolicy::DefaultType);
}

BorderLayout::~BorderLayout()
{
    delete d;
}

void BorderLayout::relayout()
{
    QRectF rect = geometry();
    qreal leftMargin, topMargin, rightMargin, bottomMargin;
    getContentsMargins(&leftMargin,&topMargin,&rightMargin,&bottomMargin);
    rect.setTopLeft(rect.topLeft() + QPointF(leftMargin,topMargin));
    rect.setBottomRight(rect.bottomRight() - QPointF(rightMargin,bottomMargin));

    QPointF origin = rect.topLeft();
    qreal top, bottom, left, right;
    top    = (d->sizes[TopPositioned] >= 0)    ? d->sizes[TopPositioned]    : 0;
    left   = (d->sizes[LeftPositioned] >= 0)   ? d->sizes[LeftPositioned]   : 0;
    bottom = rect.height() - ((d->sizes[BottomPositioned] >= 0) ? d->sizes[BottomPositioned] : 0);
    right  = rect.width()  - ((d->sizes[RightPositioned] >= 0)  ? d->sizes[RightPositioned]  : 0);

    if (d->itemPositions[TopPositioned] /*&& d->itemPositions[TopPositioned]->isVisible()*/) {
        top = (d->sizes[TopPositioned] >= 0) ? d->sizes[TopPositioned] : d->itemPositions[TopPositioned]->effectiveSizeHint(Qt::PreferredSize).height();
        d->itemPositions[TopPositioned]->setGeometry(QRectF(origin, QSizeF(
                rect.width(), top)));
        top += spacing();
    }

    if (d->itemPositions[BottomPositioned] /*&& d->itemPositions[BottomPositioned]->isVisible()*/) {
        bottom = (d->sizes[BottomPositioned] >= 0) ? d->sizes[BottomPositioned]
                : d->itemPositions[BottomPositioned]->effectiveSizeHint(Qt::PreferredSize).height();
        d->itemPositions[BottomPositioned]->setGeometry(QRectF(origin + QPointF(0,
                rect.height() - bottom), QSizeF(rect.width(),
                bottom)));
        bottom = rect.height() - bottom - spacing();
    }

    if (d->itemPositions[LeftPositioned] /*&& d->itemPositions[LeftPositioned]->isVisible()*/) {
        left = (d->sizes[LeftPositioned] >= 0) ? d->sizes[LeftPositioned] : d->itemPositions[LeftPositioned]->effectiveSizeHint(Qt::PreferredSize).width();
        d->itemPositions[LeftPositioned]->setGeometry(QRectF(origin + QPointF(0, top),
                QSizeF(left, bottom - top)));
        left += spacing();
    }

    if (d->itemPositions[RightPositioned] /*&& d->itemPositions[RightPositioned]->isVisible()*/) {
        right = (d->sizes[RightPositioned] >= 0) ? d->sizes[RightPositioned] : d->itemPositions[RightPositioned]->effectiveSizeHint(Qt::PreferredSize).width();
        d->itemPositions[RightPositioned]->setGeometry(QRectF(origin + QPointF(
                rect.width() - right, top), QSizeF(right, bottom - top)));
        right = rect.width() - right - spacing();
    }

    if (d->itemPositions[CenterPositioned] /*&& d->itemPositions[CenterPositioned]->isVisible()*/) {
        d->itemPositions[CenterPositioned]->setGeometry(QRectF(
                origin + QPointF(left, top), QSizeF(right - left, bottom - top)));
    }
}

QSizeF BorderLayout::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    qreal hintHeight = 0.0;
    qreal hintWidth = 0.0;

    if (d->itemPositions[TopPositioned] /*&& d->itemPositions[TopPositioned]->isVisible()*/) {
        hintHeight += d->itemPositions[TopPositioned]->effectiveSizeHint(Qt::PreferredSize).height();
    }

    if (d->itemPositions[BottomPositioned] /*&& d->itemPositions[BottomPositioned]->isVisible()*/) {
        hintHeight += d->itemPositions[BottomPositioned]->effectiveSizeHint(Qt::PreferredSize).height();
    }

    if (d->itemPositions[LeftPositioned] /*&& d->itemPositions[LeftPositioned]->isVisible()*/) {
        hintWidth += d->itemPositions[LeftPositioned]->effectiveSizeHint(Qt::PreferredSize).width();
    }

    if (d->itemPositions[RightPositioned] /*&& d->itemPositions[RightPositioned]->isVisible()*/) {
        hintWidth += d->itemPositions[RightPositioned]->effectiveSizeHint(Qt::PreferredSize).width();
    }

    if (d->itemPositions[CenterPositioned] /*&& d->itemPositions[CenterPositioned]->isVisible()*/) {
        hintHeight += d->itemPositions[CenterPositioned]->effectiveSizeHint(Qt::PreferredSize).height();
        hintWidth += d->itemPositions[CenterPositioned]->effectiveSizeHint(Qt::PreferredSize).width();
    }

    qreal leftMargin, topMargin, rightMargin, bottomMargin;
    getContentsMargins(&leftMargin,&topMargin,&rightMargin,&bottomMargin);
    return QSizeF(hintWidth + 2 + leftMargin + rightMargin, hintHeight + 2 + topMargin + bottomMargin);
}

void BorderLayout::addItem(QGraphicsLayoutItem * item)
{
    BorderLayout::addItem (item, CenterPositioned);
}

void BorderLayout::addItem(QGraphicsLayoutItem * item, Position position)
{
    removeItem(item);
    d->itemPositions[position] = item;
    updateGeometry();
}

void BorderLayout::removeItem(QGraphicsLayoutItem * item)
{
    QMutableMapIterator< Position, QGraphicsLayoutItem * > i(d->itemPositions);
    while (i.hasNext()) {
        i.next();
        if (i.value() == item) {
            i.remove();
        }
    }
    updateGeometry();
}

int BorderLayout::count() const
{
    int count = 0;
    foreach (QGraphicsLayoutItem * i, d->itemPositions) {
        if (i) {
            ++count;
        }
    }
    return count;
}

int BorderLayout::indexOf(QGraphicsLayoutItem * item) const
{
    int count = 0;
    foreach (QGraphicsLayoutItem * i, d->itemPositions) {
        if (i) {
            if (item == i) {
                return count;
            }
            ++count;
        }
    }
    return -1;
}

QGraphicsLayoutItem * BorderLayout::itemAt(int index) const
{
    int count = 0;
    foreach (QGraphicsLayoutItem * i, d->itemPositions) {
        if (i) {
            if (index == count) {
                return i;
            }
            count++;

        }
    }

    return 0;
}

QGraphicsLayoutItem * BorderLayout::takeAt(int i)
{
    QGraphicsLayoutItem * item = itemAt(i);
    removeItem(item);
    return item;
}

void BorderLayout::setSize(qreal size, Position border)
{
    d->sizes[border] = size;
    updateGeometry();
}

void BorderLayout::setAutoSize(Position border)
{
    d->sizes[border] = -1;
    updateGeometry();
}

qreal BorderLayout::size(Position border)
{
    if (border == CenterPositioned) {
        return -1;
    }
    return d->sizes[border];
}

qreal BorderLayout::spacing() const
{
    return d->spacing;
}

void BorderLayout::setSpacing(qreal s)
{
    d->spacing = s;
}

}
