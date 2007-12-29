/*
 *   Copyright 2007 by Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
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

#include "boxlayout.h"

#include <QtCore/QList>
#include <QtCore/QTimeLine>

#include <KDebug>

#include "layoutanimator.h"

namespace Plasma
{

class BoxLayout::Private
{
public:
    BoxLayout *const q;
    Direction direction;
    QList<LayoutItem*> children;

    Private(BoxLayout *parent)
        : q(parent)
        , direction(LeftToRight)
    {
    }

    // returns the component of 'size' in the expanding direction
    // of this layout
    qreal size(const QSizeF& size) const
    {
        switch (direction) {
            case LeftToRight:
            case RightToLeft:
                return size.width();
            case TopToBottom:
            case BottomToTop:
                return size.height();
            default:
                Q_ASSERT(false);
                return 0;
        }
    }

    // returns the direction in which this layout expands
    // or shrinks
    Qt::Orientation expandingDirection() const
    {
        switch ( direction ) {
            case LeftToRight:
            case RightToLeft:
                return Qt::Horizontal;
            case TopToBottom:
            case BottomToTop:
                return Qt::Vertical;
            default:
                Q_ASSERT(false);
                return Qt::Horizontal;
        }
    }

    // returns the position from which layouting should
    // begin depending on the direction of this layout
    qreal startPos(const QRectF& geometry) const
    {
        switch ( direction ) {
            case LeftToRight:
                return geometry.left() + q->margin(LeftMargin);
            case TopToBottom:
                return geometry.top() + q->margin(TopMargin);
            case RightToLeft:
                return geometry.right() - q->margin(RightMargin);
            case BottomToTop:
                return geometry.bottom() - q->margin(BottomMargin);
            default:
                Q_ASSERT(false);
                return 0;
        }
    }

    // lays out an item
    //
    // 'geometry' the geometry of the layout
    // 'item' the item whoose geometry should be altered
    // 'pos' the position of the item (in the expanding direction of the layout)
    // 'size' the size of the item (in the expanding direction of the layout)
    //
    // returns the position for the next item in the layout
    //
    qreal layoutItem(const QRectF& geometry , LayoutItem *item , const qreal pos , qreal size)
    {
        //qDebug() << "layoutItem: " << direction << "item size" << size;

        QRectF newGeometry;
        qreal newPos = 0;

        qreal top = 0;
        qreal height = 0;

        QSizeF minSize = item->minimumSize();
        QSizeF maxSize = item->maximumSize();
        switch ( direction ) {
            case LeftToRight:
            case RightToLeft:
               height = qBound(minSize.height(),geometry.height(),maxSize.height());
               top = geometry.top();
               break;
            case TopToBottom:
            case BottomToTop:
               height = qBound(minSize.width(),geometry.width(),maxSize.width());
               top = geometry.left();
               break;
        }

        switch ( direction ) {
            case LeftToRight:
                newGeometry = QRectF(pos,top,size,height);
                newPos = pos+size+q->spacing();
                break;
            case RightToLeft:
                newGeometry = QRectF(geometry.width()-pos-size,top,
                                     size,height);
                newPos = pos-size-q->spacing();
                break;
            case TopToBottom:
                newGeometry = QRectF(top,pos,height,size);
                newPos = pos+size+q->spacing();
                break;
            case BottomToTop:
                newGeometry = QRectF(top,geometry.height()-pos-size,
                                     height,size);
                newPos = pos-size-q->spacing();
                break;
        }

       // qDebug() << "Item geometry: " << newGeometry;

        if (q->animator()) {
            q->animator()->setGeometry(item, newGeometry);
        } else {
            item->setGeometry(newGeometry);
        }

        return newPos;
    }

    enum SizeType
    {
        MinSize,
        MaxSize,
        HintSize
    };

    // this provides a + function which can be passed as the 'op'
    // argument to calculateSize
    static qreal sum(const qreal a , const qreal b)
    {
        return a+b;
    }

    // calcualtes a size hint or value for this layout
    // 'sizeType' - The item size ( minimum , maximum , hint ) to use
    // 'dir' - The direction component of the item size to use
    // 'op' - A function to apply to the size of each item in the layout
    //        , usually qMax,qMin or sum
    template <class T>
    qreal calculateSize(SizeType sizeType , Qt::Orientation dir , T (*op)(const T,const T)) const
    {
        qreal value = 0;
        for ( int i = 0 ; i < children.count() ; i++ ) {

            QSizeF itemSize;
            switch ( sizeType ) {
                case MinSize:
                    itemSize = children[i]->minimumSize();
                    break;
                case MaxSize:
                    itemSize = children[i]->maximumSize();
                    break;
                case HintSize:
                    itemSize = children[i]->sizeHint();
                    break;
            }

            if ( dir == Qt::Horizontal ) {
                value = op(value,itemSize.width());
            } else {
                value = op(value,itemSize.height());
            }
        }

        return value;
    }

    // calculates a size hint or value for this layout
    // 'calculateSizeType' specifies the value to be calculated
    QSizeF calculateSize(SizeType calculateSizeType) const
    {
        QSizeF result;

        const qreal totalSpacing = q->spacing() * (children.count()-1);

        switch ( direction ) {
            case LeftToRight:
            case RightToLeft:
                result = QSizeF(calculateSize(calculateSizeType,Qt::Horizontal,sum),
                                calculateSize(calculateSizeType,Qt::Vertical,qMax<qreal>));

                result.rwidth() += q->margin(LeftMargin) + q->margin(RightMargin) + totalSpacing;
                result.rheight() += q->margin(TopMargin) + q->margin(BottomMargin);

                break;
            case TopToBottom:
            case BottomToTop:
                result = QSizeF(calculateSize(calculateSizeType,Qt::Horizontal,qMax<qreal>),
                                calculateSize(calculateSizeType,Qt::Vertical,sum));

                result.rheight() += q->margin(TopMargin) + q->margin(BottomMargin) + totalSpacing;
                result.rwidth() += q->margin(LeftMargin) + q->margin(RightMargin);

                break;
        }

        return result;
    }
};


BoxLayout::BoxLayout(Direction direction , LayoutItem *parent)
    : Layout(parent),
      d(new Private(this))
{
    d->direction = direction;
}

void BoxLayout::setDirection(Direction direction)
{
    d->direction = direction;
    updateGeometry();
}
BoxLayout::Direction BoxLayout::direction() const
{
    return d->direction;
}

BoxLayout::~BoxLayout()
{
    foreach (LayoutItem* item, d->children) {
        item->unsetManagingLayout(this);
    }
    delete d;
}

Qt::Orientations BoxLayout::expandingDirections() const
{
    switch ( d->direction ) {
        case LeftToRight:
        case RightToLeft:
            return Qt::Horizontal;
        case TopToBottom:
        case BottomToTop:
            return Qt::Vertical;
        default:
            Q_ASSERT(false);
            return 0;
    }
}

int BoxLayout::count() const
{
    return d->children.count();
}

void BoxLayout::insertItem(int index, LayoutItem *item)
{
    if (!item || d->children.contains(item)) {
        return;
    }

    item->setManagingLayout(this);

    if (index == -1) {
        index = d->children.size();
    }

    d->children.insert(index, item);

    if (animator())  {
        animator()->setCurrentState(item, LayoutAnimator::InsertedState);
    }

    updateGeometry();
}

void BoxLayout::addItem(LayoutItem *item)
{
    if (!item) {
        return;
    }

    insertItem(-1, item);
}

void BoxLayout::removeItem(LayoutItem *item)
{
    if (!item) {
        return;
    }

    item->unsetManagingLayout(this);
    d->children.removeAll(item);

    if (animator()) {
        animator()->setCurrentState(item, LayoutAnimator::RemovedState);
    }

    updateGeometry();
}

int BoxLayout::indexOf(LayoutItem *l) const
{
    return d->children.indexOf(l);
}

LayoutItem *BoxLayout::itemAt(int i) const
{
    if (i >= d->children.count()) {
        return 0;
    }

    return d->children[i];
}

LayoutItem *BoxLayout::takeAt(int i)
{
    if (i >= d->children.count()) {
        return 0;
    }

    return d->children.takeAt(i);
    // FIXME: This is never reached. Should it be called?
    updateGeometry();
}

void BoxLayout::relayout()
{
    QRectF margined = geometry().adjusted(margin(LeftMargin), margin(TopMargin), -margin(RightMargin), -margin(BottomMargin));

    //qDebug() << "geo before " << geo << "and with margins" << margined << "margins" << margin(LeftMargin)
    //         << margin(TopMargin) <<  -margin(RightMargin) << -margin(BottomMargin);
    //qDebug() << "Box layout beginning with geo" << geometry;
    //qDebug() << "This box max size" << maximumSize();

    QVector<qreal> sizes(count());
    QVector<qreal> expansionSpace(count());

    qreal available = d->size(margined.size()) - spacing() * (d->children.count()-1);
    qreal perItemSize = available / count();

    // initial distribution of space to items
    for ( int i = 0 ; i < sizes.count() ; i++ ) {
        const LayoutItem *item = d->children[i];

        const bool isExpanding = item->expandingDirections() & d->expandingDirection();

        if ( isExpanding )
            sizes[i] = perItemSize;
        else
            sizes[i] = d->size(item->sizeHint());

        const qreal minItemSize = d->size(item->minimumSize());
        const qreal maxItemSize = d->size(item->maximumSize());

       // qDebug() << "Layout max item " << i << "size: " << maxItemSize;

        sizes[i] = qMin( sizes[i] , maxItemSize );
        sizes[i] = qMax( sizes[i] , minItemSize );

       // qDebug() << "Available: " << available << "per item:" << perItemSize <<
       //     "Initial size: " << sizes[i];

        if ( isExpanding )
            expansionSpace[i] = maxItemSize-sizes[i];
        else
            expansionSpace[i] = 0;

        available -= sizes[i];
        // adjust the per-item size if the space was over or under used
        if ( sizes[i] != perItemSize && i != sizes.count()-1 ) {
            perItemSize = available / (sizes.count()-i-1);
        }
    }

    // distribute out any remaining space to items which can still expand
    //
    // space is distributed equally amongst remaining items until we run
    // out of space or items to expand
    int expandable = sizes.count();
    const qreal threshold = 1.0;
    while ( available > threshold && expandable > 0 ) {

        qreal extraSpace = available / expandable;
        for ( int i = 0 ; i < sizes.count() ; i++ ) {
            if ( expansionSpace[i] > threshold ) {
                qreal oldSize = sizes[i];

                sizes[i] += qMin(extraSpace,expansionSpace[i]);

                expansionSpace[i] -= sizes[i]-oldSize;
                available -= sizes[i]-oldSize;
            } else {
                expandable--;
            }
        }
    }

    // set items' geometry according to new sizes
    qreal pos = d->startPos(geometry());
    for ( int i = 0 ; i < sizes.count() ; i++ ) {

        //QObject *obj = dynamic_cast<QObject*>(d->children[i]);
        //if ( obj )
        //qDebug() << "Item " << i << obj->metaObject()->className() << "size:" << sizes[i];

       pos = d->layoutItem(margined, d->children[i], pos , sizes[i]);
    }

    startAnimation();
}


QSizeF BoxLayout::maximumSize() const
{
    return Layout::maximumSize();
}
QSizeF BoxLayout::minimumSize() const
{
    return d->calculateSize(Private::MinSize);
}
QSizeF BoxLayout::sizeHint() const
{
    return d->calculateSize(Private::HintSize);
}

HBoxLayout::HBoxLayout(LayoutItem *parent)
    : BoxLayout(LeftToRight,parent)
{
}

VBoxLayout::VBoxLayout(LayoutItem *parent)
    : BoxLayout(TopToBottom,parent)
{
}


} // Plasma namespace

