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

#ifndef __LAYOUT__
#define __LAYOUT__

#include <QtCore/QRectF>
#include <QtCore/QSizeF>

#include <kdemacros.h>

#include "layoutitem.h"

namespace Plasma
{

/**
 * Base class for Plasma Layout managers
 */
class Widget;

class KDE_EXPORT Layout : public LayoutItem
{
	public:
		Layout(LayoutItem *parent);
		virtual ~Layout();

		qreal margin() const;
		void setMargin(qreal m);

		qreal spacing() const;
		void setSpacing(qreal s);

		LayoutItem *parent() const;

		virtual int count() const = 0;
		virtual bool isEmpty() const = 0;

		virtual void addItem(LayoutItem *l) = 0;

		virtual void removeItem(LayoutItem *l) = 0;

		virtual int indexOf(LayoutItem *l) const = 0;
		virtual LayoutItem *itemAt(int i) const = 0;
		virtual LayoutItem *takeAt(int i) = 0;

	private:
		class Private;
		Private *const d;
};

}

#endif /* __LAYOUT__ */
