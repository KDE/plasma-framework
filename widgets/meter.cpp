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
#include "private/meter_p.h"

#include <cmath>

#include <QPainter>
#include <QTimeLine>
#include <QPropertyAnimation>

#include <kdebug.h>
#include <kglobalsettings.h>

#include "plasma/animator.h"
#include "plasma/framesvg.h"
#include "plasma/theme.h"

namespace Plasma {

MeterPrivate::MeterPrivate(Meter *m)
        : QObject(m),
          minimum(0),
          maximum(100),
          value(0),
          targetValue(0),
          meterType(Meter::AnalogMeter),
          image(0),
          minrotate(0),
          maxrotate(360),
          meter(m)
{
}

void MeterPrivate::progressChanged(int progress)
    {
        value = progress;
        meter->update();
    }

void MeterPrivate::paint(QPainter *p, const QString &elementID)
    {
        if (image->hasElement(elementID)) {
            QRectF elementRect = image->elementRect(elementID);
            image->paint(p, elementRect, elementID);
        }
    }

void MeterPrivate::text(QPainter *p, int index)
    {
        QString elementID = QString("label%1").arg(index);
        QString text = labels[index];

        if (image->hasElement(elementID)) {
            QRectF elementRect = image->elementRect(elementID);
            Qt::Alignment align = Qt::AlignCenter;


            if (colors.count() > index) {
                p->setPen(QPen(colors[index]));
            } else {
                p->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
            }
            if (fonts.count() > index) {
                p->setFont(fonts[index]);
            }

            QFontMetricsF fm(p->font());
            // If the height is too small increase the Height of the button to shall the whole text #192988
            if (elementRect.height() < fm.height()) {
                QPointF oldCenter = elementRect.center();
                elementRect.setHeight(fm.height());
                elementRect.moveCenter(oldCenter);
            }

            if (alignments.count() > index) {
                align = alignments[index];
            }
            if (elementRect.width() > elementRect.height()) {
                if (align&Qt::AlignLeft) {
                    p->drawText(elementRect.bottomLeft(), text);
                } else {
                    p->drawText(elementRect, align, text);
                }
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

QRectF MeterPrivate::barRect()
    {
        QRectF elementRect;

        if (labels.count() > 0) {
            elementRect = image->elementRect("background");
        } else {
            elementRect = QRectF(QPoint(0,0), meter->size());
        }

        if (image->hasElement("hint-bar-stretch") || !image->hasElement("bar-active-center")) {
            return elementRect;
        }

        QSize imageSize = image->size();
        image->resize();
        QSize tileSize = image->elementSize("bar-active-center");
        image->resize(imageSize);

        if (elementRect.width() > elementRect.height()) {
            qreal ratio = qMax(1, tileSize.height() / tileSize.width());
            int numTiles = qMax(qreal(1.0), qreal(elementRect.width())/(qreal(elementRect.height())/ratio));
            tileSize = QSize(elementRect.width()/numTiles, elementRect.height());

            QPoint center = elementRect.center().toPoint();
            elementRect.setWidth(tileSize.width()*numTiles);
            elementRect.moveCenter(center);
        } else {
            qreal ratio = qMax(1, tileSize.width() / tileSize.height());
            int numTiles = qMax(qreal(1.0), qreal(elementRect.height())/(qreal(elementRect.width())/ratio));
            tileSize = QSize(elementRect.width(), elementRect.height()/numTiles);

            QPoint center = elementRect.center().toPoint();
            elementRect.setHeight(tileSize.height()*numTiles);
            elementRect.moveCenter(center);
        }

        return elementRect;
    }

void MeterPrivate::paintBackground(QPainter *p)
    {
        //be retrocompatible with themes for kde <= 4.1
        if (image->hasElement("background-center")) {
            QRectF elementRect = barRect();
            if (elementRect.isEmpty()) {
                return; // nothing to be done
            }

            QSize imageSize = image->size();
            image->resize();

            image->setElementPrefix("background");
            image->resizeFrame(elementRect.size());
            image->paintFrame(p, elementRect.topLeft());
            image->resize(imageSize);

            paintBar(p, "bar-inactive");
        } else {
            paint(p, "background");
        }
    }

void MeterPrivate::paintBar(QPainter *p, const QString &prefix)
    {
        QRectF elementRect = barRect();

        image->setUsingRenderingCache(false);
        if (image->hasElement("hint-bar-stretch")) {
            image->resizeFrame(elementRect.size());
            image->paintFrame(p);
        } else {
            QSize imageSize = image->size();
            image->resize();
            QSize tileSize = image->elementSize("bar-active-center");

            if (elementRect.width() > elementRect.height()) {
                qreal ratio = tileSize.height() / tileSize.width();
                int numTiles = elementRect.width()/(elementRect.height()/ratio);
                tileSize = QSize(elementRect.width()/numTiles, elementRect.height());
            } else {
                qreal ratio = tileSize.width() / tileSize.height();
                int numTiles = elementRect.height()/(elementRect.width()/ratio);
                tileSize = QSize(elementRect.width(), elementRect.height()/numTiles);
            }

            image->setElementPrefix(prefix);
            image->resizeFrame(tileSize);
            p->drawTiledPixmap(elementRect, image->framePixmap());
            image->resize(imageSize);
        }
        image->setUsingRenderingCache(true);
    }

void MeterPrivate::paintForeground(QPainter *p)
    {
        for (int i = 0; i < labels.count(); ++i) {
            text(p, i);
        }

        paint(p, "foreground");
    }

void MeterPrivate::setSizePolicyAndPreferredSize()
    {
        switch (meterType) {
            case Meter::BarMeterHorizontal:
                meter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
                break;
            case Meter::BarMeterVertical:
                meter->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
                break;
            case Meter::AnalogMeter:
            default:
                meter->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
                break;
        }

        if (image) {
            //set a sane preferredSize. We can't just use the svg's native size, since that way
            //letters get cut off if the user uses a font larger then usual. Check how many rows of
            //labels we have, add 1 (the progress bar), and multiply by the font height to get a
            //somewhat sane size height. This is not perfect but work well enough for 4.2. I suggest
            //we look into alternatives for 4.3.
            uint i = 0;
            uint rows = 0;
            qreal prevY = -1;
            QString labelName = "label0";
            while (image->hasElement(labelName)) {
                if (image->elementRect(labelName).y() > prevY) {
                    prevY = image->elementRect(labelName).y();
                    rows++;
                }
                i++;
                labelName = QString("label%0").arg(i);
            }

            Plasma::Theme *theme = Plasma::Theme::defaultTheme();
            QFont font = theme->font(Plasma::Theme::DefaultFont);
            QFontMetrics fm(font);

            meter->setPreferredHeight((rows + 1) * fm.height());
        } else {
            meter->setPreferredSize(QSizeF(30, 30));
        }
    }

Meter::Meter(QGraphicsItem *parent) :
        QGraphicsWidget(parent),
        d(new MeterPrivate(this))
{
    d->setSizePolicyAndPreferredSize();

    d->animation = new QPropertyAnimation(d, "meterValue");
}

Meter::~Meter()
{
    delete d->animation;
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

int Meter::value() const
{
    return d->value;
}

void Meter::setValue(int value)
{
    if (value == d->targetValue) {
        return;
    }

    d->targetValue = qBound(d->minimum, value, d->maximum);
    int delta = abs(d->value - d->targetValue);

    if (d->animation->state() != QAbstractAnimation::Running) {
        d->animation->stop();
    }

    //kDebug() << d->targetValue << d->value << delta;
    if (!(KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects) ||
        delta / qreal(d->maximum) < 0.1) {
        d->value = value;
        update();
    } else  {
        d->animation->setStartValue(d->value);
        d->animation->setEndValue(value);
        d->animation->start();
    }
}

int MeterPrivate::meterValue() const
{
    return value;
}

void MeterPrivate::setMeterValue(int value)
{
    progressChanged(value);
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

void Meter::setLabelAlignment(int index, const Qt::Alignment alignment)
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

QRectF Meter::labelRect(int index) const
{
    QString elementID = QString("label%1").arg(index);
    return d->image->elementRect(elementID);
}

void Meter::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(sourceName)

    foreach (const QVariant &v, data) {
        if (v.type() == QVariant::Int ||
            v.type() == QVariant::UInt ||
            v.type() == QVariant::LongLong ||
            v.type() == QVariant::ULongLong) {
            setValue(v.toInt());
            return;
        }
    }
}

void Meter::setSvg(const QString &svg)
{
    if (d->svg == svg) {
        return;
    }

    d->svg = svg;
    delete d->image;
    d->image = new Plasma::FrameSvg(this);
    d->image->setImagePath(svg);
    // To create renderer and get default size
    d->image->resize();
    d->setSizePolicyAndPreferredSize();
    if (d->image->hasElement("rotateminmax")) {
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
    d->setSizePolicyAndPreferredSize();
}

Meter::MeterType Meter::meterType() const
{
    return d->meterType;
}

void Meter::paint(QPainter *p,
                  const QStyleOptionGraphicsItem *option,
                  QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (d->svg.isEmpty()) {
        setMeterType(d->meterType);
    }

    if (!d->image) {
        return;
    }

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
        percentage = (qreal)(d->value - d->minimum) / (d->maximum - d->minimum);
    }

    p->setRenderHint(QPainter::SmoothPixmapTransform);
    switch (d->meterType) {
    case BarMeterHorizontal:
    case BarMeterVertical:
        d->paintBackground(p);

        p->save();
        clipRect = d->barRect();
        if (clipRect.width() > clipRect.height()) {
            clipRect.setWidth(clipRect.width() * percentage);
        } else {
            qreal bottom = clipRect.bottom();
            clipRect.setHeight(clipRect.height() * percentage);
            clipRect.moveBottom(bottom);
        }
        p->setClipRect(clipRect, Qt::IntersectClip);

        //be retrocompatible
        if (d->image->hasElement("bar-active-center")) {
            d->paintBar(p, "bar-active");
        } else {
            d->paint(p, "bar");
        }
        p->restore();

        d->paintForeground(p);
        break;
    case AnalogMeter:
        d->paintBackground(p);

        p->save();
        if (d->image->hasElement("rotatecenter")) {
            QRectF r = d->image->elementRect("rotatecenter");
            rotateCenter = QPointF(r.left() + r.width() / 2,
                                   r.top() + r.height() / 2);
        } else {
            rotateCenter = QPointF(rect.width() / 2, rect.height() / 2);
        }
        angle = percentage * (d->maxrotate - d->minrotate) + d->minrotate;

        if (d->image->hasElement("pointer-shadow")) {
            p->save();
            p->translate(rotateCenter+QPoint(2,3));
            p->rotate(angle);
            p->translate(-1 * rotateCenter);
            d->paint(p, "pointer-shadow");
            p->restore();
        }

        p->translate(rotateCenter);
        p->rotate(angle);
        p->translate(-1 * rotateCenter);
        d->paint(p, "pointer");
        p->restore();

        d->paintForeground(p);
        break;
    }
}

QSizeF Meter::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    return QGraphicsWidget::sizeHint(which, constraint);
}

} // End of namepace

#include "meter.moc"
#include "../private/meter_p.moc"
