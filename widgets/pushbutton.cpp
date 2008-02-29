/*
 *   Copyright 2007 by Siraj Razick <siraj@kde.org>
 *   Copyright 2007 by Matt Broadstone <mbroadst@gmail.com>
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

#include "pushbutton.h"

#include <QStylePainter>
#include <QStyleOptionButton>
#include <QStyle>
#include <QWidget>
#include <QPainter>
#include <QGraphicsScene>
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
          checkable(false),
          checked(false),
          state(None)
    {}

    void init(PushButton *button);
    void initStyleOption(QStyleOptionButton *option, const PushButton *button,
            const QStyleOptionGraphicsItem *graphicsOption = 0 ) const;

    QString text;
    KIcon icon;
    QSizeF iconSize;
    bool flat;
    bool checkable;
    bool checked;
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
    if (!flat && !(checked || state == Private::Pressed)) {
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
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed,QSizePolicy::PushButton);
}

PushButton::PushButton(const QString &text, Widget *parent)
    : Plasma::Widget(parent),
      d(new Private)
{
    d->init(this);
    setText(text);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed,QSizePolicy::PushButton);
}

PushButton::PushButton(const KIcon &icon, const QString &text, Widget *parent)
    : Plasma::Widget(parent),
      d(new Private)
{
    d->init(this);
    setText(text);
    setIcon(icon);
}

PushButton::~PushButton()
{
    delete d;
}

void PushButton::paintWidget(QPainter *painter, const QStyleOptionGraphicsItem *opt, QWidget *widget)
{
    QStyleOptionButton option;
    if (widget) {
        option.initFrom(widget);
    }
    d->initStyleOption(&option, this, opt);

    QApplication::style()->drawPrimitive(QStyle::PE_PanelButtonCommand, &option, painter, widget);
    QApplication::style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, painter, widget);
    QApplication::style()->drawControl(QStyle::CE_PushButton, &option, painter, widget);
}

void PushButton::setText(const QString& text)
{
    d->text = text;
    update();
}

QString PushButton::text() const
{
    return d->text;
}

void PushButton::setIcon(const KIcon &icon)
{
    d->icon = icon;
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

bool PushButton::isChecked() const
{
    return d->checked;
}

void PushButton::setChecked(bool checked)
{
    if (isCheckable()) {
        d->checked = checked;
        update();
    }
}

bool PushButton::isCheckable() const
{
    return d->checkable;
}

void PushButton::setCheckable(bool checkable)
{
    d->checkable = checkable;
}

void PushButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    d->state = Private::Pressed;
    update();
}

void PushButton::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    if (sceneBoundingRect().contains(event->scenePos())) {
        if (d->state == Private::Released && scene()->mouseGrabberItem() == this) {
            d->state = Private::Pressed;
            update();
        }
    } else {
        if (d->state == Private::Pressed) {
            d->state = Private::Released;
            update();
        }
    }
}

void PushButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    if (d->state == Private::Pressed) {
        d->state = Private::Released;
        emit clicked();

        if (d->checkable) {
            d->checked = ! d->checked;
            emit toggled(d->checked);
        }
        update();
    }
}

QSizeF PushButton::minimumSize() const
{
    QFontMetricsF m = qApp->fontMetrics();
    return m.boundingRect(text()).size() + QSizeF(5.0f, 5.0f);
}

QSizeF PushButton::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    int width = 0;
    int height = 0;

    QStyleOptionButton option;
    d->initStyleOption(&option, this);

    if (!icon().isNull()) {
        height += qMax(option.iconSize.height(), height);
        width += 2 + option.iconSize.width() + 2;  // add margin
    }

    QString display(option.text);

    QSize textSize = option.fontMetrics.size(Qt::TextShowMnemonic, display);
    width += textSize.width();
    height = qMax(height, textSize.height());

    return QSizeF((QApplication::style()->sizeFromContents(QStyle::CT_PushButton, &option, QSize(width, height), 0).
                expandedTo(QApplication::globalStrut())));
}


} // namespace Plasma

#include "pushbutton.moc"

