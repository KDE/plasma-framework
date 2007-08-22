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

#include "layout.h"

#include <math.h>

#include <QtCore/QList>

namespace Plasma
{

class Layout::Private
{
    public:
        Private(LayoutItem* parent)
            : margin(12.0),
              spacing(6.0),
              parent(parent),
              animator(0)
        {
        }

        ~Private() {}

        qreal margin;
        qreal spacing;

        LayoutItem *parent;
        LayoutAnimator *animator;
};


Layout::Layout(LayoutItem *parent)
    : LayoutItem(),
      d(new Private(parent))
{
}

Layout::~Layout()
{
    if (parent()) {
        parent()->setLayout(0);
    }
    delete d;
}

void Layout::update()
{
    // this will force an update
    setGeometry( parent()->geometry() );
}

LayoutAnimator* Layout::animator() const
{
    return d->animator;
}

void Layout::setAnimator(LayoutAnimator* animator)
{
    d->animator = animator;
}

qreal Layout::margin() const
{
	return d->margin;
}

void Layout::setMargin(qreal m)
{
	d->margin = m;
}

qreal Layout::spacing() const
{
	return d->spacing;
}

void Layout::setSpacing(qreal s)
{
	d->spacing = s;
}

LayoutItem *Layout::parent() const
{
	return d->parent;
}

QSizeF Layout::minimumSize() const
{
    return QSizeF(0,0);
}
QSizeF Layout::maximumSize() const
{
    return QSizeF(INFINITY,INFINITY);
}

}
