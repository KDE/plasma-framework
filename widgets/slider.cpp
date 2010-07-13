/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#include "slider.h"

#include <QApplication>
#include <QPainter>
#include <QSlider>
#include <QStyleOptionSlider>
#include <QGraphicsSceneWheelEvent>
#include <kmimetype.h>

#include "theme.h"
#include "framesvg.h"

#include "private/style_p.h"
#include "private/focusindicator_p.h"

namespace Plasma
{

class SliderPrivate
{
public:
    SliderPrivate()
    {
    }

    ~SliderPrivate()
    {
    }

    Plasma::FrameSvg *background;
    Plasma::Style::Ptr style;
    FocusIndicator *focusIndicator;
};

Slider::Slider(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new SliderPrivate)
{
    QSlider *native = new QSlider;

    connect(native, SIGNAL(sliderMoved(int)), this, SIGNAL(sliderMoved(int)));
    connect(native, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged(int)));

    d->focusIndicator = new FocusIndicator(this, "widgets/slider");

    setWidget(native);
    native->setWindowIcon(QIcon());
    native->setAttribute(Qt::WA_NoSystemBackground);

    d->background = new Plasma::FrameSvg(this);
    d->background->setImagePath("widgets/slider");

    d->style = Plasma::Style::sharedStyle();
    native->setStyle(d->style.data());
}

Slider::~Slider()
{
    delete d;
    Plasma::Style::doneWithSharedStyle();
}

void Slider::paint(QPainter *painter,
                   const QStyleOptionGraphicsItem *option,
                   QWidget *widget)
{
    if (!styleSheet().isNull() || Theme::defaultTheme()->useNativeWidgetStyle()) {
        QGraphicsProxyWidget::paint(painter, option, widget);
        return;
    }

    QSlider *slider = nativeWidget();
    QStyle *style = slider->style();
    QStyleOptionSlider sliderOpt;
    sliderOpt.initFrom(slider);

    //init the other stuff in the slider, taken from initStyleOption()
    sliderOpt.subControls = QStyle::SC_None;
    sliderOpt.activeSubControls = QStyle::SC_None;
    sliderOpt.orientation = slider->orientation();
    sliderOpt.maximum = slider->maximum();
    sliderOpt.minimum = slider->minimum();
    sliderOpt.tickPosition = (QSlider::TickPosition)slider->tickPosition();
    sliderOpt.tickInterval = slider->tickInterval();
    sliderOpt.upsideDown = (slider->orientation() == Qt::Horizontal) ?
                     (slider->invertedAppearance() != (sliderOpt.direction == Qt::RightToLeft))
                     : (!slider->invertedAppearance());
    sliderOpt.direction = Qt::LeftToRight; // we use the upsideDown option instead
    sliderOpt.sliderPosition = slider->sliderPosition();
    sliderOpt.sliderValue = slider->value();
    sliderOpt.singleStep = slider->singleStep();
    sliderOpt.pageStep = slider->pageStep();
    if (slider->orientation() == Qt::Horizontal) {
        sliderOpt.state |= QStyle::State_Horizontal;
    }

    QRect backgroundRect =
        style->subControlRect(QStyle::CC_Slider, &sliderOpt, QStyle::SC_SliderGroove, slider);

    if (sliderOpt.orientation == Qt::Horizontal &&
        d->background->hasElement("horizontal-background-center")) {
        d->background->setElementPrefix("horizontal-background");
        d->background->resizeFrame(backgroundRect.size());
        d->background->paintFrame(painter, backgroundRect.topLeft());
    } else if (sliderOpt.orientation == Qt::Vertical &&
        d->background->hasElement("vertical-background-center")) {
        d->background->setElementPrefix("vertical-background");
        d->background->resizeFrame(backgroundRect.size());
        d->background->paintFrame(painter, backgroundRect.topLeft());
    } else if (sliderOpt.orientation == Qt::Horizontal) {
        QRect elementRect = d->background->elementRect("horizontal-slider-line").toRect();
        elementRect.setWidth(sliderOpt.rect.width());
        elementRect.moveCenter(sliderOpt.rect.center());
        d->background->paint(painter, elementRect, "horizontal-slider-line");
    } else {
        QRect elementRect = d->background->elementRect("vertical-slider-line").toRect();
        elementRect.setHeight(sliderOpt.rect.height());
        elementRect.moveCenter(sliderOpt.rect.center());
        d->background->paint(painter, elementRect, "vertical-slider-line");
    }

    //Tickmarks
    if (sliderOpt.tickPosition != QSlider::NoTicks) {
        sliderOpt.subControls = QStyle::SC_SliderTickmarks;
        sliderOpt.palette.setColor(
            QPalette::WindowText, Plasma::Theme::defaultTheme()->color(Theme::TextColor));
        style->drawComplexControl(QStyle::CC_Slider, &sliderOpt, painter, slider);
    }

    QRect handleRect = style->subControlRect(QStyle::CC_Slider, &sliderOpt, QStyle::SC_SliderHandle, slider);

    QString handle;
    if (sliderOpt.orientation == Qt::Horizontal) {
        handle = "horizontal-slider-handle";
    } else {
        handle = "vertical-slider-handle";
    }

    QRect elementRect = d->background->elementRect(handle).toRect();
    elementRect.moveCenter(handleRect.center());
    if (elementRect.right() > rect().right()) {
        elementRect.moveRight(rect().right());
    }

    if (elementRect.left() < rect().left()) {
        elementRect.moveLeft(rect().left());
    }

    if (elementRect.top() < rect().top()) {
        elementRect.moveTop(rect().top());
    }

    if (elementRect.bottom() > rect().bottom()) {
        elementRect.moveBottom(rect().bottom());
    }

    if (orientation() == Qt::Vertical) {
        d->focusIndicator->setCustomPrefix("vertical-slider");
    } else {
        d->focusIndicator->setCustomPrefix("horizontal-slider");
    }
    d->focusIndicator->setCustomGeometry(elementRect);
    d->background->paint(painter, elementRect, handle);
}

void Slider::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    QWheelEvent e(event->pos().toPoint(), event->delta(),event->buttons(),event->modifiers(),event->orientation());
    QApplication::sendEvent(widget(), &e);
    event->accept();
}

void Slider::setMaximum(int max)
{
    static_cast<QSlider*>(widget())->setMaximum(max);
}

int Slider::maximum() const
{
    return static_cast<QSlider*>(widget())->maximum();
}

void Slider::setMinimum(int min)
{
    static_cast<QSlider*>(widget())->setMinimum(min);
}

int Slider::minimum() const
{
    return static_cast<QSlider*>(widget())->minimum();
}

void Slider::setRange(int min, int max)
{
    static_cast<QSlider*>(widget())->setRange(min, max);
}

void Slider::setValue(int value)
{
    static_cast<QSlider*>(widget())->setValue(value);
}

int Slider::value() const
{
    return static_cast<QSlider*>(widget())->value();
}

void Slider::setOrientation(Qt::Orientation orientation)
{
    static_cast<QSlider*>(widget())->setOrientation(orientation);
}

Qt::Orientation Slider::orientation() const
{
    return static_cast<QSlider*>(widget())->orientation();
}

void Slider::setStyleSheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString Slider::styleSheet()
{
    return widget()->styleSheet();
}

QSlider *Slider::nativeWidget() const
{
    return static_cast<QSlider*>(widget());
}

} // namespace Plasma

#include <slider.moc>

