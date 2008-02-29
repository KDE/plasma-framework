/*
*   Copyright 2007 by Robert Knight <robertknight@gmail.com>
*   Copyright 2008 by William Egert <begert@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License
*   as published by the Free Software Foundation; either
*   version 2 of the License, or (at your option) any later version.
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

#include "flowlayout.h"

#include <limits.h>
#include <math.h>

#include <QtCore/QList>
#include <QtCore/QRectF>
#include <QtCore/QTimeLine>

#include <KDebug>

// #include "layoutanimator.h"

using namespace Plasma;

class FlowLayout::Private
{
public:
    Private() : columnWidth( -1 ), spacing(6.0) {}
    QList<QGraphicsLayoutItem*> items; 
    qreal columnWidth;
    qreal spacing;
};

FlowLayout::FlowLayout(QGraphicsLayoutItem* parent)
    : QGraphicsLayout(parent)
    , d(new Private)
{
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding,QSizePolicy::DefaultType);
}

FlowLayout::~FlowLayout()
{
    delete d;
}

int FlowLayout::count() const
{
    return d->items.count();
}

void FlowLayout::addItem(QGraphicsLayoutItem* item)
{
    if (!item || d->items.contains(item)) {
        return;
    }

    d->items << item;
//FIXME: Port
//     if (animator()) {
//         animator()->setCurrentState(item,LayoutAnimator::InsertedState);
//     }

    updateGeometry();
}
void FlowLayout::removeItem(QGraphicsLayoutItem* item)
{
    if (!item) {
        return;
    }

    d->items.removeAll(item);
//FIXME: Port
//     if (animator()) {
//         animator()->setCurrentState(item,LayoutAnimator::RemovedState);
//     }

    updateGeometry();
}
int FlowLayout::indexOf(QGraphicsLayoutItem* item) const
{
    if (!item) {
        return -1;
    }

    return d->items.indexOf(item);
}
QGraphicsLayoutItem* FlowLayout::itemAt(int i) const
{
    if (i >= d->items.count()) {
        return 0;
    }

    return d->items[i];
}

QSizeF FlowLayout::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    // TODO A proper algorithm here
    // 
    // Idea:  Return a size hint based on the golden ratio to
    //        make it aesthetically good
    //        eg. Longer side is 1.61x the length of the shorter side
    //

    // testing
    return QSizeF(500,500);
}

QGraphicsLayoutItem* FlowLayout::takeAt(int i)
{
    if (i >= d->items.count()) {
        return 0;
    }

    return d->items.takeAt(i);
    // FIXME: Should updateGeometry() be called?
}

void FlowLayout::removeAt(int i)
{
    if (i >= d->items.count()) {
        return;
    }

    d->items.removeAt(i);
}

QRectF FlowLayout::geometry() const
{
    if (parentLayoutItem()) {
        return parentLayoutItem()->geometry();
    }

    return QRectF(QPointF(0, 0), maximumSize());
}

void FlowLayout::setGeometry(const QRectF &geom)
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

qreal FlowLayout::spacing() const
{
    return d->spacing;
}

void FlowLayout::setSpacing(qreal s)
{
    d->spacing = s;
}

template <class T>
T qSum(const QList<T>& container) 
{
    T total = 0;
    foreach( const T& item , container ) {
        total += item; 
    }   
    return total;
}

void FlowLayout::relayout()
{
//     const QRectF rect = adjustToMargins(geometry());
    const QRectF rect = geometry();

    const qreal space = spacing();
    const qreal rectWidth = rect.width();
    const qreal rectHeight = rect.height();
    const int count = d->items.count();
    //kDebug() << "Flow layout geometry set to " << geometry();

    // calculate average size of items
    qreal colWidth = 0;
    qreal rowHeight = 0;
    qreal maxItemWidth = 0;
    qreal minItemWidth = 0;
    //qreal maxItemHeight = 0;
    qreal minItemHeight = 0;
    int colCnt = 0;
    int rowCnt = 0;

    foreach(QGraphicsLayoutItem *item , d->items) {
        maxItemWidth = (maxItemWidth < item->maximumSize().width()) ? 
                        item->maximumSize().width() : maxItemWidth;
        minItemWidth = (minItemWidth < item->minimumSize().width()) ? 
                      item->minimumSize().width() : minItemWidth;
        //maxItemHeight = (maxItemHeight < item->maximumSize().height()) ? 
        //              item->maximumSize().height() : maxItemHeight;
        minItemHeight = (minItemHeight < item->minimumSize().height()) ? 
                        item->minimumSize().height() : minItemHeight;
    }

    const int rowMax = ((minItemHeight != 0) && (minItemHeight != rectHeight)) ?
                        (int)(rectHeight / (minItemHeight + space)) : 1;

    if( maxItemWidth == 0 && minItemWidth != 0 ) {
        kDebug() << "******POSSIBLE DIVIDE BY ZERO: maxItemWidth = minItemWidth ********";
        maxItemWidth = minItemWidth + space;
    } else if( maxItemWidth == 0 && minItemWidth == 0 ) {
        kDebug() << "******POSSIBLE DIVIDE BY ZERO: maxItemWidth = rectWidth ********";
        maxItemWidth = rectWidth + space;
    }

    // try to use the maxwidth if there is room
    // need usedSpace so we don't try to use the leftover
    // area of our rect to make a item location.
    maxItemWidth += space;
    qreal usedSpace = floor( (rectWidth / maxItemWidth ) ) * maxItemWidth;
    for(int i = 1; (i <= rowMax) && (colWidth == 0); i++) {
        if( i * (usedSpace / maxItemWidth ) >= count) {
            colWidth = maxItemWidth;
            rowHeight = (rectHeight + space) / i;
            rowCnt = i;
            colCnt = (int)(usedSpace / colWidth);
        }
    }

    //crazy algorithm to make items fit in available space
    if( colWidth == 0) {
        // These gave me the most trouble and should
        // be taken into account if you try and change this:
        // - maxRow = 3 with 9 items and 3 columns.
        // - maxRow = 5 with 8 items and 3 colums.
        // - maxRow = 1 with odd number columns.

        const qreal tmp = (qreal)(count + (count % 2)) / rowMax;
        if( (tmp - floor(tmp)) > 0.5) {
            colCnt = (int)ceil(tmp) + 1;
        } else {
            colCnt = (int)ceil(tmp);
        }
        rowCnt = (int)ceil((qreal)count / colCnt);
        if( (rowCnt == 1) && (colCnt&2) ) {
            colCnt--;
        }
        colWidth = rectWidth / colCnt;
        rowHeight = (rectHeight + space) / rowCnt;
    }


    if( minItemHeight > (rowHeight - space) ) {
        rowHeight = minItemHeight + space;
    }

//     kDebug() << "colWidth: " << colWidth << "rowHeight: " << rowHeight
//                 << "rowCnt: " << rowCnt << "rowMax: " << rowMax << "colCnt: " << colCnt;


    // lay the items out in left-to-right , top-to-bottom order
    int insertColumn = 0;
    qreal rowPos = 0;
    foreach(QGraphicsLayoutItem *item , d->items) {

        if(insertColumn >= colCnt) {
            insertColumn = 0;
            rowPos += rowHeight;
        }

        // position the item
        const QRectF newGeometry(rect.left() + (insertColumn * colWidth),
                                 rect.top() + rowPos,
                                 colWidth - space,
                                 rowHeight - space);

        //kDebug() << "newGeometry: " << newGeometry;
        insertColumn++;

        //FIXME: Port
/*        if ( animator() ){
            animator()->setGeometry( item , newGeometry );
        } else {*/
            item->setGeometry( newGeometry );
//         }
    }
//FIXME: Port
//     startAnimation();
}

qreal FlowLayout::columnWidth() const
{
    return d->columnWidth;
}

void FlowLayout::setColumnWidth( const qreal width )
{
    d->columnWidth = width;
}
