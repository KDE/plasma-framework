/*
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
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

#include "busywidget.h"

//Qt
#include <QPainter>
#include <QTimer>
#include <QGraphicsSceneResizeEvent>
#include <QTextOption>

//Plasma
#include "plasma/theme.h"
#include "plasma/svg.h"

namespace Plasma
{

class BusyWidgetPrivate
{
public:
    BusyWidgetPrivate()
        : svg(0),
          timerId(0),
          rotationAngle(0),
          rotation(0),
          running(true)
    {
    }

    ~BusyWidgetPrivate()
    {
    }

    void themeChanged()
    {
        frames.clear();
        rotationAngle = svg->elementSize("hint-rotation-angle").width();

        //use an angle near to rotationAngle but that it fits an integer number of times in 360
        int nFrames = 360/rotationAngle;
        rotationAngle = 360/nFrames;
    }

    Svg *svg;
    QString styleSheet;
    int timerId;
    QHash<int, QPixmap> frames;
    qreal rotationAngle;
    qreal rotation;
    bool running;
    QString label;
};


BusyWidget::BusyWidget(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      d(new BusyWidgetPrivate)
{
    d->svg = new Plasma::Svg(this);
    d->svg->setImagePath("widgets/busywidget");
    d->svg->setContainsMultipleImages(true);
    d->themeChanged();

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(themeChanged()));
}

BusyWidget::~BusyWidget()
{
    delete d;
}

void BusyWidget::setRunning(bool running)
{
    if (running && !d->timerId && isVisible()) {
        d->timerId = startTimer(150);
    } else if (!running && d->timerId) {
        killTimer(d->timerId);
        d->timerId = 0;
    }
    d->running = running;
}

bool BusyWidget::isRunning() const
{
    return d->running;
}

void BusyWidget::setLabel(const QString &label)
{
    d->label = label;
    update();
}

QString BusyWidget::label() const
{
    return d->label;
}

void BusyWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != d->timerId) {
        QObject::timerEvent(event);
        return;
    }

    d->rotation += d->rotationAngle;

    qreal overflow = d->rotation - 360;
    if ( overflow > 0) {
        d->rotation = overflow;
    }
    update();
}

void BusyWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    int intRotation = int(d->rotation);

    QRectF spinnerRect(QPoint(0, 0), QSize(qMin(size().width(), size().height()), qMin(size().width(), size().height())));
    spinnerRect.moveCenter(boundingRect().center());

    if (!isEnabled()) {
        painter->setOpacity(painter->opacity() / 2);
    }

    if (!d->running && d->svg->hasElement("paused")) {
        d->svg->paint(painter, spinnerRect, "paused");
    } else {
        if (!d->frames[intRotation]) {
            QPointF translatedPos(spinnerRect.width()/2, spinnerRect.height()/2);

            d->frames[intRotation] = QPixmap(spinnerRect.size().toSize());
            d->frames[intRotation].fill(Qt::transparent);

            QPainter buffPainter(&d->frames[intRotation]);

            buffPainter.setRenderHints(QPainter::SmoothPixmapTransform);
            buffPainter.translate(translatedPos);

            if (d->svg->hasElement("busywidget-shadow")) {
                buffPainter.save();
                buffPainter.translate(1,1);
                buffPainter.rotate(intRotation);
                d->svg->paint(&buffPainter, QRectF(-translatedPos.toPoint(), spinnerRect.size()), "busywidget-shadow");
                buffPainter.restore();
            }

            buffPainter.rotate(intRotation);
            d->svg->paint(&buffPainter, QRectF(-translatedPos.toPoint(), spinnerRect.size()), "busywidget");
        }

        painter->drawPixmap(spinnerRect.topLeft().toPoint(), d->frames[intRotation]);
    }

    painter->setPen(Plasma::Theme::defaultTheme()->color(Theme::TextColor));
    Qt::Alignment align(Qt::AlignVCenter | Qt::AlignHCenter);
    painter->drawText(boundingRect(), d->label, QTextOption(align));
}

void BusyWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    if (d->running) {
        d->timerId = startTimer(150);
    }
}

void BusyWidget::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)
    if (d->timerId) {
        killTimer(d->timerId);
    }

    d->timerId = 0;
}

void BusyWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_UNUSED(event)
    d->frames.clear();
}

void BusyWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    emit clicked();
}

} // namespace Plasma

#include <busywidget.moc>

