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

#include "layout.h"

#include <math.h>

#include <QtCore/QList>
#include <QtCore/QTimeLine>
#include <QtDebug>

#include "widget.h"
#include "layoutanimator.h"

namespace Plasma
{

class Layout::Private
{
    public:
        Private(LayoutItem* p)
            : margin(12.0),
              spacing(6.0),
              parent(p),
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
    if (parent) {
        parent->setLayout(this);
    }
}

void Layout::setParent(LayoutItem *parent) {
    d->parent = parent;
}

Layout::~Layout()
{
    if (parent()) {
        parent()->setLayout(0);
    }
    delete d;
}

bool Layout::isEmpty() const 
{
    return count() == 0;
}

void Layout::update()
{

    setGeometry(geometry());

}
void Layout::invalidate()
{
    // find and update the top level layout
    Layout *layout = this;
    Layout *parentLayout = 0;

    do {
        parentLayout = dynamic_cast<Layout*>(layout->parent());
        if (parentLayout) {
            layout = parentLayout;
        }
    } while (parentLayout);

    layout->update();
}


LayoutAnimator* Layout::animator() const
{
    return d->animator;
}

void Layout::setAnimator(LayoutAnimator *animator)
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

void Layout::startAnimation() 
{
    if (animator() && animator()->timeLine()) {
        animator()->timeLine()->setCurrentTime(0);
        if (animator()->timeLine()->state() == QTimeLine::NotRunning) {
            animator()->timeLine()->start();
        }
    }
}

}
