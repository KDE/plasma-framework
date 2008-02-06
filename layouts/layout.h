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

#ifndef __LAYOUT__
#define __LAYOUT__

#include <QtCore/QRectF>
#include <QtCore/QSizeF>

#include <plasma/layouts/layoutitem.h>
#include <plasma/plasma_export.h>
#include <plasma/plasma.h>

namespace Plasma
{

class LayoutAnimator;

/**
 * Base class for Plasma Layout managers
 *
 * @author Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
 *
 * All layout managers must implement this class. Normal users should use the specific layouts,
 * like Plasma::VBoxLayout, Plasma::HBoxLayout and Plasma::GridLayout.
 */

class PLASMA_EXPORT Layout : public LayoutItem
{
    public:
        /**
         * Constructor.
         */
        explicit Layout(LayoutItem *parent);

        /**
         * Virtual Destructor.
         */
        virtual ~Layout();

        /**
         * Returns the margin of this Layout.
         */
        qreal margin(Plasma::MarginEdge edge) const;

        /**
         * Sets the margin of this Layout.
         */
        void setMargin(Plasma::MarginEdge edge, qreal m);

        /**
         * Sets all the margins of this Layout.
         */
        void setMargin(qreal m);

        /**
         * Returns the spacing between Layout elements of this Layout.
         */
        qreal spacing() const;

        /**
         * Sets the spacing of this Layout.
         */
        void setSpacing(qreal s);

        /**
         * Returns the parent of this Layout.
         */
        LayoutItem *parent() const;

        /**
         * Sets the parent of this layout.
         */
        void setParent(LayoutItem *parent);

        /**
         * Returns the number of elements of this Layout.
         */
        virtual int count() const = 0;

        /**
         * Returns true if this Layout contains no elements, false otherwise.
         */
        bool isEmpty() const;

        /**
         * Adds a Item to this Layout.
         * @param l Pointer to the Item to be added.
         */
        virtual void addItem(LayoutItem *l) = 0;

         /**
          * Removes a Item from this Layout.
          * @param l Pointer to the Item to be removed.
          */
        virtual void removeItem(LayoutItem *l) = 0;

        /**
         * Returns the index of a Item in this Layout.
         * @param l Pointer to an Item to be queryed.
         */
        virtual int indexOf(LayoutItem *l) const = 0;

        /**
         * Returns a Pointer to an Item in this Layout.
         * @param i Index of the desired Item.
         */
        virtual LayoutItem *itemAt(int i) const = 0;

        /**
         * Takes the Pointer of an Item in this Layout.
         * @param i Index of the desired Item.
         */
        virtual LayoutItem *takeAt(int i) = 0;

        /**
         * Returns the object controlling animation of changes
         * in this layout or 0 if no animator has been set.
         */
        virtual LayoutAnimator* animator() const;

        /** 
         * Sets the object controlling animation of changes in this
         * layout.
         */
        virtual void setAnimator( LayoutAnimator* animator );

        /**
         * Returns the current geometry for this layout
         */
        virtual QRectF geometry() const;

        /**
         * Changes the geometry of this layout
         */
        void setGeometry(const QRectF &geometry);

        /** Triggers an update of the layout. */ 
        void updateGeometry();

        /** 
         * Returns the minimum size of this layout.
         * The default implementation allows unlimited resizing.
         */
        virtual QSizeF minimumSize() const;
        /**
         * Returns the maximum size of this layout.  The default
         * implementation allows unlimited resizing.
         */
        virtual QSizeF maximumSize() const;      

        /** TODO Document me */
        void invalidate();

    protected:
        /**
         * Triggers a layout, usually after a change in geometry
         */
        virtual void relayout() = 0;

        /**
         * Starts a layout animation.  Subclasses may call this 
         * at the end of their relayout() implementation to
         * start the timeline associated with the layout's animator()
         * if there is one.  If an animation is already in progress then
         * the timeline is reset to 0ms and the animation continues.
         */
        void startAnimation();

    private:
        class Private;
        Private *const d;
};

}

#endif /* __LAYOUT__ */
