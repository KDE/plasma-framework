/*
 *   Copyright 2007 by Siraj Razick <siraj@kde.org>
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

#include "checkbox.h"

#include <QStyleOption>
#include <QStyle>
#include <QWidget>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

namespace Plasma
{

class CheckBox::Private
{
    public:
        Private() { }
        ~Private() { }
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
    : Plasma::Widget(parent),
      d(new Private)
{
    init();
}


CheckBox::CheckBox(const QString &text, QGraphicsItem *parent)
    : Plasma::Widget(parent),
      d(new Private)
{
    init();
    setText(text);
}

void CheckBox::init()
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptsHoverEvents(true);
    setAcceptDrops(true);
    setEnabled(true);

    setPos(QPointF(0.0,0.0));

    d->height = 40;
    d->width = 100 ;
    d->maxWidth = 600;
    d->state= Qt::Unchecked;

    d->hasIcon = false;
    d->iconSize = QSize(32,32);
    d->hasMouse = false;
    d->down = false;
    d->hovering = false;
    d->down = false;
}

CheckBox::~CheckBox()
{
    delete d;
}

void CheckBox::paintWidget(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)

    QStyleOptionButton options;
    options.rect = boundingRect().toRect();
    options.text = text();
    options.state |= (d->state == Qt::Checked) ? QStyle::State_On : QStyle::State_Off;

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

void CheckBox::dataUpdated(const QString&, const DataEngine::Data& data)
{
    foreach (const QVariant& variant, data) {
        if (variant.canConvert(QVariant::Bool)) {
            setChecked(variant.toBool());
            return;
        }
    }
}

void CheckBox::setText(const QString& text)
{
    d->labelText = text;
    /*
    QFont * _font = new QFont ( text );
    QFontMetrics fm ( *_font );
    if ( fm.width(text) >= d->width ) {
        setWidth(fm.width(text)+4);
    }

    delete _font;
    */
}

QString CheckBox::text() const
{
    return d->labelText;
}

Qt::CheckState CheckBox::checkState() const
{
    return d->state;
}

void CheckBox::setChecked(bool checked)
{
    d->state = checked ? Qt::Checked : Qt::Unchecked;
}

bool CheckBox::isChecked() const
{
    return (d->state == Qt::Checked);
}

void CheckBox::setCheckState(Qt::CheckState state)
{
    d->state = state;
}

void CheckBox::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    event->accept();
    d->hasMouse= false;
}

void CheckBox::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    event->accept();
    d->down = true;
    update();
}

void CheckBox::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
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

    if (sceneBoundingRect().contains(event->scenePos())) {
        emit clicked();
    }
}


void CheckBox::hoverMoveEvent(QGraphicsSceneHoverEvent * event)
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
    Widget::hoverEnterEvent(event);
}

void  CheckBox::hoverEnterEvent ( QGraphicsSceneHoverEvent * event)
{
    event->accept();
    d->hasMouse = true;
    update();
    Widget::hoverLeaveEvent(event);
}

/*
void CheckBox::dragLeaveEvent ( QGraphicsSceneDragDropEvent * event )
{
event->accept();
}
*/



} // namespace Plasma

#include "checkbox.moc"


