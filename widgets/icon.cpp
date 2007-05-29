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

#include <QPainter>
#include <QGraphicsSceneMouseEvent>

#include <KIcon>
#include <KDebug>

#include "svg.h"
#include "icon.h"

namespace Plasma
{

class Icon::Private
{
    public:
        Private()
            : size(40, 40),
              state(Icon::None),
              background("widgets/iconbg"),
              backgroundPressed("widgets/iconbgpressed"),
              foreground("widgets/iconfg"),
              foregroundHover("widgets/iconfghover"),
              foregroundPressed("widgets/iconfgpressed")
        {
            minSize = size;
            maxSize = size;
            state = Icon::None;
        }
        ~Private() {}

        QString text;
        QIcon icon;
        QSizeF size;
        QSizeF minSize;
        QSizeF maxSize;
        Icon::ButtonState state;
        Svg background;
        Svg backgroundPressed;
        Svg foreground;
        Svg foregroundHover;
        Svg foregroundPressed;
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
    return QRectF(pos(), d->size);
}

void Icon::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    QRectF rect = boundingRect();

    switch (d->state) {
        case None:
        case Hover:
            d->background.paint(painter, 0, 0);
            break;
        case Pressed:
            d->backgroundPressed.paint(painter, 0, 0);
            break;
    }

    if (!d->icon.isNull()) {
        int deltaX = d->size.width() * 0.05;
        int deltaY = d->size.height() * 0.05;
        painter->drawPixmap(deltaX, deltaY, d->icon.pixmap((d->size * 0.9).toSize()));
    }

    //TODO: draw text

    switch (d->state) {
        case None:
            d->foreground.paint(painter, 0, 0);
            break;
        case Hover:
            d->foregroundHover.paint(painter, 0, 0);
            break;
        case Pressed:
            d->foregroundPressed.paint(painter, 0, 0);
            break;
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

void Icon::setSize(const QSizeF& s)
{
    prepareGeometryChange();
    d->size = s.boundedTo(d->maxSize); //FIXME: maxSize always == size means it can be changed. wtf. =)
    d->background.resize(s);
    d->backgroundPressed.resize(s);
    d->foreground.resize(s);
    d->foregroundHover.resize(s);
    d->foregroundPressed.resize(s);
    update();
}

void Icon::setSize(int w, int h)
{
    setSize(QSizeF(w, h));
}

bool Icon::isDown()
{
    return d->state == Icon::Pressed;
}

void Icon::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    d->state = Icon::Pressed;
    update();
    emit pressed(true);
}

void Icon::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    bool wasClicked = d->state == Icon::Pressed && boundingRect().contains(event->scenePos());
    d->state = Icon::None;
    update();

    if (wasClicked) {
        emit pressed(false);
        emit clicked();
    }
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

} // namespace Plasma

#include "icon.moc"
