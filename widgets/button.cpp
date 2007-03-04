/*
 *   Copyright (C) 2007 by Siraj Razick <siraj@kdemail.net>
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
#include <QPalette>
#include <QTimer>
#include <QSvgRenderer>
#include <QGraphicsSceneMouseEvent>
#include "button.h"
#include "button.moc"
#include "theme.h"

namespace Plasma
{

class Button::Private
{
    public:
        Private() { };
        ~Private() { };
            QString labelText;
            QString labelIcon;
            QColor  labelTextColor;
            int labelTextOpacity;
            int height;
            int width;
            int radius;
            QTimer * updateTimer;
            ButtonState state;
};


Button::Button(QGraphicsItem *parent)
  : Widget(parent),
    d(new Private)
{
    setDefaultStates();
}

Button::~Button()
{
    delete d;
}

void Button::setDefaultStates()
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptsHoverEvents(true);
    setEnabled(true);
    d->height = 180;
    d->width = 260 ;
    setPos(QPointF(0.0,0.0));
    d->state= Button::NONE;
    d->labelText=tr("Ok");
    d->labelTextColor= QColor(201,201,255);
}

void Button::setText(const QString& text) const
{
    d->labelText = text;
}

QString Button::text()
{
    return d->labelText;
}

void Button::paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
    Q_UNUSED(widget)
    Q_UNUSED(option)
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter->setPen(d->labelTextColor);

    if (d->state == Button::NONE ) {
        drawSVG(painter,"buttonnormalbg");
        painter->drawText(QRectF(x(),y(),d->width,d->height), Qt::AlignCenter, this->text());
        drawSVG(painter,"buttonoverylay");
    }else if (d->state == PRESSED) {
        drawSVG(painter,"buttonnormalbg");
    //    drawSVG(painter,"buttonoverylay");
        }else {
        drawSVG(painter,"buttonhoverbg");
        painter->drawText(QRectF(x(),y(),d->width,d->height), Qt::AlignCenter, this->text());
        drawSVG(painter,"buttonoverylay");
    }
}

QRectF Button::boundingRect() const 
{
    return QRectF(x(),y(),d->width,d->height);
}

void Button::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event )
{
    d->state = Button::NONE;
    update();
}

void Button::hoverEnterEvent ( QGraphicsSceneHoverEvent * event )
{
    d->state = Button::HOVER;
    update();
}

void Button::mousePressEvent ( QGraphicsSceneMouseEvent * event)
{
    event->accept();
    Button::ButtonState tmp = d->state;
    d->state = PRESSED;
    update();
    d->state = tmp;
    update();
    emit clicked();
}

int Button::height()
{
    return d->height;
}

int Button::width()
{
    return d->width;
}

void Button::setHeight(int h)
{
    prepareGeometryChange ();
    d->height = h;
    update();
}

void Button::setWidth(int w)
{
    prepareGeometryChange ();
    d->width = w;
    update();
}

QSize Button::size()
{
    return QSize(d->width,d->height);
}

void Button::setSize(QSize s)
{
    prepareGeometryChange ();
    d->width = s.width();
    d->height = s.height();
    update();
}

void Button::drawOverlay(QPainter * painter)
{
   //TODO
   //Draw overlay when no SVG was found 
}

void Button::drawSVG(QPainter * painter , const QString& imageName)
{
    QString file = Theme().imagePath(imageName);
    if (!file.isEmpty()) {
             QSvgRenderer svgimg(file);
             svgimg.render(painter,this->boundingRect());
    }
}

void Button::drawBackDrop(QPainter * painter)
{
    //TODO
    //Drawbackground when no SVG was found    
}



} // Plasma namespace

