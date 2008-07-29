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

#include <QSlider>
#include <QPainter>

#include <KMimeType>

#include "theme.h"
#include "svg.h"

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
};


Slider::Slider(Qt::Orientation orientation, QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(0)
{
    QSlider* native = new QSlider;

    connect(native, SIGNAL(sliderMoved(int)), this, SIGNAL(sliderMoved(int)));
    connect(native, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged(int)));

    setWidget(native);
    native->setAttribute(Qt::WA_NoSystemBackground);
    native->setOrientation(orientation);
}

Slider::Slider(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(0)
{
    QSlider* native = new QSlider;

    connect(native, SIGNAL(sliderMoved(int)), this, SIGNAL(sliderMoved(int)));
    connect(native, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged(int)));

    setWidget(native);
    native->setAttribute(Qt::WA_NoSystemBackground);
}

Slider::~Slider()
{
    delete d;
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

QSlider* Slider::nativeWidget() const
{
    return static_cast<QSlider*>(widget());
}

} // namespace Plasma

#include <slider.moc>

