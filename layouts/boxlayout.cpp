/*
 *   Copyright 2007 by Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
 *   Copyright 2007 by Robert Knight <robertknight@gmail.com>
 *   Copyright 2008 by Olivier Goffart <ogoffart@kde.org>
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

#include <math.h>

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
    bool expandingBoth;
    bool multiRow;
    int rowCount;
    int colCount() const {
        return ((children.count() - 1) / rowCount) + 1;
    }

    Private(BoxLayout *parent)
        : q(parent)
        , direction(LeftToRight), expandingBoth(false), multiRow(false) , rowCount(1)
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
    
    // returns the component of 'size' in the other direction
    qreal size_o(const QSizeF& size) const
    {
        switch (direction) {
            case LeftToRight:
            case RightToLeft:
                return size.height();
            case TopToBottom:
            case BottomToTop:
                return size.width();
            default:
                Q_ASSERT(false);
                return 0;
        }
    }


    // returns the directions in which this layout expands
    // or shrinks
    Qt::Orientations expandingDirections() const
    {
        if (expandingBoth) {
            return Qt::Horizontal|Qt::Vertical;
        }

        switch (direction) {
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
        switch (direction) {
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
    qreal layoutItem(const QRectF& geometry , LayoutItem *item , const qreal pos ,  qreal size, int row)
    {
        //kDebug() << "layoutItem: " << direction << "item size" << size;

        QRectF newGeometry;
        qreal newPos = 0;

        qreal top = 0;
        qreal height = 0;

        const QSizeF minSize = item->minimumSize();
        const QSizeF maxSize = item->maximumSize();
        switch (direction) {
            case LeftToRight:
            case RightToLeft:
               height = (geometry.height() - q->spacing() * (rowCount - 1)) / rowCount;
               top = geometry.top() + row * (height + q->spacing());
               height = qBound(minSize.height(), height, maxSize.height());
               break;
            case TopToBottom:
            case BottomToTop:
               height = (geometry.width() - q->spacing() * (rowCount - 1)) / rowCount;
               top = geometry.left() + row * (height + q->spacing());
               height = qBound(minSize.width(), height, maxSize.width());
               break;
        }

        switch (direction) {
            case LeftToRight:
                newGeometry = QRectF(pos, top, size, height);
                newPos = pos + size + q->spacing();
                break;
            case RightToLeft:
                newGeometry = QRectF(geometry.width() - pos - size, top, size, height);
                newPos = pos - size - q->spacing();
                break;
            case TopToBottom:
                newGeometry = QRectF(top, pos, height, size);
                newPos = pos + size + q->spacing();
                break;
            case BottomToTop:
                newGeometry = QRectF(top, geometry.height() - pos - size, height, size);
                newPos = pos - size - q->spacing();
                break;
        }

       // kDebug() << "Item geometry: " << newGeometry;

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
    static qreal sum(const qreal a, const qreal b)
    {
        return a + b;
    }

    // calcualtes a size hint or value for this layout
    // 'sizeType' - The item size ( minimum , maximum , hint ) to use
    // 'dir' - The direction component of the item size to use
    // 'op' - A function to apply to the size of each item in the layout
    //        , usually qMax,qMin or sum
    template <class T>
    qreal calculateSize(SizeType sizeType, Qt::Orientation dir, T (*op)(const T, const T)) const
    {
        qreal value = 0;
        for (int i = 0; i < children.count(); i++) {

            QSizeF itemSize;
            switch (sizeType) {
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

            if (dir == Qt::Horizontal) {
                value = op(value, itemSize.width());
            } else {
                value = op(value, itemSize.height());
            }
        }

        return value;
    }

    // calculates a size hint or value for this layout
    // 'calculateSizeType' specifies the value to be calculated
    QSizeF calculateSize(SizeType calculateSizeType) const
    {
        QSizeF result;

        const qreal totalSpacingC = q->spacing() * (colCount() - 1);
        const qreal totalSpacingR = q->spacing() * (rowCount - 1);

        switch (direction) {
            case LeftToRight:
            case RightToLeft:
                result = QSizeF(calculateSize(calculateSizeType, Qt::Horizontal, sum) / rowCount,
                                calculateSize(calculateSizeType, Qt::Vertical, qMax<qreal>) * rowCount);

                result.rwidth() += q->margin(LeftMargin) + q->margin(RightMargin) + totalSpacingC;
                result.rheight() += q->margin(TopMargin) + q->margin(BottomMargin) + totalSpacingR;

                break;
            case TopToBottom:
            case BottomToTop:
                result = QSizeF(calculateSize(calculateSizeType, Qt::Horizontal, qMax<qreal>) / rowCount,
                                calculateSize(calculateSizeType, Qt::Vertical, sum) * rowCount);

                result.rheight() += q->margin(TopMargin) + q->margin(BottomMargin) + totalSpacingC;
                result.rwidth() += q->margin(LeftMargin) + q->margin(RightMargin) + totalSpacingR;

                break;
        }

        return result;
    }
};


BoxLayout::BoxLayout(Direction direction, LayoutItem *parent)
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
    releaseManagedItems();
    delete d;
}

Qt::Orientations BoxLayout::expandingDirections() const
{
    return d->expandingDirections();
}

int BoxLayout::count() const
{
    return d->children.count();
}

void BoxLayout::setAnimator(LayoutAnimator *animator)
{
    Layout::setAnimator(animator);

    if (animator) {
        foreach (LayoutItem *item, d->children) {
            animator->setGeometry(item, item->geometry());
            animator->setCurrentState(item, LayoutAnimator::StandardState);
        }
    }
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
    const QRectF margined = geometry().adjusted(margin(LeftMargin), margin(TopMargin), -margin(RightMargin), -margin(BottomMargin));

    //kDebug() << "geo before " << geo << "and with margins" << margined << "margins" << margin(LeftMargin)
    //         << margin(TopMargin) <<  -margin(RightMargin) << -margin(BottomMargin);
    //kDebug() << "Box layout beginning with geo" << geometry;
    //kDebug() << "This box max size" << maximumSize();
    d->rowCount = 1;
    if (d->multiRow) {
        qreal minRowSize = 1;
        qreal minWidth = 0;
        for(int i = 0; i < d->children.count(); i++) {
            minRowSize = qMax(minRowSize, d->size_o(d->children[i]->minimumSize()));
            minWidth += d->size(d->children[i]->minimumSize());
        }

        const qreal ratio = 2.25; //maybe this should not be hardcoded
        //we want the height of items be larger than the minimum size
        int maxRow = (d->size_o(margined.size()) + spacing()) / 
                (minRowSize + spacing());
        //we want enough rows to be able to fit each items width.
        int minRow = (minWidth + d->children.count() * spacing()) / 
                (d->size(margined.size()) + spacing() + 0.1);
        //FIXME:  this formula doesn't take the cellspacing in account
        //        it should also try to "fill" before adding a row
        d->rowCount = 1 + sqrt(ratio * count() * d->size_o(margined.size()) /
                (d->size(margined.size()) + 1));

        d->rowCount = qMax(minRow, d->rowCount);
        d->rowCount = qMin(maxRow, d->rowCount);
        d->rowCount = qMax(1, d->rowCount);
    }

    const int colCount = d->colCount();

    QVector<qreal> sizes(colCount, 0);
    QVector<qreal> expansionSpace(colCount, 0);

    qreal available = d->size(margined.size()) - spacing() * colCount;
    qreal perItemSize = available / colCount;

    // initial distribution of space to items
    for (int i = 0; i < colCount; i++) {
        qreal minItemSize = 0;
        qreal maxItemSize = 65536;
        bool isExpanding = true;
        qreal hint = 0;

        for (int f = i * d->rowCount; f < (i + 1) * d->rowCount; f++) {
            if (f >= count()) {
                break;
            }
            const LayoutItem *item = d->children[f];
            const bool itemExp = (item->expandingDirections() & d->expandingDirections());
            isExpanding = isExpanding && itemExp;
            minItemSize = qMax(minItemSize, d->size(item->minimumSize()));
            maxItemSize = qMin(maxItemSize, d->size(item->maximumSize()));
            if (!itemExp) {
                hint = qMax(hint, d->size(item->sizeHint()));
            }
        }

        if (isExpanding) {
            sizes[i] = perItemSize;
        } else {
            sizes[i] = hint;
        }

       // kDebug() << "Layout max item " << i << "size: " << maxItemSize;

        sizes[i] = qMin(sizes[i], maxItemSize);
        sizes[i] = qMax(sizes[i], minItemSize);

       // kDebug() << "Available: " << available << "per item:" << perItemSize <<
       //     "Initial size: " << sizes[i];

        if (isExpanding) {
            expansionSpace[i] = maxItemSize - sizes[i];
        } else {
            expansionSpace[i] = 0;
        }

        available -= sizes[i];
        // adjust the per-item size if the space was over or under used
        if (sizes[i] != perItemSize && i != sizes.count() - 1) {
            perItemSize = available / (sizes.count() - i - 1);
        }
    }

    // distribute out any remaining space to items which can still expand
    //
    // space is distributed equally amongst remaining items until we run
    // out of space or items to expand
    int expandable = sizes.count();
    const qreal threshold = 1.0;
    while (available > threshold && expandable > 0) {

        const qreal extraSpace = available / expandable;
        for (int i = 0; i < colCount; i++) {
            if (expansionSpace[i] > threshold) {
                qreal oldSize = sizes[i];

                sizes[i] += qMin(extraSpace, expansionSpace[i]);

                expansionSpace[i] -= sizes[i] - oldSize;
                available -= sizes[i] - oldSize;
            } else {
                expandable--;
            }
        }
    }

    // set items' geometry according to new sizes
    qreal pos = d->startPos(geometry());
    for (int col = 0; col < colCount; col++) {
        int newPos = pos;
        for (int row = 0; row < d->rowCount; row++) {
            int i = col * d->rowCount + row;
            if (i >= count()) {
                break;
            }

            //QObject *obj = dynamic_cast<QObject*>(d->children[i]);
            //if ( obj )
            //kDebug() << "Item " << i << obj->metaObject()->className() << "size:" << sizes[i];

           int p = d->layoutItem(margined, d->children[i], pos, sizes[col], row);
           newPos = (row != 0 && p < pos) ? qMin(p, newPos) : qMax(p, newPos);
        }
        pos = newPos;
    }

    startAnimation();
}

void BoxLayout::releaseManagedItems()
{
    foreach (LayoutItem* item, d->children) {
        item->unsetManagingLayout(this);
    }
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

void BoxLayout::setMultiRow(bool b)
{
    d->multiRow = b;
}

void BoxLayout::setExpandingBoth(bool both)
{
    d->expandingBoth = both;
}

HBoxLayout::HBoxLayout(LayoutItem *parent)
    : BoxLayout(LeftToRight, parent)
{
}

VBoxLayout::VBoxLayout(LayoutItem *parent)
    : BoxLayout(TopToBottom, parent)
{
}

} // Plasma namespace


