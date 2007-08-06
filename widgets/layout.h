/*
 *   Copyright 2007 by Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
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

#ifndef __LAYOUT__
#define __LAYOUT__

#include <QtCore/QRectF>
#include <QtCore/QSizeF>

#include <plasma/widgets/layoutitem.h>
#include <plasma/plasma_export.h>

namespace Plasma
{

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
		qreal margin() const;

        /**
         * Sets the margin of this Layout.
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
         * Returns the number of elements of this Layout.
         */
		virtual int count() const = 0;

        /**
         * Returns true if this Layout contains no elements, false otherwise.
         */
		virtual bool isEmpty() const = 0;

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

	private:
		class Private;
		Private *const d;
};

}

#endif /* __LAYOUT__ */
