/*
 *   Copyright (C) 2007  Jon de Andres Frias <jondeandres@gmail.com>
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

#include "progressbar.h"

#include <QWidget>
#include <QStyleOptionProgressBarV2>
#include <QPainter>
#include <QStyle>
#include <QtGlobal>
#include <QApplication>

#include <kdebug.h>

namespace Plasma
{

class ProgressBar::Private
{

public:
    Private() {}
    ~Private() {}

    QString format;
    QString text;
    Qt::Alignment alignment;
    bool invertedAppearance;
    bool textVisible;
    int maximum;
    int minimum;
    int value;
    int lastPaintedValue;
};

ProgressBar::ProgressBar(Widget *parent)
        : Plasma::Widget(parent),
        d(new Private)
{
    init();
}

ProgressBar::~ProgressBar()
{
    delete d;
}

void ProgressBar::init()
{
    setAlignment(Qt::AlignLeft);
    setFormat(QLatin1String("%p%"));
    setInvertedAppearance(false);
    setTextVisible(true);
    setMinimum(0);
    setMaximum(100);
    setValue(-1);

    d->lastPaintedValue = -1;
}

void ProgressBar::setAlignment(Qt::Alignment alignment)
{
    if (d->alignment != alignment) {
        d->alignment = alignment;
        update();
    }
}

Qt::Alignment ProgressBar::alignment() const
{
    return d->alignment;
}

void ProgressBar::setFormat(const QString &format)
{
    if (d->format != format) {
        d->format = format;
        update();
    }
}

QString ProgressBar::format() const
{
    return d->format;
}


void ProgressBar::setInvertedAppearance(bool invert)
{
    if (d->invertedAppearance & invert) {
        return;
    }

    d->invertedAppearance = invert;

    update();

}

bool ProgressBar::invertedAppearance()
{
    return d->invertedAppearance;
}

void ProgressBar::setTextVisible(bool visible)
{
    if (d->textVisible & visible) {
        return;
    }

    d->textVisible = visible;
}

bool ProgressBar::isTextVisible() const
{
    return d->textVisible;
}


void ProgressBar::reset()
{
    d->value = d->minimum - 1;
    update();
}

void ProgressBar::setRange(int minimum, int maximum)
{
    d->minimum = minimum;
    d->maximum = maximum;

    if (d->value < (d->minimum - 1) || d->value > d->maximum) {
        reset();
    }
}

void ProgressBar::setMinimum(int minimum)
{
    setRange(minimum, qMax(d->maximum, minimum));
}

void ProgressBar::setMaximum(int maximum)
{
    setRange(qMin(d->minimum, maximum), maximum);
}

int ProgressBar::maximum() const
{
    return d->maximum;
}

int ProgressBar::minimum() const
{
    return d->minimum;
}

void ProgressBar::setValue(int value)
{
    //This line is from qprogressbar.cpp by Qt.
    if (d->value == value
            || ((value > d->maximum || value < d->minimum)
                && (d->maximum != 0 || d->minimum != 0)))
        return;

    d->value = value;
    emit valueChanged(value);

    update();

}

int ProgressBar::value() const
{
    return d->value;
}

void ProgressBar::paintWidget(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyle *style = widget ? widget->style() : QApplication::style();

    QStyleOptionProgressBarV2 options;

    options.minimum = d->minimum;
    options.maximum = d->maximum;
    options.progress = d->value;

    options.textAlignment = d->alignment;
    options.textVisible = d->textVisible;
    options.text = text();

    options.invertedAppearance = d->invertedAppearance;
    options.rect = option->rect;

    style->drawControl(QStyle::CE_ProgressBar, &options, painter, widget);
}

QSizeF ProgressBar::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    QFontMetrics fm(font());
    QStyleOptionProgressBarV2 options;

    options.minimum = d->minimum;
    options.maximum = d->maximum;
    options.progress = d->value;

    options.textAlignment = d->alignment;
    options.textVisible = d->textVisible;
    options.text = text();

    options.invertedAppearance = d->invertedAppearance;

    QStyle *style = QApplication::style();

    int cw = style->pixelMetric(QStyle::PM_ProgressBarChunkWidth,  &options);
    QSize size = QSize(cw * 7 + fm.width(QLatin1Char('0')) * 4, fm.height() + 8);

    return style->sizeFromContents(QStyle::CT_ProgressBar, &options, size);
}


/*This method has the same code that QProgressBar::text() */

QString ProgressBar::text() const
{
    qint64 totalSteps = qint64(d->maximum) - qint64(d->minimum);

    QString result = d->format;
    result.replace(QLatin1String("%m"), QString::fromLatin1("%1").arg(totalSteps));
    result.replace(QLatin1String("%v"), QString::fromLatin1("%1").arg(d->value));

    if (totalSteps == 0) {
        result.replace(QLatin1String("%p"), QString::fromLatin1("%1").arg(100));
        return result;
    }

    int progress = int(((qreal(d->value) - qreal(d->minimum)) * 100.0) / totalSteps);
    result.replace(QLatin1String("%p"), QString::fromLatin1("%1").arg(progress));
    return result;
}

}

#include "progressbar.moc"
