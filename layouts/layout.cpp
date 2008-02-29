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
#include <limits>

#include <QtCore/QList>
#include <QtCore/QTimeLine>
#include <QtDebug>

#include <KDebug>

#include "widgets/widget.h"
#include "layouts/layoutanimator.h"

namespace Plasma
{

class Layout::Private
{
    public:
        Private()
            : spacing(6.0),
              parent(0),
              animator(0),
              relayouting(false)
        {
        }

        ~Private() {}

        qreal spacing;

        LayoutItem *parent;
        LayoutAnimator *animator;

        bool relayouting;
        QPointF pos;
};


Layout::Layout(LayoutItem *parent)
    : LayoutItem(),
      d(new Private)
{
    setMargins(12, 12, 12, 12);
    setParent(parent);
}

void Layout::setParent(LayoutItem *parent)
{
    if (d->parent == parent) {
        return;
    }

    if (d->parent && d->parent->layout() == this) {
        d->parent->unsetLayout();
        releaseManagedItems();
    }

    d->parent = parent;

    if (parent && parent->layout() != this) {
        parent->setLayout(this);
    }
}

Layout::~Layout()
{
    if (d->parent) {
        d->parent->unsetLayout();
        d->parent = 0;
    }

    delete d;
}

bool Layout::isEmpty() const 
{
    return count() == 0;
}

void Layout::updateGeometry()
{
    if (d->relayouting) {
        return;
    }

    d->relayouting = true;
    relayout();
    d->relayouting = false;
}

QRectF Layout::geometry() const
{
    return QRectF(d->pos, size());
}

void Layout::setGeometry(const QRectF &geom)
{
    if (!geom.isValid() || geom == geometry()) {
        return;
    }

    QRectF newGeom = geom;

    if (d->parent && !dynamic_cast<Layout*>(d->parent)) {
        newGeom = d->parent->adjustToMargins(newGeom);
        //kDebug() << "parent rect is" << d->parent->topLeft() << d->parent->size()
        //         << "and we are" << geometry() << "but aiming for"
        //         << newGeom << "from" << geom;
    }

    d->pos = newGeom.topLeft();
    setSize(newGeom.size());
    // TODO: respect minimum and maximum sizes: is it possible?
    //setSize(newGeom.size().expandedTo(minimumSize()).boundedTo(maximumSize()));

    //kDebug() << "geometry is now" << geometry();
    invalidate();
}

void Layout::invalidate()
{
    if (d->relayouting) {
        return;
    }

    d->relayouting = true;
    // find and update the top level layout
    Layout *layout = this;
    Layout *parentLayout = 0;

    do {
        parentLayout = dynamic_cast<Layout*>(layout->parent());
        if (parentLayout) {
            if (parentLayout->d->relayouting) {
                break;
            }
            layout = parentLayout;
        }
    } while (parentLayout);

    layout->relayout();
    d->relayouting = false;
}


LayoutAnimator* Layout::animator() const
{
    return d->animator;
}

void Layout::setAnimator(LayoutAnimator *animator)
{
    d->animator = animator;
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
    return QSizeF(std::numeric_limits<qreal>::infinity(),std::numeric_limits<qreal>::infinity());
}

void Layout::startAnimation() 
{
    if (animator() && animator()->timeLine()) {
        if (animator()->timeLine()->state() == QTimeLine::NotRunning) {
            animator()->timeLine()->setCurrentTime(0);
            animator()->timeLine()->start();
        }
    }
}

}
