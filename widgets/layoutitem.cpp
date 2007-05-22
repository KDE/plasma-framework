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

#include "layoutitem.h"

namespace Plasma
{

class LayoutItem::Private
{
	public:
		Private() {}
		~Private() {}
};


LayoutItem::LayoutItem()
	: d(new Private)
{
}

LayoutItem::~LayoutItem()
{
	delete d;
}

bool LayoutItem::hasHeightForWidth() const
{
	return false;
}

qreal LayoutItem::heightForWidth(qreal w) const
{
	return 0.0;
}

bool LayoutItem::hasWidthForHeight() const
{
	return false;
}

qreal LayoutItem::widthForHeight(qreal h) const
{
	return 0.0;
}

};
