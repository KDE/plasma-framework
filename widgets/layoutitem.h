/*
 *   Copyright (C) 2007 by Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include <plasma_export.h>

namespace Plasma
{

class Layout;

/**
 * Base class for Plasma layout-managed items
 */
class PLASMA_EXPORT LayoutItem
{
    public:
        LayoutItem();
        virtual ~LayoutItem();

        virtual Qt::Orientations expandingDirections() const = 0;

        virtual QSizeF minimumSize() const = 0;
        virtual QSizeF maximumSize() const = 0;

        virtual bool hasHeightForWidth() const;
        virtual qreal heightForWidth(qreal w) const;

        virtual bool hasWidthForHeight() const;
        virtual qreal widthForHeight(qreal h) const;

        virtual QRectF geometry() const = 0;
        virtual void setGeometry(const QRectF& geometry) = 0;

        virtual QSizeF sizeHint() const = 0;

        /**
         * Resets the layout to 0 and doesn't notify the previous layout.
         * Should only be used by the current layout when relinquishing the item,
         * e.g. during layout destruction.
         */
        void resetLayout();

        /**
         * Sets the layout so that the LayoutItem may inform the layout of its
         * deletion. Should only be used by the layout it is added to.
         *
         * If the layout item is currently associated with another layout, it will
         * first remove itself from that layout.
         *
         * @param layout The Layout that this LayoutItem will be managed by.
         */
        void setLayout(Layout* layout);

        /**
         * Returns the layout this item is currently associated with
         */
        Layout* layout();

    private:
        class Private;
        Private *const d;
};

}

#endif /* __LAYOUT_ITEM__ */
