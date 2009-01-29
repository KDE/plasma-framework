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
          rotation(0)
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

void BusyWidget::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)

    d->rotation += d->rotationAngle;

    qreal overflow = d->rotation - 360;
    if ( overflow > 0) {
        d->rotation = overflow;
    }
    update();
}

void BusyWidget::paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    int intRotation = int(d->rotation);

    if (!d->frames[intRotation]) {
        QPointF translatedPos(size().width()/2.0, size().height()/2.0);

        d->frames[intRotation] = QPixmap(size().toSize());
        d->frames[intRotation].fill(Qt::transparent);

        QPainter buffPainter(&d->frames[intRotation]);

        buffPainter.setRenderHints(QPainter::SmoothPixmapTransform);
        buffPainter.translate(translatedPos);

        if (d->svg->hasElement("busywidget-shadow")) {
            buffPainter.save();
            buffPainter.translate(2,2);
            buffPainter.rotate(intRotation);
            d->svg->paint(&buffPainter, QRect(-translatedPos.toPoint(), size().toSize()), "busywidget-shadow");
            buffPainter.restore();
        }

        buffPainter.rotate(intRotation);
        d->svg->paint(&buffPainter, QRect(-translatedPos.toPoint(), size().toSize()), "busywidget");
    }

    painter->drawPixmap(QPoint(0,0), d->frames[intRotation]);
}

void BusyWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    d->timerId = startTimer(150);
}

void BusyWidget::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)
    killTimer(d->timerId);
    d->timerId = 0;
}

void BusyWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    d->frames.clear();
}

} // namespace Plasma

#include <busywidget.moc>

