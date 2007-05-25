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

#ifndef __V_BOX_LAYOUT__
#define __V_BOX_LAYOUT__

#include <QtCore/QRectF>
#include <QtCore/QSizeF>

#include <kdemacros.h>

#include "layout.h"

namespace Plasma
{

class Widget;

/**
 * Vertical Box Layout
 */
class KDE_EXPORT VBoxLayout : public Layout
{
	public:
		VBoxLayout(LayoutItem *parent = 0);
		virtual ~VBoxLayout();

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

#endif /* __V_BOX_LAYOUT__ */
