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

#include "vboxlayout.h"

#include <QtCore/QList>

#include <KDebug>

namespace Plasma
{

class VBoxLayout::Private
{
	public:
		Private() {}
		~Private() {}

		QRectF geometry;
		QList<LayoutItem *> childList;
};


VBoxLayout::VBoxLayout(LayoutItem *parent)
	: Layout(parent),
	  d(new Private)
{
}

VBoxLayout::~VBoxLayout()
{
    foreach (LayoutItem *l, d->childList) {
        l->resetLayout();
    }

    delete d;
}

Qt::Orientations VBoxLayout::expandingDirections() const
{
	return Qt::Vertical;
}

QSizeF VBoxLayout::minimumSize() const
{
        return QSizeF();
}

QSizeF VBoxLayout::maximumSize() const
{
	return QSizeF();
}

bool VBoxLayout::hasHeightForWidth() const
{
	return true;
}

qreal VBoxLayout::heightForWidth(qreal w) const
{
	return qreal();
}

QRectF VBoxLayout::geometry() const
{
	return d->geometry;
}

void VBoxLayout::setGeometry(const QRectF& geometry)
{
    if (!geometry.isValid() || geometry.isEmpty()) {
        kDebug() << "Invalid Geometry!" << endl;
        return;
    }

    qDebug("Geometry %p : %f, %f by %f, %f", this, geometry.x(), geometry.y(), geometry.width(), geometry.height());

    QList<LayoutItem *> children;
    QList<LayoutItem *> expandingChildren;
    QList<QSizeF> sizes;
    QSizeF available = geometry.size() - QSizeF(2 * margin(), 2 * margin());

    foreach (LayoutItem *l, d->childList) {
        kDebug() << "testing layout item " << l << endl;
        if (l->expandingDirections() & Qt::Vertical) {
            expandingChildren += l;
        } else {

            children += l;
        }
    }

    foreach (LayoutItem *l, children) {
        QSizeF hint = l->sizeHint();
        sizes.insert(indexOf(l), QSizeF(available.width(), hint.height()));
        available -= QSizeF(0.0, hint.height() + spacing());
    }

    qreal expandHeight = (available.height() - ((expandingChildren.count() - 1) * spacing())) / expandingChildren.count();

    foreach (LayoutItem *l, expandingChildren) {

        sizes.insert(indexOf(l),QSizeF(available.width(), expandHeight));
    }

    QPointF start = geometry.topLeft();
    start += QPointF(margin(), spacing());

    for (int i = 0; i < sizes.size(); i++) {

        LayoutItem *l = itemAt(i);

        l->setGeometry(QRectF(start, sizes[i]));
        start += QPointF(0.0, sizes[i].height() + spacing());
    }

    d->geometry = geometry;
}

QSizeF VBoxLayout::sizeHint() const
{
	qreal hintHeight = 0.0;
	qreal hintWidth = 0.0;

	foreach(LayoutItem *l, d->childList) {

		QSizeF hint = l->sizeHint();

		hintWidth = qMax(hint.width(), hintWidth);
		hintHeight += hint.height() + spacing();
	}

	return QSizeF(hintWidth, hintHeight);
}

int VBoxLayout::count() const
{
	return d->childList.count();
}

bool VBoxLayout::isEmpty() const
{
	return count() == 0;
}

void VBoxLayout::insertItem(int index, LayoutItem *l)
{
    if (!l) {
        return;
    }

    l->setLayout(this);
    d->childList.insert(index, l);
    setGeometry(geometry());
}

void VBoxLayout::addItem(LayoutItem *l)
{
    if (!l) {
        return;
    }

    l->setLayout(this);
    d->childList.append(l);
    qDebug("Added Child LayoutItem : %p", l);
    setGeometry(geometry());
}

void VBoxLayout::removeItem(LayoutItem *l)
{
	d->childList.removeAll(l);
}

int VBoxLayout::indexOf(LayoutItem *l) const
{
	return d->childList.indexOf(l);
}

LayoutItem *VBoxLayout::itemAt(int i) const
{
	return d->childList[i];
}

LayoutItem *VBoxLayout::takeAt(int i)
{
	return d->childList.takeAt(i);

	setGeometry(geometry());
}

QSizeF VBoxLayout::size() const
{
	return geometry().size();
}

}
