/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
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

#include "view.h"
#include "corona.h"
#include "containment.h"

using namespace Plasma;

namespace Plasma
{

class View::Private
{
public:
    Private()
        : drawWallpaper(true),
          screen(-1),
          containment(0)
    {
    }

    ~Private()
    {
    }

    bool drawWallpaper;
    int screen;
    Plasma::Containment *containment;
};

View::View(int screen, Corona *corona, QWidget *parent)
    : QGraphicsView(parent),
      d(new Private)
{
    initGraphicsView();
    setScene(corona);
    setScreen(screen);
}

View::View(Containment *containment, QWidget *parent)
    : QGraphicsView(parent),
      d(new Private)
{
    initGraphicsView();
    setScene(containment->scene());
    setContainment(containment);
}

void View::initGraphicsView()
{
    setFrameShape(QFrame::NoFrame);
    setAutoFillBackground(true);
    setDragMode(QGraphicsView::NoDrag);
    //setCacheMode(QGraphicsView::CacheBackground);
    setInteractive(true);
    setAcceptDrops(true);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

View::~View()
{
    delete d;
}

void View::setScreen(int screen)
{
    if (screen > -1) {
        Corona *corona = qobject_cast<Corona*>(scene());

        if (!corona) {
            return;
        }

        setContainment(corona->containmentForScreen(screen));
    } else {
        // reseting the screen who knows what.
        d->screen = screen;
    }
}

int View::screen() const
{
    return d->screen;
}

void View::setContainment(Containment *containment)
{
    if (!containment) {
        return;
    }

    if (d->containment) {
        disconnect(d->containment, SIGNAL(geometryChanged()), this, SLOT(updateSceneRect()));
    }

    d->containment = containment;
    d->screen = containment->screen();
    updateSceneRect();
    connect(containment, SIGNAL(geometryChanged()), this, SLOT(updateSceneRect()));
}

Containment* View::containment() const
{
    return d->containment;
}

void View::setDrawWallpaper(bool draw)
{
    d->drawWallpaper = draw;
}

bool View::drawWallpaper() const
{
    return d->drawWallpaper;
}

void View::updateSceneRect()
{
    setSceneRect(d->containment->sceneBoundingRect());
}

} // namespace Plasma

#include "view.moc"

