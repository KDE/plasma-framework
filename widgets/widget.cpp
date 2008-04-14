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
#include <QGraphicsLayout>
#include <QGraphicsLinearLayout>

#include <KDebug>

#include "plasma/applet.h"

#include "plasma/plasma.h"
#include "plasma/view.h"
#include "plasma/containment.h"

namespace Plasma
{

class Widget::Private
{
    public:
        Private()
            : minimumSize(0,0),
              maximumSize(std::numeric_limits<qreal>::infinity(),
                          std::numeric_limits<qreal>::infinity()),
              wasMovable(false)
              //toolTip(0)
        { }

        ~Private()
        {
            //delete toolTip;
        }

        QSizeF minimumSize;
        QSizeF maximumSize;

        bool wasMovable;

        bool shouldPaint(QPainter *painter, const QTransform &transform);
        //ToolTipData *toolTip;
};

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
}

Widget::~Widget()
{
    #ifdef TOOLTIPMANAGER
    if (ToolTip::self()->currentWidget() == this) {
        ToolTip::self()->hide();
    }
    #endif
    delete d;
}


#ifdef TOOLTIPMANAGER
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
#endif
} // Plasma namespace

