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

#include "hboxlayout.h"

#include <QtCore/QList>

#include <KDebug>

namespace Plasma
{

class HBoxLayout::Private
{
	public:
		Private() {}
		~Private() {}

		QRectF geometry;
		QList<LayoutItem *> childList;
};


HBoxLayout::HBoxLayout(LayoutItem *parent)
	: Layout(parent),
	  d(new Private)
{
}

HBoxLayout::~HBoxLayout()
{
    foreach (LayoutItem *l, d->childList) {
        l->resetLayout();
    }

    delete d;
}

Qt::Orientations HBoxLayout::expandingDirections() const
{
	return Qt::Horizontal;
}

QSizeF HBoxLayout::minimumSize() const
{
    return QSizeF();
}

QSizeF HBoxLayout::maximumSize() const
{
	return QSizeF();
}

bool HBoxLayout::hasWidthForHeight() const
{
	return true;
}

qreal HBoxLayout::widthForHeight(qreal w) const
{
	return qreal();
}

QRectF HBoxLayout::geometry() const
{
	return d->geometry;
}

void HBoxLayout::setGeometry(const QRectF& geometry)
{
    if (!geometry.isValid() || geometry.isEmpty()) {
        kDebug() << "Invalid Geometry " << geometry << endl;
        return;
    }

    kDebug() << this << " Geometry process " << geometry << " for " << d->childList.count() << " childrens"<< endl;

    QList<LayoutItem *> children;
    QList<LayoutItem *> expandingChildren;
    QList<QSizeF> sizes;
    QSizeF available = geometry.size() - QSizeF(2 * margin(), 2 * margin());

    foreach (LayoutItem *l, d->childList) {
        kDebug() << "testing layout item " << l << endl;
        if (l->expandingDirections() & Qt::Horizontal) {
            expandingChildren += l;
        } else {

            children += l;
        }
    }

    foreach (LayoutItem *l, children) {
        QSizeF hint = l->sizeHint();
        sizes.insert(indexOf(l), QSizeF(available.width(), hint.height()));
        available -= QSizeF(hint.width() + spacing(), 0.0f);
    }

    qreal expandWidth = (available.width() - ((expandingChildren.count() - 1) * spacing())) / expandingChildren.count();

    foreach (LayoutItem *l, expandingChildren) {

        sizes.insert(indexOf(l), QSizeF(expandWidth, available.height()));
    }

    QPointF start = QPointF(0.0f, 0.0f);
    start += QPointF(margin(), spacing());

    for (int i = 0; i < sizes.size(); i++) {

        LayoutItem *l = itemAt(i);

        kDebug() << "Setting Geometry for child " << l << " to " << QRectF(start, sizes[i]) << endl;

        l->setGeometry(QRectF(start, sizes[i]));
        start += QPointF(sizes[i].width() + spacing(), 0.0);
    }

    d->geometry = geometry;
}

QSizeF HBoxLayout::sizeHint() const
{
	qreal hintHeight = 0.0;
	qreal hintWidth = 0.0;

	foreach(LayoutItem *l, d->childList) {

		QSizeF hint = l->sizeHint();

        hintHeight = qMax(hint.height(), hintHeight);
        hintWidth += hint.width() + spacing();
	}

	return QSizeF(hintWidth, hintHeight);
}

int HBoxLayout::count() const
{
	return d->childList.count();
}

bool HBoxLayout::isEmpty() const
{
	return count() == 0;
}

void HBoxLayout::insertItem(int index, LayoutItem *l)
{
    if (!l) {
        return;
    }

    l->setLayout(this);
    d->childList.insert(index, l);
    setGeometry(geometry());
}

void HBoxLayout::addItem(LayoutItem *l)
{
    if (!l) {
        return;
    }

    l->setLayout(this);
    d->childList.append(l);
    qDebug("Added Child LayoutItem : %p", l);
    setGeometry(geometry());
}

void HBoxLayout::removeItem(LayoutItem *l)
{
	d->childList.removeAll(l);
}

int HBoxLayout::indexOf(LayoutItem *l) const
{
	return d->childList.indexOf(l);
}

LayoutItem *HBoxLayout::itemAt(int i) const
{
	return d->childList[i];
}

LayoutItem *HBoxLayout::takeAt(int i)
{
	return d->childList.takeAt(i);

	setGeometry(geometry());
}

QSizeF HBoxLayout::size() const
{
	return geometry().size();
}

}
