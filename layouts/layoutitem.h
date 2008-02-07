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

#ifndef __LAYOUT_ITEM__
#define __LAYOUT_ITEM__

#include <QtCore/QRectF>
#include <QtCore/QSizeF>

#include <plasma/plasma_export.h>

class QGraphicsItem;

namespace Plasma
{

class Layout;

/**
 * Base class for Plasma layout-managed items
 *
 * @author Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
 *
 * All layout-managed items should implement this class, but regular users just need to use
 * Plasma::Widget and Plasma::Layout.
 */
class PLASMA_EXPORT LayoutItem
{
    public:

        /**
         * Constructor.
         */
        explicit LayoutItem();

        /**
         * Virtual Destructor.
         */
        virtual ~LayoutItem();

        /**
         * Returns a bitmask with the directions that this Item can be expanded.
         */
        virtual Qt::Orientations expandingDirections() const = 0;

        /**
         * Returns the minimum size of this Item and it's contents.
         */
        virtual QSizeF minimumSize() const = 0;

        /**
         * Returns the maximum size of this Item.
         */
        virtual QSizeF maximumSize() const = 0;

        /**
         * Returns true whatever this Item can use height-for-width layout management,
         * false otherwise.
         */
        virtual bool hasHeightForWidth() const;

        /**
         * Returns the corresponding height for a given width.
         * @param w Width
         */
        virtual qreal heightForWidth(qreal w) const;

        /**
         * Returns true whatever this Item can use width-for-height layout management,
         * false otherwise.
         */
        virtual bool hasWidthForHeight() const;

        /**
         * Returns the corresponding width for a given height.
         * @param h Height
         */
        virtual qreal widthForHeight(qreal h) const;

        /**
         * Returns the geometry of this Item.
         */
        virtual QRectF geometry() const = 0;

        /**
         * Sets the geometry of this Item.
         */
        virtual void setGeometry(const QRectF& geometry) = 0;

        /**
         * Updates the layouting of the item without first changing its geometry.
         * Calling this may result in a geometry change, but may not, depending
         * on the managing layout if any.
         */
        virtual void updateGeometry() = 0;

        /**
         * Returns the most appropriate size of this Item to hold whatever contents it has.
         */
        virtual QSizeF sizeHint() const = 0;

        /**
         * Sets the layout that will manage children items. The LayoutItem
         * takes ownership of the layout from that point forward, unless
         * unsetLayout() is called.
         *
         * @param layout The Layout that this LayoutItem will be managed by.
         */
        void setLayout(Layout* layout);

        /**
         * Resets the layout that will manage children items to no layout.
         * Note that the caller of this method must alert any items managed
         * by the layout of this change if necessary. Primarily, this should
         * only be used from the dtors of LayoutItem subclasses.
         */
        void unsetLayout();

        /**
         * @return the layout this item is currently associated with.
         */
        Layout* layout() const;

        /**
         * Sets the layout that manages this item's geometry
         *
         * @param layout the layout that manage this item's geometry
         **/
        void setManagingLayout(Layout* layout);

        /**
         * Resets the layout that manges this item's geometry if it is the
         * currently associated layout
         *
         * @param layout to unset
         **/
        void unsetManagingLayout(Layout* layout);

        /**
         * @return the layout that manages this item's geometry, or 0 if none
         **/
        Layout* managingLayout() const;

        /**
         * Returns the graphics item associated with this layout item or 0
         * if there is no associated graphics item.
         *
         * The default implementation returns 0.
         */
        virtual QGraphicsItem* graphicsItem();

    protected:
        /**
         * Reimplement to respond to a change in managing layout
         */
        virtual void managingLayoutChanged();

    private:
        class Private;
        Private *const d;
};

}

#endif /* __LAYOUT_ITEM__ */
