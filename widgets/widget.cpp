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

#include <KDebug>

#include <QtCore/QList>

#include "layout.h"

namespace Plasma
{

class Widget::Private
{
    public:
        Private()
            : parent(0),
              layout(0)
        { }
        ~Private() { }

        QSizeF size;
        QSizeF minimumSize;
        QSizeF maximumSize;

        Widget *parent;
        Layout *layout;
        QList<Widget *> childList;
};

Widget::Widget(QGraphicsItem *parent)
  : QGraphicsItem(parent),
    d(new Private)
{
    setFlag(QGraphicsItem::ItemClipsToShape, true);

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
    if (layout()) {

        kDebug() << (void *) this << " updating geometry to " << size() << endl;

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

void Widget::setLayout(Layout *l)
{
    if(!d->layout) {
        d->layout = l;
    } else {
        kDebug() << "Widget " << this << "already has a layout!" << endl;
    }
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
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // do nothing, but we need to reimp so we can create Widget items as this method
    // is pure virtual in QGraphicsItem
}

void Widget::reparent(Widget *w)
{
    d->parent = w;
    setParentItem(w);
    update();
}

} // Plasma namespace

