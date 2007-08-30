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

#include "boxlayout.h"

#include <QtCore/QList>

#include <KDebug>

namespace Plasma
{

class BoxLayout::Private
{
    public:
        QRectF geometry;
        QList<LayoutItem *> childList;
        Qt::Orientations orientation;
};


BoxLayout::BoxLayout(Qt::Orientations orientation, LayoutItem *parent)
    : Layout(parent),
      d(new Private)
{
    if (parent) {
        parent->setLayout(this);
    }
    d->orientation = orientation;
}

BoxLayout::~BoxLayout()
{
    foreach (LayoutItem* item, children()) {
        item->unsetManagingLayout(this);
    }
    delete d;
}

Qt::Orientations BoxLayout::expandingDirections() const
{
    return d->orientation;
}

QSizeF BoxLayout::minimumSize() const
{
    return QSizeF();
}

QSizeF BoxLayout::maximumSize() const
{
    return QSizeF();
}

QRectF BoxLayout::geometry() const
{
    return d->geometry;
}

void BoxLayout::setGeometry(const QRectF& geometry)
{
    if (!geometry.isValid() || geometry.isEmpty()) {
        kDebug() << "Invalid Geometry " << geometry;
        d->geometry = geometry;
        return;
    }

    kDebug() << this << " Geometry process " << geometry << " for " << children().count() << " children";

    QList<LayoutItem *> fixedChildren;
    QList<LayoutItem *> expandingChildren;
    QList<QSizeF> sizes;
    QSizeF available = geometry.size() - QSizeF(2 * margin(), 2 * margin());

    foreach (LayoutItem *l, children()) {
        if (l->expandingDirections() & d->orientation) {
            expandingChildren.append(l);
        } else {
            fixedChildren.append(l);
        }
    }

    foreach (LayoutItem *l, fixedChildren) {
        QSizeF hint = l->sizeHint();
        if ( d->orientation == Qt::Vertical ) {
            sizes.insert(indexOf(l), QSizeF(available.width(), hint.height()));
            available -= QSizeF(0.0, hint.height() + spacing());
        } else {
            sizes.insert(indexOf(l), QSizeF(hint.width(), available.height()));
            available -= QSizeF(hint.width() + spacing(), 0.0f);
        }
    }

    if (expandingChildren.count() > 0) {
        if ( d->orientation == Qt::Vertical ) {
            qreal expandHeight = (available.height() - ((expandingChildren.count() - 1) * spacing())) / expandingChildren.count();
            foreach (LayoutItem *l, expandingChildren) {
                sizes.insert(indexOf(l),QSizeF(available.width(), expandHeight));
            }
        } else {
            qreal expandWidth = (available.width() - ((expandingChildren.count() - 1) * spacing())) / expandingChildren.count();
            foreach (LayoutItem *l, expandingChildren) {
                sizes.insert(indexOf(l), QSizeF(expandWidth, available.height()));
            }
        }
    }

    QPointF start = geometry.topLeft();
    start += QPointF(margin(), spacing());

    for (int i = 0; i < sizes.size(); i++) {
        LayoutItem *l = itemAt(i);
        kDebug() << "Setting Geometry for child " << l << " to " << QRectF(start, sizes[i]);
        l->setGeometry(QRectF(start, sizes[i]));
        if ( d->orientation == Qt::Vertical ) {
            start += QPointF(0.0, sizes[i].height() + spacing());
        } else {
            start += QPointF(sizes[i].width() + spacing(), 0.0);
        }
    }

    d->geometry = geometry;
}

QSizeF BoxLayout::sizeHint() const
{
    qreal hintHeight = 0.0;
    qreal hintWidth = 0.0;

    foreach(LayoutItem *l, children()) {

        QSizeF hint = l->sizeHint();

        if ( d->orientation == Qt::Vertical ) {
            hintWidth = qMax(hint.width(), hintWidth);
            hintHeight += hint.height() + spacing();
        } else {
            hintHeight = qMax(hint.height(), hintHeight);
            hintWidth += hint.width() + spacing();
        }
    }
    hintWidth += 2 * margin();
    hintHeight += 2 * margin();

    return QSizeF(hintWidth, hintHeight);
}

int BoxLayout::count() const
{
    return d->childList.count();
}

bool BoxLayout::isEmpty() const
{
    return count() == 0;
}

void BoxLayout::insertItem(int index, LayoutItem *l)
{
    if (!l) {
        return;
    }

    //l->setLayout(this);
    d->childList.insert(index, l);
    setGeometry(geometry());
}

void BoxLayout::addItem(LayoutItem *l)
{
    if (!l) {
        return;
    }

    l->setManagingLayout(this);
    d->childList.append(l);
    setGeometry(geometry());
}

void BoxLayout::removeItem(LayoutItem *l)
{
    if (!l) {
        return;
    }

    l->unsetManagingLayout(this);
    d->childList.removeAll(l);
    setGeometry(geometry());
}

int BoxLayout::indexOf(LayoutItem *l) const
{
    return d->childList.indexOf(l);
}

LayoutItem *BoxLayout::itemAt(int i) const
{
    return d->childList[i];
}

LayoutItem *BoxLayout::takeAt(int i)
{
    return d->childList.takeAt(i);

    setGeometry(geometry());
}

QSizeF BoxLayout::size() const
{
    return d->geometry.size();
}

QList<LayoutItem *> BoxLayout::children() const
{
    return d->childList;
}

} // Plasma namespace
