/*
 *   Copyright (C) 2007 by Aaron Seigo aseigo@kde.org
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

#include "icon.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>

#include <KIcon>
#include <KDebug>

#include "svg.h"

namespace Plasma
{

class Icon::Private
{
    public:
        Private()
            : size(128*1.1, 128*1.1),
              state(Private::NoState),
              svg("widgets/iconbutton")
        {
            svg.setContentType(Plasma::Svg::ImageSet);
            svg.resize(size);
            minSize = size;
            maxSize = size;

            if (svg.elementExists("background")) {
                svgElements |= SvgBackground;
            }

            if (svg.elementExists("background-hover")) {
                svgElements |= SvgBackgroundHover;
            }

            if (svg.elementExists("background-pressed")) {
                svgElements |= SvgBackgroundPressed;
            }

            if (svg.elementExists("foreground")) {
                svgElements |= SvgForeground;
            }

            if (svg.elementExists("foreground-hover")) {
                svgElements |= SvgForegroundHover;
            }

            if (svg.elementExists("foreground-pressed")) {
                svgElements |= SvgForegroundPressed;
            }

        }
        ~Private() {}
        enum ButtonState
        {
            NoState,
            HoverState,
            PressedState
        };


        enum { NoSvg = 0,
               SvgBackground = 1,
               SvgBackgroundHover = 2,
               SvgBackgroundPressed = 4,
               SvgForeground = 8,
               SvgForegroundHover = 16,
               SvgForegroundPressed = 32 };

        QString text;
        QIcon icon;
        QSizeF size;
        QSizeF minSize;
        QSizeF maxSize;
        ButtonState state;
        Svg svg;
        int svgElements;
};

Icon::Icon(QGraphicsItem *parent)
    : QObject(),
      QGraphicsItem(parent),
      d(new Private)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptsHoverEvents(true);
    setEnabled(true);
    setPos(QPointF(0.0,0.0));
}

Icon::~Icon()
{
    delete d;
}

QRectF Icon::boundingRect() const
{
    return QRectF(QPointF(0, 0), d->size);
}

void Icon::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

//    QRectF rect = boundingRect();

    QString element;
    if (d->svgElements & Private::SvgBackground) {
        element = "background";
    }

    switch (d->state) {
        case Private::NoState:
            break;
        case Private::HoverState:
            if (d->svgElements & Private::SvgBackgroundHover) {
                element = "background-hover";
            }
            break;
        case Private::PressedState:
            if (d->svgElements & Private::SvgBackgroundPressed) {
                element = "background-pressed";
            }
            break;
    }

    if (!element.isEmpty()) {
        //kDebug() << "painting " << element << endl;
        d->svg.paint(painter, 0, 0, element);
        element = QString();
    }


    if (!d->icon.isNull()) {
        qreal deltaX = d->size.width() * 0.1;
        qreal deltaY = d->size.height() * 0.1;
        painter->drawPixmap(deltaX, deltaY, d->icon.pixmap((d->size * 0.9).toSize()));
    }

    //TODO: draw text

    if (d->svgElements & Private::SvgForeground) {
        element = "foreground";
    }

    switch (d->state) {
        case Private::NoState:
            break;
        case Private::HoverState:
            if (d->svgElements & Private::SvgForegroundHover) {
                element = "foreground-hover";
            }
            break;
        case Private::PressedState:
            if (d->svgElements & Private::SvgForegroundPressed) {
                element = "foreground-pressed";
            }
            break;
    }

    if (!element.isEmpty()) {
        //kDebug() << "painting " << element << endl;
        d->svg.paint(painter, 0, 0, element);
    }
}

void Icon::setText(const QString& text)
{
    d->text = text;
//TODO: implement this puppy    calculateSize();
}

QString Icon::text() const
{
    return d->text;
}

void Icon::setIcon(const QString& icon)
{
    if (icon.isEmpty()) {
        setIcon(QIcon());
        return;
    }

    setIcon(KIcon(icon));
}

void Icon::setIcon(const QIcon& icon)
{
    d->icon = icon;
    update();
}

QSizeF Icon::size() const
{
    return d->size;
}

QSizeF Icon::iconSize() const
{
    return d->size * .9;
}

void Icon::setSize(const QSizeF& s)
{
    prepareGeometryChange();
    d->size = s.boundedTo(d->maxSize); //FIXME: maxSize always == size means it can be changed. wtf. =)
    d->svg.resize(d->size);
    update();
}

void Icon::setSize(int w, int h)
{
    setSize(QSizeF(w, h));
}

bool Icon::isDown()
{
    return d->state == Private::PressedState;
}

void Icon::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    d->state = Private::PressedState;
    emit pressed(true);
    QGraphicsItem::mousePressEvent(event);
    update();
}

void Icon::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    bool inside = boundingRect().contains(event->pos());
    bool wasClicked = d->state == Private::PressedState && inside;

    if (inside) {
        d->state = Private::HoverState;
    } else {
        d->state = Private::NoState;
    }

    if (wasClicked) {
        emit pressed(false);
        emit clicked();
    }
    QGraphicsItem::mouseReleaseEvent(event);
    update();
}

void Icon::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    d->state = Private::HoverState;
    QGraphicsItem::hoverEnterEvent(event);
}

void Icon::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    d->state = Private::NoState;
    QGraphicsItem::hoverLeaveEvent(event);
}

QSizeF Icon::sizeHint() const
{
    return d->size;
}

QSizeF Icon::minimumSize() const
{
    return d->minSize;
}

QSizeF Icon::maximumSize() const
{
    return d->maxSize;
}

Qt::Orientations Icon::expandingDirections() const
{
    return Qt::Horizontal;
}

bool Icon::hasHeightForWidth() const
{
    return true;
}

qreal Icon::heightForWidth(qreal w) const
{
    return w; //FIXME: we shouldn't assume squareness but actually calculate based on text and what not
}

bool Icon::hasWidthForHeight() const
{
    return true;
}

qreal Icon::widthForHeight(qreal h) const
{
    return h; //FIXME: we shouldn't assume squareness but actually calculate based on text and what not
}

QRectF Icon::geometry() const
{
    return boundingRect().toRect();
}

void Icon::setGeometry(const QRectF &r)
{
    setSize(r.size());
    setPos(r.x(),r.y());
}

int Icon::boundsForIconSize(int iconSize)
{
    return iconSize * 1.1;
}

} // namespace Plasma

#include "icon.moc"
