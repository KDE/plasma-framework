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

#include "widgets/widget.h"
#include "layouts/layoutanimator.h"

namespace Plasma
{

class Layout::Private
{
    public:
        Private(LayoutItem* p)
            : leftMargin(12.0),
              rightMargin(12.0),
              topMargin(12.0),
              bottomMargin(12.0),
              spacing(6.0),
              parent(p),
              animator(0),
              relayouting(false)
        {
        }

        ~Private() {}

        qreal leftMargin;
        qreal rightMargin;
        qreal topMargin;
        qreal bottomMargin;
        qreal spacing;

        LayoutItem *parent;
        LayoutAnimator *animator;

        bool relayouting;
        QRectF geometry;
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
    return d->geometry;
}

void Layout::setGeometry(const QRectF &geometry)
{
    if (!geometry.isValid() || geometry == d->geometry) {
        return;
    }

    d->geometry = geometry;
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

qreal Layout::margin(MarginEdge edge) const
{
    switch (edge) {
        case LeftMargin:
            return d->leftMargin;
            break;
        case RightMargin:
            return d->rightMargin;
            break;
        case TopMargin:
            return d->topMargin;
            break;
        case BottomMargin:
            return d->bottomMargin;
            break;
    }

    return 0;
}

void Layout::setMargin(MarginEdge edge, qreal m)
{
    switch (edge) {
        case LeftMargin:
            d->leftMargin = m;
            break;
        case RightMargin:
            d->rightMargin = m;
            break;
        case TopMargin:
            d->topMargin = m;
            break;
        case BottomMargin:
            d->bottomMargin = m;
            break;
    }
}

void Layout::setMargin(qreal m)
{
    d->leftMargin = m;
    d->rightMargin = m;
    d->topMargin = m;
    d->bottomMargin = m;
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
