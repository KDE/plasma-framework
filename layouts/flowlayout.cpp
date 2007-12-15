/*
*   Copyright 2007 by Robert Knight <robertknight@gmail.com>
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

#include <QtDebug>

#include "layoutanimator.h"

using namespace Plasma;

class FlowLayout::Private
{
public:
    Private() : columnWidth( -1 ) {}
    QList<LayoutItem*> items; 
    qreal columnWidth;
};

FlowLayout::FlowLayout(LayoutItem* parent)
    : Layout(parent)
    , d(new Private)
{
}
FlowLayout::~FlowLayout()
{
    delete d;
}

int FlowLayout::count() const
{
    return d->items.count();
}

void FlowLayout::addItem(LayoutItem* item)
{
    if (!item || d->items.contains(item)) {
        return;
    }

    item->setManagingLayout(this);
    d->items << item;

    if (animator()) {
        animator()->setCurrentState(item,LayoutAnimator::InsertedState);
    }

    updateGeometry();
    startAnimation();
}
void FlowLayout::removeItem(LayoutItem* item)
{
    if (!item) {
        return;
    }

    item->unsetManagingLayout(this);
    d->items.removeAll(item);

    if (animator()) {
        animator()->setCurrentState(item,LayoutAnimator::RemovedState);
    }

    updateGeometry();
    startAnimation();
}
int FlowLayout::indexOf(LayoutItem* item) const
{
    if (!item) {
        return -1;
    }

    return d->items.indexOf(item);
}
LayoutItem* FlowLayout::itemAt(int i) const
{
    if (i >= d->items.count()) {
        return 0;
    }

    return d->items[i];
}

QSizeF FlowLayout::sizeHint() const
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

LayoutItem* FlowLayout::takeAt(int i)
{
    if (i >= d->items.count()) {
        return 0;
    }

    return d->items.takeAt(i);
    // FIXME: Should updateGeometry() and startAnimation() be called?
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
    QRectF rect = geometry().adjusted(margin(LeftMargin), margin(TopMargin), -margin(RightMargin), -margin(BottomMargin));

    qDebug() << "Flow layout geometry set to " << geometry();

    // calculate average size of items
    qreal totalWidth = 0;
    qreal totalHeight = 0;

    foreach(LayoutItem *item , d->items) {
        totalWidth += item->sizeHint().width();
        totalHeight += item->sizeHint().height();
    }

    // use the average item width as the column width.
    // Also include the spacing either side of each item as part of the 
    // average width, this provides the spacing between the items and
    // also allows some tolerance for small differences in item widths 
    qreal averageWidth;
    if (d->columnWidth == -1) {
        averageWidth = totalWidth / d->items.count() + 2*spacing();
    } else {
        averageWidth = d->columnWidth;
    }

    const int columnCount = (int)(rect.width() / averageWidth);

    int insertColumn = 0;
    qreal rowPos = 0;
    qreal rowHeight = 0;

    // lay the items out in left-to-right , top-to-bottom order
    foreach(LayoutItem *item , d->items) {
        const QSizeF& itemSize = item->sizeHint();

        int columnSpan = (int)ceil(itemSize.width() / averageWidth);

        if ( insertColumn + columnSpan > columnCount ) {
            // start a new row
            insertColumn = 0;
            rowPos += rowHeight + spacing();
        }

       // qDebug() << "Inserting item at column" << insertColumn 
       //          << "spanning" << columnSpan << "columns"
       //          << "with offset" << offset;


        // try to expand the item to fill its allocated number of columns
        qreal itemWidth = itemSize.width(); 
        const qreal idealWidth = columnSpan * averageWidth - spacing();
        if ( itemWidth < idealWidth && 
             idealWidth < item->maximumSize().width() ) {
             itemWidth = idealWidth; 
        }
       
        // calculate offset to horizontally center item 
        qreal offset = (columnSpan * averageWidth) - itemWidth;
        if ( insertColumn == 0 )
            offset -= spacing();  
        offset /= 2;

        // try to restrict the item width to the available geometry's
        // width
        if ( itemWidth > rect.width() ) {
            itemWidth = qMax(rect.width(),item->minimumSize().width());
            offset = 0;
        }        

        // position the item
        const QRectF newGeometry(rect.left() + insertColumn * averageWidth + offset,
                                 rect.top() + rowPos,
                                 itemWidth,
                                 itemSize.height());

        rowHeight = qMax(rowHeight,itemSize.height());
        insertColumn += columnSpan;

        if ( animator() )
            animator()->setGeometry( item , newGeometry );
        else
            item->setGeometry( newGeometry );
    }
}

Qt::Orientations FlowLayout::expandingDirections() const
{
    return Qt::Vertical | Qt::Horizontal;
}

qreal FlowLayout::columnWidth() const
{
    return d->columnWidth;
}

void FlowLayout::setColumnWidth( const qreal width )
{
    d->columnWidth = width;
}
