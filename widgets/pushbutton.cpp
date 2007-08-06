/*
 *   Copyright 2007 by Siraj Razick <siraj@kde.org>
 *   Copyright 2007 by Matt Broadstone <mbroadst@gmail.com>
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

#include <QStylePainter>
#include <QStyleOptionButton>
#include <QStyle>
#include <QWidget>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QFontMetricsF>
#include <QApplication>

namespace Plasma
{

class PushButton::Private
{
public:
    enum ButtonShape
    {
        Rectangle = 0,
        Round,
        Custom
    };

    enum ButtonState
    {
        None,
        Hover,
        Pressed,
        Released
    };

public:
    Private()
        : flat(false),
          state(None)
    {}

    void init(PushButton *button);
    void initStyleOption(QStyleOptionButton *option, const PushButton *button,
            const QStyleOptionGraphicsItem *graphicsOption = 0 ) const;

    QString text;
    KIcon icon;
    QSizeF iconSize;
    QSizeF size;
    bool flat;
    ButtonState state;
};

void PushButton::Private::init(PushButton *button)
{
    button->setAcceptedMouseButtons(Qt::LeftButton);
    button->setAcceptsHoverEvents(true);
    button->setEnabled(true);
}

void PushButton::Private::initStyleOption(QStyleOptionButton *option, const PushButton *button,
    const QStyleOptionGraphicsItem *graphicsOption) const
{
    option->state = QStyle::State_None;
    if (button->isEnabled()) {
        option->state |= QStyle::State_Enabled;
    }
    if (button->hasFocus()) {
        option->state |= QStyle::State_HasFocus;
    }
    if (state == Private::Hover) {
        option->state |= QStyle::State_MouseOver;
    }

    if (graphicsOption) {
        option->palette = graphicsOption->palette;
        option->fontMetrics = graphicsOption->fontMetrics;
        option->rect = graphicsOption->rect;
    } else {
        option->palette = QApplication::palette();  // pretty good guess
        option->fontMetrics = QApplication::fontMetrics();  // hrm
    }

    option->features = QStyleOptionButton::None;
    if (flat) {
        option->features |= QStyleOptionButton::Flat;
    }
    if (!flat && !(state == Private::Pressed)) {
        option->state |= QStyle::State_Raised;
    } else {
        option->state |= QStyle::State_Sunken;
    }
    option->text = text;
    option->icon = icon;
    option->iconSize = button->iconSize().toSize();
}

PushButton::PushButton(Widget *parent)
    : Plasma::Widget(parent),
      d(new Private)
{
    d->init(this);
}

PushButton::PushButton(const QString &text, Widget *parent)
    : Plasma::Widget(parent),
      d(new Private)
{
    d->init(this);
    setText(text);
}

PushButton::PushButton(const KIcon &icon, const QString &text, Widget *parent)
    : Plasma::Widget(parent),
      d(new Private)
{
    d->init(this);
    setText(text);
    setIcon(icon);
}

QRectF PushButton::boundingRect() const
{
    if (!d->size.isValid()) {
        int width = 0;
        int height = 0;

        QStyleOptionButton option;
        d->initStyleOption(&option, this);

        if (!icon().isNull()) {
            height += qMax(option.iconSize.height(), height);
            width += 2 + option.iconSize.width() + 2;  // add margin
        }

        QString display(option.text);
        if (display.isEmpty())
            display = "Plasma";

        QSize textSize = option.fontMetrics.size(Qt::TextShowMnemonic, display);
        width += textSize.width();
        height = qMax(height, textSize.height());
        d->size = QSizeF((QApplication::style()->sizeFromContents(QStyle::CT_PushButton, &option, QSize(width, height), 0).
                    expandedTo(QApplication::globalStrut())));
    }

    return QRectF(QPointF(0.0, 0.0), d->size);
}

PushButton::~PushButton()
{
    delete d;
}

void PushButton::paintWidget(QPainter *painter, const QStyleOptionGraphicsItem *opt, QWidget *widget)
{
    QStyleOptionButton option;
    option.initFrom(widget);
    d->initStyleOption(&option, this, opt);

    widget->style()->drawPrimitive(QStyle::PE_PanelButtonCommand, &option, painter, widget);
    widget->style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, painter, widget);
    widget->style()->drawControl(QStyle::CE_PushButton, &option, painter, widget);
}

void PushButton::setText(const QString& text)
{
    d->text = text;
    d->size = QSizeF();
    update();
}

QString PushButton::text() const
{
    return d->text;
}

void PushButton::setIcon(const KIcon &icon)
{
    d->icon = icon;
    d->size = QSizeF();
    update();
}

void PushButton::setIcon(const QString &path)
{
    KIcon icon(path);
    setIcon(icon);
}

KIcon PushButton::icon() const
{
    return d->icon;
}

QSizeF PushButton::iconSize() const
{
    if (d->iconSize.isValid())
        return d->iconSize;

    qreal metric = qreal(QApplication::style()->pixelMetric(QStyle::PM_ButtonIconSize));
    return QSizeF(metric, metric);
}

void PushButton::setIconSize(const QSizeF &size)
{
    if (d->iconSize == size)
        return;

    d->iconSize = size;
    d->size = QSizeF();
    update();
}

bool PushButton::isDown() const
{
    return (d->state == Private::Pressed);
}

bool PushButton::isFlat() const
{
    return d->flat;
}

void PushButton::setFlat(bool flat)
{
    d->flat = flat;
    update();
}

void PushButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    d->state = Private::Pressed;
    update();
}

void PushButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    if (d->state == Private::Pressed)
        emit clicked();
    d->state = Private::Released;
    update();
}

QSizeF PushButton::minimumSize() const
{
    QFontMetricsF m = qApp->fontMetrics();
    return m.boundingRect(text()).size() + QSizeF(5.0f, 5.0f);
}

Qt::Orientations PushButton::expandingDirections() const
{
    return Qt::Horizontal;
}

QSizeF PushButton::sizeHint() const
{
    return minimumSize();
}


} // namespace Plasma

#include "pushbutton.moc"

