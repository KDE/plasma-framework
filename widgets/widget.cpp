/*
 *   Copyright 2007 by Alexander Wiedenbruch <mail@wiedenbruch.de>
 *                      and Matias Valdenegro <mvaldenegro@informatica.utem.cl>
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


#include "widget.h"

#include <cmath>
#include <limits>

#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsView>
#include <QHelpEvent>
#include <QList>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDesktopWidget>
#include <QGraphicsLayout>
#include <QGraphicsLinearLayout>

#include <KDebug>

#include "plasma/applet.h"

#include "layouts/freelayout.h"
#include "plasma/plasma.h"
#include "plasma/view.h"
#include "plasma/containment.h"
#include "plasma/widgets/tooltip_p.h"

namespace Plasma
{

class Widget::Private
{
    public:
        Private()
            : minimumSize(0,0),
              maximumSize(std::numeric_limits<qreal>::infinity(),
                          std::numeric_limits<qreal>::infinity()),
              opacity(1.0),
              wasMovable(false),
              toolTip(0)
        { }

        ~Private()
        {
            delete toolTip;
        }

        QSizeF minimumSize;
        QSizeF maximumSize;

        qreal opacity;

        bool wasMovable;

        bool shouldPaint(QPainter *painter, const QTransform &transform);
        ToolTipData *toolTip;
};

QGraphicsItem* Widget::graphicsItem()
{
    return this;
}

QGraphicsView *Widget::view() const
{
    // It's assumed that we won't be visible on more than one view here.
    // Anything that actually needs view() should only really care about
    // one of them anyway though.
    if (!scene()) {
        return 0;
    }

    foreach (QGraphicsView *view, scene()->views()) {
        if (view->sceneRect().intersects(sceneBoundingRect()) ||
            view->sceneRect().contains(scenePos())) {
            return view;
        }
    }
    return 0;
}

QRectF Widget::mapFromView(const QGraphicsView *view, const QRect &rect) const
{
    // TODO: Confirm that adjusted() is needed and is not covering for some
    // issue elsewhere
    return mapFromScene(view->mapToScene(rect)).boundingRect().adjusted(0, 0, 1, 1);
}

QRect Widget::mapToView(const QGraphicsView *view, const QRectF &rect) const
{
    // TODO: Confirm that adjusted() is needed and is not covering for some
    // issue elsewhere
    return view->mapFromScene(mapToScene(rect)).boundingRect().adjusted(0, 0, -1, -1);
}

bool Widget::Private::shouldPaint(QPainter *painter, const QTransform &transform)
{
    Q_UNUSED(painter)
    Q_UNUSED(transform)
    //qreal zoomLevel = painter->transform().m11() / transform.m11();
    //return (fabs(zoomLevel - scalingFactor(Plasma::DesktopZoom))) < std::numeric_limits<double>::epsilon();
    return true;
}

Widget::Widget(QGraphicsItem *parent, QObject* parentObject)
  : QGraphicsWidget(parent),
    d(new Private)
{
    setFlag(QGraphicsItem::ItemClipsToShape, true);
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setCacheMode(DeviceCoordinateCache);

    Widget *w = dynamic_cast<Widget *>(parent);
    if (w) {
        w->addChild(this);
    }
}

Widget::~Widget()
{
    if (ToolTip::self()->currentWidget() == this) {
        ToolTip::self()->hide();
    }
    delete d;
}

void Widget::setOpacity(qreal opacity)
{
    d->opacity = opacity;
}

qreal Widget::opacity() const
{
    return d->opacity;
}

void Widget::setCachePaintMode(CachePaintMode mode, const QSize &size)
{
    setCacheMode(CacheMode(mode), size);
}

Widget::CachePaintMode Widget::cachePaintMode() const
{
    return CachePaintMode(cacheMode());
}

void Widget::update(const QRectF &rect)
{
    QGraphicsWidget::update(rect);
}

Qt::Orientations Widget::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
}

QFont Widget::font() const
{
    return QApplication::font();
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

void Widget::setSize(const QSizeF &s)
{
    resize(s);
}

Widget *Widget::parent() const
{
    return parent(this);
}

Widget *Widget::parent(const QGraphicsItem *item)
{
    Q_ASSERT(item);
    QGraphicsItem *parent = item->parentItem();

    while (parent) {
        Widget *parentWidget = dynamic_cast<Widget *>(parent);

        if (parentWidget) {
            return parentWidget;
        }

        parent = parent->parentItem();
    }
    return 0;
}

void Widget::addChild(Widget *w)
{
    if (!w || QGraphicsItem::children().contains(w)) {
        return;
    }

    w->setParentItem(this);

    //kDebug() << "Added Child Widget" <<  (QObject*)w << "our geom is" << geometry();
    QGraphicsLinearLayout * lay = dynamic_cast<QGraphicsLinearLayout *>(layout());
    if (lay) {
        lay->addItem(w);
    }

    updateGeometry();
    //kDebug() << "after the item is added our geom is now" << geometry();
}

void Widget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (d->opacity < 1.0) {
        painter->setOpacity(painter->opacity() * d->opacity);
    }

    /*
NOTE: put this back if we end up needing to control when things paint due to, e.g. zooming.
    if (!d->shouldPaint(painter, transform())) {
        return;
    }
    */

    paintWidget(painter, option, widget);
}

const ToolTipData* Widget::toolTip() const
{
    return d->toolTip;
}

void Widget::setToolTip(const ToolTipData &tip)
{
    if (tip.image.isNull() &&
        tip.subText.isEmpty() &&
        tip.mainText.isEmpty()) {
        delete d->toolTip;
        d->toolTip = 0;
        return;
    }

    if (!d->toolTip) {
        d->toolTip = new ToolTipData;
    }

    *d->toolTip = tip;

    // this does a check to ensure the current widget is us
    ToolTip::self()->setData(this, *d->toolTip);
}

void Widget::updateToolTip(bool update)
{
    Q_UNUSED(update)
}

void Widget::paintWidget(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // Replaced by widget's own function
}

QPoint Widget::popupPosition(const QSize &s) const
{
    QGraphicsView *v = view();
    Q_ASSERT(v);

    QPoint pos = v->mapFromScene(scenePos());
    pos = v->mapToGlobal(pos);
    kDebug() << "==> position is" << scenePos() << v->mapFromScene(scenePos()) << pos;
    Plasma::View *pv = dynamic_cast<Plasma::View *>(v);

    Plasma::Location loc = Floating;
    if (pv) {
        loc = pv->containment()->location();
    }

    switch (loc) {
    case BottomEdge:
        pos = QPoint(pos.x(), pos.y() - s.height());
        break;
    case TopEdge:
        pos = QPoint(pos.x(), pos.y() + (int)size().height());
        break;
    case LeftEdge:
        pos = QPoint(pos.x() + (int)size().width(), pos.y());
        break;
    case RightEdge:
        pos = QPoint(pos.x() - s.width(), pos.y());
        break;
    default:
        if (pos.y() - s.height() > 0) {
             pos = QPoint(pos.x(), pos.y() - s.height());
        } else {
             pos = QPoint(pos.x(), pos.y() + (int)size().height());
        }
    }

    //are we out of screen?

    QRect screenRect = QApplication::desktop()->screenGeometry(pv ? pv->containment()->screen() : -1);
    kDebug() << "==> rect for" << (pv ? pv->containment()->screen() : -1) << "is" << screenRect;

    if (pos.rx() + s.width() > screenRect.right()) {
        pos.rx() -= ((pos.rx() + s.width()) - screenRect.right());
    }

    if (pos.ry() + s.height() > screenRect.bottom()) {
        pos.ry() -= ((pos.ry() + s.height()) - screenRect.bottom());
    }
    pos.rx() = qMax(0, pos.rx());

    return pos;
}

void Widget::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    // HACK: QGraphicsItem's documentation says that the event will be passed
    // to the parent if it's not handled, but it isn't passed. This can be
    // removed when Qt4.4 becomes a requirement. See Qt bug #176902.
    Widget *parentWidget = parent();
    if (parentWidget) {
        parentWidget->contextMenuEvent(event);
    }
}

bool Widget::sceneEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::GraphicsSceneHoverMove:
        // If the tooltip isn't visible, run through showing the tooltip again
        // so that it only becomes visible after a stationary hover
        if (ToolTip::self()->isVisible()) {
            break;
        }

    case QEvent::GraphicsSceneHoverEnter:
    {
        // Check that there is a tooltip to show
        if (!d->toolTip) {
            break;
        }

        // If the mouse is in the widget's area at the time that it is being
        // created the widget can receive a hover event before it is fully
        // initialized, in which case view() will return 0.
        QGraphicsView *parentView = view();
        if (parentView) {
            ToolTip::self()->show(this);
        }

        break;
    }

    case QEvent::GraphicsSceneHoverLeave:
        ToolTip::self()->delayedHide();
        break;

    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneWheel:
        ToolTip::self()->hide();

    default:
        break;
    }

    return QGraphicsItem::sceneEvent(event);
}

} // Plasma namespace

