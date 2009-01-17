/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2009 Davide Bettio <davide.bettio@kdemail.net>
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

#include "spinbox.h"

#include <QApplication>
#include <QPainter>
#include <knuminput.h>
#include <kmimetype.h>

#include <plasma/theme.h>
#include <plasma/private/style_p.h>

namespace Plasma
{

class SpinBoxPrivate
{
public:
    SpinBoxPrivate(SpinBox *spinBox)
        : q(spinBox)
    {
    }

    ~SpinBoxPrivate()
    {
    }

    void setPalette()
    {
        QSpinBox *native = q->nativeWidget();
        QColor color = Theme::defaultTheme()->color(Theme::TextColor);
        QPalette p = native->palette();

        p.setColor(QPalette::Normal, QPalette::Text, color);
        p.setColor(QPalette::Inactive, QPalette::Text, color);
        p.setColor(QPalette::Normal, QPalette::ButtonText, color);
        p.setColor(QPalette::Inactive, QPalette::ButtonText, color);
        p.setColor(QPalette::Normal, QPalette::Base, QColor(0,0,0,0));
        p.setColor(QPalette::Inactive, QPalette::Base, QColor(0,0,0,0));
        native->setPalette(p);
        native->setFont(Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont));
    }

    SpinBox *q;
    Plasma::Style::Ptr style;
};

SpinBox::SpinBox(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new SpinBoxPrivate(this))
{
    KIntSpinBox *native = new KIntSpinBox;

    connect(native, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged(int)));
    connect(native, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));

    setWidget(native);
    native->setAttribute(Qt::WA_NoSystemBackground);
    native->setAutoFillBackground(false);

    d->style = Plasma::Style::sharedStyle();
    native->setStyle(d->style.data());
    d->setPalette();
    connect(Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(setPalette()));
}

SpinBox::~SpinBox()
{
    delete d;
    Plasma::Style::doneWithSharedStyle();
}

void SpinBox::setMaximum(int max)
{
    static_cast<KIntSpinBox*>(widget())->setMaximum(max);
}

int SpinBox::maximum() const
{
    return static_cast<KIntSpinBox*>(widget())->maximum();
}

void SpinBox::setMinimum(int min)
{
    static_cast<KIntSpinBox*>(widget())->setMinimum(min);
}

int SpinBox::minimum() const
{
    return static_cast<KIntSpinBox*>(widget())->minimum();
}

void SpinBox::setRange(int min, int max)
{
    static_cast<KIntSpinBox*>(widget())->setRange(min, max);
}

void SpinBox::setValue(int value)
{
    static_cast<KIntSpinBox*>(widget())->setValue(value);
}

int SpinBox::value() const
{
    return static_cast<KIntSpinBox*>(widget())->value();
}

void SpinBox::setStyleSheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString SpinBox::styleSheet()
{
    return widget()->styleSheet();
}

KIntSpinBox *SpinBox::nativeWidget() const
{
    return static_cast<KIntSpinBox*>(widget());
}

} // namespace Plasma

#include <spinbox.moc>

