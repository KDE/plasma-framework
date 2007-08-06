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
};


BoxLayout::BoxLayout(LayoutItem *parent)
    : Layout(parent),
      d(new Private)
{
    if (parent) {
        parent->setLayout(this);
    }
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
    return Qt::Vertical | Qt::Horizontal;
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
    d->geometry = geometry;
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
