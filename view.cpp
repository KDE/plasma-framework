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

#include <KWindowSystem>

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
          desktop(-1),
          containment(0)
    {
    }

    ~Private()
    {
    }

    bool drawWallpaper;
    int desktop;
    Plasma::Containment *containment;
};

View::View(Containment *containment, QWidget *parent)
    : QGraphicsView(parent),
      d(new Private)
{
    Q_ASSERT(containment);
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

        Containment *containment = corona->containmentForScreen(screen);
        if (containment) {
            d->containment = 0; //so that we don't end up on the old containment's screen
            setContainment(containment);
        }
    }
}

int View::screen() const
{
    if (d->containment) {
        return d->containment->screen();
    }

    return -1;
}

void View::setDesktop(int desktop)
{
    // -1 == All desktops
    if (desktop < -1 || desktop > KWindowSystem::numberOfDesktops() - 1) {
        desktop = -1;
    }

    d->desktop = desktop;
}

int View::desktop() const
{
    return d->desktop;
}

int View::effectiveDesktop() const
{
    return d->desktop > -1 ? d->desktop : KWindowSystem::currentDesktop();
}

void View::setContainment(Containment *containment)
{
    if (containment == d->containment) {
        return;
    }

    int screen = -1;

    if (d->containment) {
        disconnect(d->containment, SIGNAL(geometryChanged()), this, SLOT(updateSceneRect()));
        screen = d->containment->screen();
    }

    d->containment = containment;
    if (! containment) {
        return;
    }
    
    if (screen > -1) {
        containment->setScreen(screen);
    }

    if (containment->screen() > -1 && d->desktop < -1) {
        // we want to set it to "all desktops" if we get ownership of
        // a screen but don't have a desktop explicitly set
        d->desktop = -1;
    }

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
    if (!d->containment) {
        return;
    }

    kDebug() << "!!!!!!!!!!!!!!!!! setting the scene rect to"
             << d->containment->sceneBoundingRect()
             << "associated screen is" << d->containment->screen();

    emit sceneRectAboutToChange();
    setSceneRect(d->containment->sceneBoundingRect());
    emit sceneRectChanged();
}

} // namespace Plasma

#include "view.moc"

