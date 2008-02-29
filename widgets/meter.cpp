/*
 *   Copyright (C) 2007 Petri Damsten <damu@iki.fi>
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

#include "meter.h"
#include "plasma/svg.h"
#include <kdebug.h>
#include <QPainter>

namespace Plasma {

class Meter::Private
{
public:
    Private() :
        minimum(0),
        maximum(100),
        value(0),
        meterType(AnalogMeter),
        image(0),
        sizeHint(QSizeF(0.0, 0.0)),
        minrotate(0),
        maxrotate(360) {};

    void paint(QPainter *p, const QString& elementID)
    {
        if (image->elementExists(elementID)) {
            QRectF elementRect = image->elementRect(elementID);
            image->paint(p, elementRect.topLeft(), elementID);
        }
    }

    void text(QPainter *p, int index)
    {
        QString elementID = QString("label%1").arg(index);
        QString text = labels[index];

        if (image->elementExists(elementID)) {
            QRectF elementRect = image->elementRect(elementID);
            Qt::Alignment align = Qt::AlignCenter;

            if (colors.count() > index) {
                p->setPen(QPen(colors[index]));
            }
            if (fonts.count() > index) {
                p->setFont(fonts[index]);
            }
            if (alignments.count() > index) {
                align = alignments[index];
            }
            if (elementRect.width() > elementRect.height()) {
                p->drawText(elementRect, align, text);
            } else {
                p->save();
                QPointF rotateCenter(
                        elementRect.left() + elementRect.width() / 2,
                        elementRect.top() + elementRect.height() / 2);
                p->translate(rotateCenter);
                p->rotate(-90);
                p->translate(elementRect.height() / -2,
                             elementRect.width() / -2);
                QRectF r(0, 0, elementRect.height(), elementRect.width());
                p->drawText(r, align, text);
                p->restore();
            }
        }
    }

    void paintBackground(QPainter *p)
    {
        paint(p, "background");
        p->save();
    }

    void paintForeground(QPainter *p)
    {
        p->restore();
        for (int i = 0; i < labels.count(); ++i) {
            text(p, i);
        }
        paint(p, "foreground");
    }

    int minimum;
    int maximum;
    int value;
    QStringList labels;
    QList<Qt::Alignment> alignments;
    QList<QColor> colors;
    QList<QFont> fonts;
    QString svg;
    MeterType meterType;
    Plasma::Svg *image;
    QSizeF sizeHint;
    int minrotate;
    int maxrotate;
};

Meter::Meter(QGraphicsItem *parent) :
        Plasma::Widget(parent),
        d(new Private)
{
}

Meter::~Meter()
{
    delete d;
}

void Meter::setMaximum(int maximum)
{
    d->maximum = maximum;
}

int Meter::maximum() const
{
    return d->maximum;
}

void Meter::setMinimum(int minimum)
{
    d->minimum = minimum;
}

int Meter::minimum() const
{
    return d->minimum;
}

void Meter::setValue(int value)
{
    d->value = value;
    update();
}

int Meter::value() const
{
    return d->value;
}

void Meter::setLabel(int index, const QString &text)
{
    while (d->labels.count() <= index) {
        d->labels << QString();
    }
    d->labels[index] = text;
}

QString Meter::label(int index) const
{
    return d->labels[index];
}

void Meter::setLabelColor(int index, const QColor &color)
{
    while (d->colors.count() <= index) {
        d->colors << color;
    }
    d->colors[index] = color;
}

QColor Meter::labelColor(int index) const
{
    return d->colors[index];
}

void Meter::setLabelFont(int index, const QFont &font)
{
    while (d->fonts.count() <= index) {
        d->fonts << font;
    }
    d->fonts[index] = font;
}

QFont Meter::labelFont(int index) const
{
    return d->fonts[index];
}

void Meter::setLabelAlignment(int index, Qt::Alignment alignment)
{
    while (d->alignments.count() <= index) {
        d->alignments << alignment;
    }
    d->alignments[index] = alignment;
}

Qt::Alignment Meter::labelAlignment(int index) const
{
    return d->alignments[index];
}

void Meter::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(sourceName)

    foreach (QVariant d, data) {
        if (d.canConvert(QVariant::Int)) {
            setValue(d.toInt());
            return;
        }
    }
}

void Meter::setSvg(const QString &svg)
{
    d->svg = svg;
    delete d->image;
    d->image = new Plasma::Svg(svg, this);
    // To create renderer and get default size
    d->image->resize();
    d->sizeHint = d->image->size();
    if (d->image->elementExists("rotateminmax")) {
        QRectF r = d->image->elementRect("rotateminmax");
        d->minrotate = (int)r.height();
        d->maxrotate = (int)r.width();
    }
}

QString Meter::svg() const
{
    return d->svg;
}

void Meter::setMeterType(MeterType meterType)
{
    d->meterType = meterType;
    if (d->svg.isEmpty()) {
        if (meterType == BarMeterHorizontal) {
            setSvg("widgets/bar_meter_horizontal");
        } else if (meterType == BarMeterVertical) {
            setSvg("widgets/bar_meter_vertical");
        } else if (meterType == AnalogMeter) {
            setSvg("widgets/analog_meter");
        }
    }
}

Meter::MeterType Meter::meterType() const
{
    return d->meterType;
}

QSizeF Meter::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    return d->sizeHint;
}

void Meter::paintWidget(QPainter *p,
                            const QStyleOptionGraphicsItem *option,
                            QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    QRectF rect(QPointF(0, 0), size());
    QRectF clipRect;
    qreal percentage = 0.0;
    qreal angle = 0.0;
    QPointF rotateCenter;
    QSize intSize = QSize((int)size().width(), (int)size().height());

    if (intSize != d->image->size()) {
        d->image->resize(intSize);
    }

    if (d->maximum != d->minimum) {
        percentage = (qreal)d->value / (d->maximum - d->minimum);
    }
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    switch (d->meterType) {
    case BarMeterHorizontal:
    case BarMeterVertical:
        d->paintBackground(p);

        clipRect = d->image->elementRect("bar");
        if (clipRect.width() > clipRect.height()) {
            clipRect.setWidth(clipRect.width() * percentage);
        } else {
            qreal bottom = clipRect.bottom();
            clipRect.setHeight(clipRect.height() * percentage);
            clipRect.moveBottom(bottom);
        }
        p->setClipRect(clipRect);
        d->paint(p, "bar");

        d->paintForeground(p);
        break;
    case AnalogMeter:
        d->paintBackground(p);

        if (d->image->elementExists("rotatecenter")) {
            QRectF r = d->image->elementRect("rotatecenter");
            rotateCenter = QPointF(r.left() + r.width() / 2,
                                   r.top() + r.height() / 2);
        } else {
            rotateCenter = QPointF(rect.width() / 2, rect.height() / 2);
        }
        angle = percentage * (d->maxrotate - d->minrotate) + d->minrotate;

        p->translate(rotateCenter);
        p->rotate(angle);
        p->translate(-1 * rotateCenter);
        d->paint(p, "pointer");

        d->paintForeground(p);
        break;
    }
}

} // End of namepace

#include "meter.moc"
