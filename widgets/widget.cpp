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


#include "widget.h"

#include <cmath>
#include <limits>

#include <QtCore/QList>
#include <QPainter>

#include <KDebug>

#include "layout.h"
#include "plasma/plasma.h"

namespace Plasma
{

class Widget::Private
{
    public:
        Private()
            : parent(0)
        { }
        ~Private() { }

        QSizeF size;
        QSizeF minimumSize;
        QSizeF maximumSize;

        Widget *parent;
        QList<Widget *> childList;

        bool shouldPaint(QPainter *painter, const QTransform &transform);
};

bool Widget::Private::shouldPaint(QPainter *painter, const QTransform &transform)
{
    qreal zoomLevel = painter->transform().m11() / transform.m11();
    return (fabs(zoomLevel - scalingFactor(Plasma::DesktopZoom))) < std::numeric_limits<double>::epsilon();
}

Widget::Widget(QGraphicsItem *parent)
  : QGraphicsItem(parent),
    d(new Private)
{
    setFlag(QGraphicsItem::ItemClipsToShape, true);
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    d->parent = dynamic_cast<Widget *>(parent);

    if (d->parent) {
        d->parent->addChild(this);
        d->parent->invalidate();
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

void Widget::setMinimumSize(const QSizeF& size)
{
    d->minimumSize = size;
}

QSizeF Widget::minimumSize() const
{
    return d->minimumSize;
}

void Widget::setMaximumSize(const QSizeF& size)
{
    d->maximumSize = size;
}

QSizeF Widget::maximumSize() const
{
    return d->maximumSize;
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
    return QRectF(pos(), size());
}

QRectF Widget::localGeometry() const
{
    return QRectF(QPointF(0.0f, 0.0f), size());
}

void Widget::setGeometry(const QRectF& geometry)
{
    prepareGeometryChange();

    setPos(geometry.topLeft());
    d->size = geometry.size();

    updateGeometry();
    update();
}

void Widget::updateGeometry()
{
    prepareGeometryChange();

    if (layout()) {
        kDebug() << (void *) this << " updating geometry to " << size();
        layout()->setGeometry(geometry());
    }
}

void Widget::invalidate()
{
    updateGeometry();

    if (parent()) {
        parent()->updateGeometry();
    }
}

QSizeF Widget::sizeHint() const
{
    return size();
}

void Widget::setSize(const QSizeF &newSize)
{
    prepareGeometryChange();
    qreal width = qBound(d->minimumSize.width(), newSize.width(), d->maximumSize.width());
    qreal height = qBound(d->minimumSize.height(), newSize.height(), d->maximumSize.height());

    d->size.setWidth(width);
    d->size.setHeight(height);

    update();
}

QSizeF Widget::size() const
{
    return d->size;
}

QRectF Widget::boundingRect() const
{
    return QRectF(QPointF(0.0f, 0.0f), size());
}

void Widget::resize(const QSizeF& size)
{
    setGeometry(QRectF(pos(), size));
}

void Widget::resize(qreal w, qreal h)
{
    resize(QSizeF(w, h));
}

Widget *Widget::parent() const
{
    return d->parent;
}

void Widget::addChild(Widget *w)
{
    if (!w) {
        return;
    }

    w->reparent(this);
    d->childList.append(w);

    qDebug("Added Child Widget : %p", (void*)w);

    if (layout()) {
        layout()->addItem(w);
        updateGeometry();
    }
}

void Widget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (d->shouldPaint(painter, transform())) {
        paintWidget(painter, option, widget);
    }
    return;
}

void Widget::paintWidget(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // Replaced by widget's own function
}

void Widget::reparent(Widget *w)
{
    d->parent = w;
    setParentItem(w);
    update();
}

} // Plasma namespace

