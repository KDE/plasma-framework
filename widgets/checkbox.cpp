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

#include <QStyleOption>
#include <QStyle>
#include <QWidget>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

#include "checkbox.h"
#include "checkbox.moc"

namespace Plasma
{

class CheckBox::Private
{
    public:
        Private() { };
        ~Private() { };
            QString labelText;
            QString labelIcon;
            QColor  labelTextColor;
            QIcon icon;
            QSize iconSize;
            bool hasIcon;
            bool hasMouse;
            int labelTextOpacity;
            int height;
            int width;
            int maxWidth;
            int radius;
            QTimer * updateTimer;
            Qt::CheckState state;
            bool down;
            bool hovering;
};

CheckBox::CheckBox(QGraphicsItem *parent)
    :   DataVisualization(),
        QGraphicsItem(parent),
        d(new Private)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptsHoverEvents(true);
    setAcceptDrops(true);
    setEnabled(true);
    d->height = 40;
    d->width = 100 ;
    d->maxWidth = 600;
    setPos(QPointF(0.0,0.0));
    d->state= Qt::Unchecked;
    d->labelText=tr("Plasma");
    d->labelTextColor= QColor(201,201,255);
    d->hasIcon = false;
    d->iconSize=QSize(32,32);
    d->hasMouse = false;
    d->down = false;
    d->hovering = false;
    d->down = false;

}


CheckBox::~CheckBox()
{
    delete d;
}

QRectF CheckBox::boundingRect() const
{
    return QRectF(x(),y(),d->width,d->height);
}

void CheckBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{


    QStyleOptionButton options;
    options.rect = boundingRect().toRect();
    options.text = text();
    options.state |= (d->state == Qt::Checked)? QStyle::State_On : QStyle::State_Off;
 
    //if (d->down) {
      //  options.state |= QStyle::State_Sunken;
   // }

    if (d->hasMouse) {

        options.state |= QStyle::State_MouseOver;
        options.state |= QStyle::State_HasFocus;
        options.state |= QStyle::State_Sunken;
        options.state |= QStyle::State_Raised;
        options.state |= QStyle::State_On;
    }
    widget-> style()->drawControl(QStyle::CE_CheckBox, &options, painter, widget);

}


void CheckBox::data(const DataSource::Data&)
{
}

void CheckBox::setText(const QString& text)
{
    d->labelText = text;
    QFont * _font = new QFont ( text );
    QFontMetrics fm ( *_font );
    if ( fm.width(text) >= d->width ) {
        setWidth(fm.width(text)+4);
    }

    delete _font;

}

QString CheckBox::text()
{
    return d->labelText;
}
int CheckBox::height()
{
    return d->height;
}

int CheckBox::width()
{
    return d->width;
}

void CheckBox::setHeight(int h)
{
    prepareGeometryChange ();
    d->height = h;
    update();
}

void CheckBox::setWidth(int w)
{
    if (!(w >= d->maxWidth)) {
    prepareGeometryChange ();
    d->width = w;
    update();
    }
}

QSize CheckBox::size()
{
    return QSize(d->width,d->height);
}

void CheckBox::setSize(QSize s)
{
    prepareGeometryChange ();
  if (!d->maxWidth  >= s.width() ) {
        d->width = s.width();
  }
    d->height = s.height();
    update();
}

void CheckBox::setMaximumWidth(int w)
{
    d->maxWidth= w;
}

void CheckBox::setCheckState(Qt::CheckState state)
{
    d->state = state;
}

void CheckBox::mouseMoveEvent (QGraphicsSceneMouseEvent * event)
{
    event->accept();
    d->hasMouse= false;

}
void CheckBox::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    event->accept();
    d->down = true;
    update();
}


void CheckBox::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
{
    event->accept();
    
    if (d->hasMouse) {
    if (d->state == Qt::Checked) {
    d->state = Qt::Unchecked;
    } else {
        d->state = Qt::Checked;
        }
    }
    update();
    emit clicked();

}


void CheckBox::hoverMoveEvent ( QGraphicsSceneHoverEvent * event )
{
    event->accept();
    d->hasMouse= true;
    update();
}


void CheckBox::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event )
{
    event->accept();
    d->hasMouse= false;
    update();
}

void  CheckBox::hoverEnterEvent ( QGraphicsSceneHoverEvent * event)
{
    event->accept();
    d->hasMouse = true;
    update();
}

/*
void CheckBox::dragLeaveEvent ( QGraphicsSceneDragDropEvent * event )
{
event->accept();
}
*/



} // namespace Plasma
