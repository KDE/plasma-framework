/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMA_NODE_LAYOUT
#define PLASMA_NODE_LAYOUT

#include <QtCore/QMap>
#include <cmath>

#include <plasma/plasma_export.h>
#include <plasma/plasma.h>
#include <plasma/layouts/layout.h>

namespace Plasma {

/**
 * Node layout has an advanced layouting mechanism. Every item's position
 * is defined by two nodes - one for top-left corner, and the other one for
 * bottom-right corner.
 * 
 * Each node is defined by a pair of relative (xr, yr) and a pair of 
 * absolute (xa, ya) coordinates. The calculated node coordinates depend
 * on the size and position of the NodeLayout object in the following
 * manner:
 *   x = layoutLeft + (xr * layoutWidth)  + xa
 *   y = layoutTop  + (yr * layoutHeight) + ya
 *
 * Alternatively, the item's position can be defined by using one node and
 * one pair of relative coordinates (xr, yr). In that case, the item is sized
 * following the sizeHint(). The relative coordinates (this time they are 
 * relative to the item's geometry, not the layout's) specify what point of
 * the item will be bound to the defined node.
 */

class PLASMA_EXPORT NodeLayout : public Layout {
public:
    class PLASMA_EXPORT NodeCoordinate {
    public:
        /**
         * Position is calculated:
         * x = parentLeft + (xRelative * parentWidth)  + xAbsolute
         * y = parentTop  + (yRelative * parentHeight) + yAbsolute
         */
        NodeCoordinate(qreal xRelative = 0, qreal yRelative = 0, qreal xAbsolute = 0, qreal yAbsolute = 0);

        enum CoordinateType {
            Relative = 0,
            Absolute = 1,
            InnerRelative = 2 };

        static NodeCoordinate simple(qreal x, qreal y, CoordinateType xType = Relative, CoordinateType yType = Relative);

        float xr, xa;
        float yr, ya;
    };

    // reimplemented
    virtual Qt::Orientations expandingDirections() const;

    explicit NodeLayout(LayoutItem * parent = 0);
    virtual ~NodeLayout();

    QSizeF sizeHint() const;

    /**
     * Adds item at top-left corner, with automatic sizing
     * (using sizeHint of the item)
     */
    void addItem (LayoutItem * item);

    /**
     * Adds item with specified top-left and bottom right corners.
     */
    void addItem (LayoutItem * item,
            NodeCoordinate topLeft, NodeCoordinate bottomRight);

    /**
     * Adds item with automatic sizing turned on. xr and yr specify
     * which point of the item is bound to node coordinate. Those
     * are relative coordinates so (0, 0) represent top left corner,
     * (0.5, 0.5) represent the center of the item etc.
     */
    void addItem (LayoutItem * item,
            NodeCoordinate node, qreal xr = 0, qreal yr = 0);

    void removeItem (LayoutItem * item);

    virtual int count() const;
    virtual int indexOf(LayoutItem * item) const;
    virtual LayoutItem * itemAt(int i) const;
    virtual LayoutItem * takeAt(int i);

protected:
    void relayout();
    void releaseManagedItems();

private:
    class Private;
    Private * const d;
};

}

#endif /* PLASMA_NODE_LAYOUT */
