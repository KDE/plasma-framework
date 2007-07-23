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

#include "pushbutton.h"

#include <QStyleOptionFrameV2>
#include <QStyleOption>
#include <QStyle>
#include <QWidget>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QFontMetricsF>
#include <QApplication>

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
        int radius;
        QTimer *updateTimer;
        PushButton::ButtonState state;
};

PushButton::PushButton(Widget *parent)
    : QObject(),
      Widget(parent),
      d(new Private)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptsHoverEvents(true);
    setEnabled(true);

    resize(40.0f, 100.0f);
    setPos(QPointF(0.0,0.0));

    /*FIXME: Don't use hardcoded strings and colors. */

    d->state = PushButton::None;
    d->labelText = QString("Plasma");
    d->labelTextColor = QColor(201, 201, 255);
    d->hasIcon = false;
    d->iconSize = QSize(32,32);
}

PushButton::~PushButton()
{
    delete d;
}

void PushButton::updated(const QString&, const DataEngine::Data &data)
{
    Q_UNUSED(data)
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
    widget->style()->drawControl(QStyle::CE_PushButton, &options, painter, widget);
}

void PushButton::setText(const QString& text)
{
    d->labelText = text;
/*
    QFont * font = new QFont (text);
    QFontMetrics  *  fm  = new QFontMetrics(*font);
    if (fm->width(text) >= d->width)
        setWidth(fm->width(d->labelText) + 4);
    delete fm;
    delete font;
*/

}

QString PushButton::text() const
{
    return d->labelText;
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

bool PushButton::isDown()
{
    if (d->state == PushButton::Pressed)
        return true;
    return false;
}

void PushButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    d->state = PushButton::Pressed;
    update();
//     emit clicked();
}

void PushButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    if (d->state == PushButton::Pressed)
        emit clicked();
    d->state = PushButton::Released;
    update();
}

QSizeF PushButton::sizeHint() const
{
    return minimumSize();
}

QSizeF PushButton::minimumSize() const
{
    QFontMetricsF m = qApp->fontMetrics();

    return m.boundingRect(text()).size() + QSizeF(5.0f, 5.0f);
}

QSizeF PushButton::maximumSize() const
{
    return QSizeF();
}

Qt::Orientations PushButton::expandingDirections() const
{
    return Qt::Horizontal;
}

bool PushButton::isEmpty() const
{
    return false;
}

} // namespace Plasma
