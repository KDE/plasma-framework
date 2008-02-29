
/*
 *   Copyright 2007 by Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
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
                 

#include "label.h"

#include <QPainter>
#include <QFontMetricsF>
#include <QStyleOptionGraphicsItem>

namespace Plasma {

class Label::Private
{
    public:
        Private()
            : maximumWidth(9999)
        {}

        QString text;
        Qt::Alignment alignment;
        QPen textPen;
        QFont textFont;
        int maximumWidth;
};

Label::Label(Widget *parent)
    : Plasma::Widget(parent),
      d(new Private)
{
    setAlignment(Qt::AlignHCenter);
    setPen(QPen(Qt::black, 1));
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding,QSizePolicy::Label);
}

Label::~Label()
{
    delete d;
}

bool Label::hasHeightForWidth() const
{
    return true;
}

qreal Label::heightForWidth(qreal w) const
{
    Q_UNUSED( w );
    //FIXME: this looks a bit odd?
    QFontMetricsF m(d->textFont);
    //return m.boundingRect(QRectF(0, 0, w, 9999), d->alignment | Qt::TextWordWrap, d->text).height();
	return 0;
}

QSizeF Label::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    QFontMetricsF m(d->textFont);
    return m.boundingRect(QRectF(0,0,9999,9999), d->alignment | Qt::TextWordWrap, d->text).size();
	//return m.boundingRect(QRectF(0, 0, d->maximumWidth, 9999), d->alignment | Qt::TextWordWrap, d->text).size();
}

void Label::setText(const QString& text)
{
    d->text = text;
    updateGeometry();
    update();
}

QString Label::text() const
{
    return d->text;
}

void Label::setAlignment(Qt::Alignment align)
{
    d->alignment = align;
}

Qt::Alignment Label::alignment() const
{
    return d->alignment;
}

void Label::setPen(const QPen& pen)
{
    d->textPen = pen;
    updateGeometry();
    update();
}

QPen Label::pen() const
{
    return d->textPen;
}

void Label::setMaximumWidth(int width)
{
    d->maximumWidth = width;
}

int Label::maximumWidth() const
{
    return d->maximumWidth;
}

void Label::setFont(const QFont& font)
{
    d->textFont = font;
    updateGeometry();
    update();
}

QFont Label::font() const
{
    return d->textFont;
}

void Label::paintWidget(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    p->setPen(d->textPen);
    p->setFont(d->textFont);
    p->drawText(option->rect, d->alignment | Qt::TextWordWrap, d->text);
}

}
