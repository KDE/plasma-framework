/*
 *   Copyright 2007 by Dan Meltzer <hydrogen@notyetimplemented.com>
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
#include "tooltip_p.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QTimer>
#include <QGraphicsView>

#include <kglobal.h>

namespace Plasma {

class ToolTip::Private
{
    public:
        Private()
        : label(0)
        , imageLabel(0)
        , currentWidget(0)
        , isShown(false)
        , showTimer(0)
        , hideTimer(0)
    { }

    QLabel *label;
    QLabel *imageLabel;
    Plasma::Widget *currentWidget;
    bool isShown;
    QTimer *showTimer;
    QTimer *hideTimer;
};

class ToolTipSingleton
{
    public:
        ToolTip self;
};
K_GLOBAL_STATIC( ToolTipSingleton, privateInstance )

ToolTip *ToolTip::instance()
{
    return &privateInstance->self;
}

void ToolTip::show(const QPoint &location, Plasma::Widget *widget)
{
    d->currentWidget = widget;
    setData(widget->toolTip());
    move(location.x(), location.y() - sizeHint().height());
    if (d->isShown) {
        // Don't delay if the tooltip is already shown(i.e. moving from one task to another)
        // Qt doesn't seem to like visible tooltips moving though, so hide it and then
        // immediately show it again
        setVisible(false);
        d->showTimer->start(0);
    } else {
        d->showTimer->start(1000);  //Shown after a one second delay.
    }
}

void ToolTip::hide()
{
    d->currentWidget = 0;
    d->showTimer->stop();  //Mouse out, stop the timer to show the tooltip
    if (!isVisible()) {
        d->isShown = false;
    }
    setVisible(false);
    d->hideTimer->start(500);  //500 ms delay before we are officially "gone" to allow for the time to move between widgets
}

Plasma::Widget *ToolTip::currentWidget() const
{
    return d->currentWidget;
}

//PRIVATE FUNCTIONS
void ToolTip::slotShowToolTip()
{
    if ( d->currentWidget->view()->mouseGrabber() )
	return;

    d->isShown = true;  //ToolTip is visible
    setVisible(true);
}

void ToolTip::slotResetTimer()
{
    if (!isVisible()) { //One might have moused out and back in again
        d->isShown = false;
    }
}

ToolTip::ToolTip()
    : QWidget(0)
    , d( new Private )
{
    setWindowFlags(Qt::ToolTip);
    QHBoxLayout *l = new QHBoxLayout;
    d->label = new QLabel;
    d->label->setWordWrap(true);
    d->imageLabel = new QLabel;
    d->imageLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    l->addWidget(d->imageLabel);
    l->addWidget(d->label);
    setLayout(l);

    d->showTimer = new QTimer(this);
    d->showTimer->setSingleShot(true);
    d->hideTimer = new QTimer(this);
    d->hideTimer->setSingleShot(true);

    connect(d->showTimer, SIGNAL(timeout()), SLOT(slotShowToolTip()));
    connect(d->hideTimer, SIGNAL(timeout()), SLOT(slotResetTimer()));
}

void ToolTip::setData(const Plasma::ToolTipData &data)
{
    d->label->setText("<qt><h3>" + data.mainText + "</h3><p>" +
                        data.subText + "</p></qt>");
    d->imageLabel->setPixmap(data.image);
}

ToolTip::~ToolTip()
{
    delete d;
}

}
#include "tooltip_p.moc"
