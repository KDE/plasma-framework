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

#ifndef __H_BOX_LAYOUT__
#define __H_BOX_LAYOUT__

#include <QtCore/QRectF>
#include <QtCore/QSizeF>

#include <plasma/plasma_export.h>
#include <plasma/widgets/layout.h>

namespace Plasma
{


/**
 * Vertical Box Layout
 *
 * @author Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
 *
 * This class implements a Horizontal Box Layout, it just lays items horizontally, from left to right.
 */
class PLASMA_EXPORT HBoxLayout : public Layout
{
	public:

        /**
         * Constructor.
         */
		HBoxLayout(LayoutItem *parent = 0);

        /**
         * Virtual Destructor.
         */
		virtual ~HBoxLayout();

		Qt::Orientations expandingDirections() const;

		QSizeF minimumSize() const;
		QSizeF maximumSize() const;

		bool hasHeightForWidth() const;
		qreal heightForWidth(qreal w) const;

		QRectF geometry() const;
		void setGeometry(const QRectF& geometry);

		QSizeF sizeHint() const;

		int count() const;

		bool isEmpty() const;

        void insertItem(int index, LayoutItem *l);
		void addItem(LayoutItem *l);

		void removeItem(LayoutItem *l);

		int indexOf(LayoutItem *l) const;
		LayoutItem *itemAt(int i) const;
		LayoutItem *takeAt(int i);

		QSizeF size() const;

	private:
		class Private;
		Private *const d;
};

}

#endif /* __H_BOX_LAYOUT__ */
