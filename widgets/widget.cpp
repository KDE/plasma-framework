/*
 *   Copyright (C) 2007 by Alexander Wiedenbruch <mail@wiedenbruch.de>
 *                      and Matias Valdenegro <mvaldenegro@informatica.utem.cl>
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


#include <QtCore/QList>

#include "layout.h"
#include "widget.h"

namespace Plasma
{

class Widget::Private
{
    public:
        Private() : parent(0) { }
        ~Private() { }

		QRectF geometry;

		Widget *parent;
		Layout *layout;
		QList<Widget *> childList;
};


Widget::Widget(Widget *parent)
  : QGraphicsItem(parent),
    d(new Private)
{
	d->parent = parent;
	d->layout = 0;

	if(parent) {
		parent->addChild(this);
		parent->setGeometry(QRectF(QPointF(0.0, 0.0), parent->size()));
	}
}

Widget::~Widget()
{
    delete d;
}

Qt::Orientations Widget::expandingDirections() const
{
	return 0;
}

QSizeF Widget::maximumSize() const
{
	return QSizeF();
}

QSizeF Widget::minimumSize() const
{
	return QSizeF(0.0, 0.0);
}

bool Widget::hasHeightForWidth() const
{
	return false;
}

qreal Widget::heightForWidth(qreal w) const
{
	Q_UNUSED(w);

	return -1.0;
}

bool Widget::hasWidthForHeight() const
{
	return false;
}

qreal Widget::widthForHeight(qreal h) const
{
	Q_UNUSED(h);

	return -1.0;
}

QRectF Widget::geometry() const
{
	return d->geometry;
}

void Widget::setGeometry(const QRectF& geometry)
{
	prepareGeometryChange();

	d->geometry = geometry;

	updateGeometry();
	update();
}

void Widget::updateGeometry()
{
	if(layout()) {
		layout()->setGeometry(QRectF(QPointF(0.0, 0.0), size()));
	}
}

void Widget::invalidate()
{
	updateGeometry();

	if(parent()) {
		parent()->updateGeometry();
	}
}

QSizeF Widget::sizeHint() const
{
	return QSizeF(0.0, 0.0);
}

QSizeF Widget::size() const
{
	return geometry().size();
}

QRectF Widget::boundingRect() const
{
	return geometry();
}

void Widget::resize(const QSizeF& size)
{
	setGeometry(QRectF(d->geometry.topLeft(), size));
}

void Widget::resize(qreal w, qreal h)
{
	resize(QSizeF(w, h));
}

void Widget::setLayout(Layout *l)
{
	d->layout = l;
}

Layout *Widget::layout() const
{
	return d->layout;
}

Widget *Widget::parent() const
{
	return d->parent;
}

void Widget::addChild(Widget *w)
{
	if(!w) {
		return;
	}

	w->reparent(this);
	d->childList.append(w);

	qDebug("Added Child Widget : %p", (void*)w);

	if(layout()) {

		layout()->addItem(w);

		updateGeometry();
	}
}

void Widget::reparent(Widget *w)
{
	d->parent = w;
	setParentItem(w);
	update();
}

} // Plasma namespace

