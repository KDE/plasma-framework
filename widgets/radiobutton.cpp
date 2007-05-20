/**
 *   Copyright (C) 2007 by Rafael Fernández López <ereslibre@gmail.com>
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

// Qt includes
#include <QtGui/QWidget>
#include <QtGui/QApplication>
#include <QtGui/QGraphicsScene>
#include <QtGui/QStyleOptionButton>
#include <QtGui/QGraphicsSceneMouseEvent>

// Radio button includes
#include "radiobutton.h"

namespace Plasma
{


/// Private section ==============================

class RadioButton::Private
{
public:
    Private();
    ~Private();

    // Attributes
    bool checked;
    bool mouseOver;
    bool mouseDown;
    QString text;
};

RadioButton::Private::Private()
    : checked(false)
    , mouseOver(false)
    , mouseDown(false)
    , text(QString())
{
}

RadioButton::Private::~Private()
{
}

/// End Private section ==========================


RadioButton::RadioButton(QGraphicsItem *parent)
    : DataVisualization()
    , QGraphicsItem(parent)
    , d(new Private)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptsHoverEvents(true);
}

RadioButton::~RadioButton()
{
    delete d;
}

QRectF RadioButton::boundingRect() const
{
    return QRectF(0, 0, 150, 30); // FIXME: this is obviously wrong
}

void RadioButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyle *style = widget ? widget->style() : QApplication::style();

    QStyleOptionButton radioButtonOption;
    radioButtonOption.rect = option->rect;
    radioButtonOption.text = d->text;
    radioButtonOption.state = option->state;
    radioButtonOption.state |= d->checked ? QStyle::State_On : QStyle::State_Off;
    radioButtonOption.state |= d->mouseOver ? QStyle::State_MouseOver : QStyle::State_None;
    radioButtonOption.state |= d->mouseDown ? QStyle::State_Sunken : QStyle::State_Raised;

    style->drawControl(QStyle::CE_RadioButton, &radioButtonOption, painter, widget);
}

bool RadioButton::isChecked() const
{
    return d->checked;
}

const QString &RadioButton::text() const
{
    return d->text;
}

void RadioButton::setChecked(bool checked)
{
    RadioButton *siblingRadioButton;
    // If we have a parent item (some kind of grouping widget or whatever)
    // check first there
    if (d->mouseOver && checked && parentItem())
    {
        foreach(QGraphicsItem *sibling, parentItem()->children())
        {
            siblingRadioButton = dynamic_cast<RadioButton*>(sibling);

            if (siblingRadioButton && siblingRadioButton->isChecked())
            {
                siblingRadioButton->setChecked(false);
                break; // Only an item is checked at same time as maximum
            }
        }
    }
    // If not, we should be on a scene, not flying anywhere
    else if (checked && !parentItem() && scene())
    {
        foreach(QGraphicsItem *sibling, scene()->items())
        {
            siblingRadioButton = dynamic_cast<RadioButton*>(sibling);

            if (siblingRadioButton && siblingRadioButton->isChecked() && !siblingRadioButton->parentItem())
            {
                siblingRadioButton->setChecked(false);
                break; // Only an item is checked at same time as maximum
            }
        }
    }

    d->checked = checked;

    update();
}

void RadioButton::setText(const QString &text)
{
    d->text = text;
    update();
}

void RadioButton::updated(const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(data);
}

void RadioButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    d->mouseDown = true;
    update();
}

void RadioButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    d->mouseDown = false;
    setChecked(true);
    update();

    emit clicked();
}

void RadioButton::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    d->mouseOver = true;
    update();
}

void RadioButton::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    event->accept();
    d->mouseOver = true;
    update();
}

void RadioButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    event->accept();
    d->mouseOver = false;
    update();
}

void RadioButton::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    event->accept();
    d->mouseOver = true;
    update();
}


} // Plasma namespace

#include "radiobutton.moc"
