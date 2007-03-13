/*
 *   Copyright (C) 2007 by Siraj Razick siraj@kde.org
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
#include <QDebug>

#include "pushbutton.h"
#include "pushbutton.moc"

namespace Plasma
{

class PushButton::Private
{
    public:
        Private() {}
        ~Private() {}

        QString labelText;
        QString labelIcon;
        QColor  labelTextColor;
        QIcon icon;
        QSize iconSize;
        bool hasIcon;
        int labelTextOpacity;
        int height;
        int width;
        int maxWidth;
        int minWidth;
        int minHeight;
        int maxHeight;
        int radius;
        QTimer * updateTimer;
        PushButton::ButtonState state;
};

PushButton::PushButton(QGraphicsItem *parent)
    :   DataVisualization(),
        QGraphicsItem(parent),
        QLayoutItem (Qt::AlignHCenter),
        d(new Private)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptsHoverEvents(true);
    setEnabled(true);

    d->height = 40;
    d->width = 100 ;
    d->minWidth = d->width;
    d->maxWidth = d->width;
    d->minHeight = d->height;
    d->maxHeight = d->height;
    setPos(QPointF(0.0,0.0));
    d->state = PushButton::None;
    d->labelText = tr("Plasma");
    d->labelTextColor = QColor(201,201,255);
    d->hasIcon = false;
    d->iconSize = QSize(32,32);
}

PushButton::~PushButton()
{
    delete d;
}

QRectF PushButton::boundingRect() const
{
    return QRectF(x(), y(), d->width, d->height);
}

void PushButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionButton options;
    options.initFrom(widget);
    options.state = option->state;
    options.state |= isDown() ? QStyle::State_Sunken : QStyle::State_Raised;
    options.rect = boundingRect().toRect();
    options.text = text();

    if (d->hasIcon)
    {
       options.icon= d->icon;
       options.iconSize = d->iconSize;
    }

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

    QFont font(text);
    QFontMetrics fm(font);
    if (fm.width(text) >= d->width)
        setWidth(fm.width(text) + 4);
}

QString PushButton::text() const
{
    return d->labelText;
}

int PushButton::height() const
{
    return d->height;
}

int PushButton::width() const
{
    return d->width;
}

void PushButton::setHeight(int h)
{
    prepareGeometryChange();
    d->height = h;
    update();
}

void PushButton::setWidth(int w)
{
    if (!(w >= d->maxWidth))
    {
        prepareGeometryChange ();
        d->width = w;
        update();
    }
}

void PushButton::setIcon(const QString& path)
{
    if (!path.isNull())
    {
        QPixmap iconPixmap(path);
        d->icon = QIcon(iconPixmap);
        d->iconSize = iconPixmap.size();
        d->hasIcon=true;
    }
    else
        d->hasIcon = false;
}

QSize PushButton::size() const
{
    return QSize(d->width, d->height);
}

void PushButton::setSize(QSize s)
{
    prepareGeometryChange();
    if (!d->maxWidth >= s.width())
        d->width = s.width();
    d->height = s.height();
    update();
}

void PushButton::setMaximumWidth(int w)
{
    d->maxWidth= w;
}

bool PushButton::isDown()
{
    if (d->state == PushButton::Pressed)
        return true;
    return false;
}

void PushButton::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    event->accept();
    d->state = PushButton::Pressed;
    update();
   // emit clicked();
}

void PushButton::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
{
    event->accept();
    d->state = PushButton::RELEASED;
    update();
    emit clicked();
}

QSize PushButton::sizeHint() const
{
    return QSize(d->width,d->height);
}

QSize PushButton::minimumSize() const
{
    return QSize(d->minWidth,d->minHeight);
}

QSize PushButton::maximumSize() const
{
    return QSize(d->maxWidth,d->maxHeight);
}

Qt::Orientations PushButton::expandingDirections() const 
{
    return Qt::Horizontal;
}

void PushButton::setGeometry(const QRect & r)
{
    setSize(r.size());
    setPos(r.x(),r.y());
}

QRect PushButton::geometry() const
{
    return boundingRect().toRect();
}

} // namespace Plasma
