/*
 *   Copyright (C) 2007 by Siraj Razick siraj@kdemail.net
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

#include <QStyleOptionFrameV2>
#include <QStyleOption>
#include <QStyle>
#include <QWidget>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

#include "pushbutton.h"
#include "pushbutton.moc"

namespace Plasma
{

class PushButton::Private
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
            int maxWidth;
            int radius;
            QTimer * updateTimer;
            PushButton::ButtonState state;
};

PushButton::PushButton(QGraphicsItem *parent)
    :   QGraphicsItem(parent),
        DataVisualization(),
        d(new Private)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptsHoverEvents(true);
    setEnabled(true);
    d->height = 40;
    d->width = 100 ;
    d->maxWidth = 600;
    setPos(QPointF(0.0,0.0));
    d->state= PushButton::NONE;
    d->labelText=tr("Plasma");
    d->labelTextColor= QColor(201,201,255);

}


PushButton::~PushButton()
{
}

QRectF PushButton::boundingRect() const
{
    return QRectF(x(),y(),d->width,d->height);
}

void PushButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionButton options;
    options.initFrom(widget);
    options.state = option->state;
    options.rect = boundingRect().toRect();
    options.text = text();
 
    widget->style()->drawPrimitive(QStyle::PE_PanelButtonCommand, &options, painter, widget);
    widget->style()->drawPrimitive(QStyle::PE_FrameFocusRect, &options, painter, widget);
    widget-> style()->drawControl(QStyle::CE_PushButton, &options, painter, widget);

}


void PushButton::data(const DataSource::Data&)
{
}

void PushButton::setText(const QString& text)
{
    d->labelText = text;
    QFont * _font = new QFont ( text );
    QFontMetrics fm ( *_font );
    if ( fm.width(text) >= d->width ) {
        setWidth(fm.width(text)+4);
    }

    delete _font;

}

QString PushButton::text()
{
    return d->labelText;
}
int PushButton::height()
{
    return d->height;
}

int PushButton::width()
{
    return d->width;
}

void PushButton::setHeight(int h)
{
    prepareGeometryChange ();
    d->height = h;
    update();
}

void PushButton::setWidth(int w)
{
    if (!(w >= d->maxWidth)) {
    prepareGeometryChange ();
    d->width = w;
    update();
    }
}

QSize PushButton::size()
{
    return QSize(d->width,d->height);
}

void PushButton::setSize(QSize s)
{
    prepareGeometryChange ();
  if (!d->maxWidth  >= s.width() ) {
        d->width = s.width();
  }
    d->height = s.height();
    update();
}

void PushButton::setMaximumWidth(int w)
{
    d->maxWidth= w;
}

bool PushButton::isDown()
{
    if (d->state == PushButton::PRESSED) {
    return true;
    }
    return false;
}

void PushButton::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    event->accept();
    d->state = PushButton::PRESSED;
    update();
    emit clicked();
}
} // namespace Plasma
