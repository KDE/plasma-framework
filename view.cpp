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

#include <KGlobal>
#include <KWindowSystem>
#include <KActionCollection>

#include "corona.h"
#include "containment.h"

using namespace Plasma;

namespace Plasma
{

class View::Private
{
public:
    Private(View *view, int uniqueId)
        : drawWallpaper(true),
          trackChanges(true),
          desktop(-1),
          containment(0),
          q(view),
          viewId(0)
    {
        if (uniqueId > s_maxViewId) {
            s_maxViewId = uniqueId;
            viewId = uniqueId;
        }

        if (viewId == 0) {
            // we didn't get a sane value assigned to us, so lets
            // grab the next available id
            viewId = ++s_maxViewId;
        }
    }

    ~Private()
    {
    }

    void updateSceneRect()
    {
        if (!containment || !trackChanges) {
            return;
        }

        kDebug() << "!!!!!!!!!!!!!!!!! setting the scene rect to"
                 << containment->sceneBoundingRect()
                 << "associated screen is" << containment->screen();

        emit q->sceneRectAboutToChange();
        q->setSceneRect(containment->sceneBoundingRect());
        emit q->sceneRectChanged();
    }

    void initGraphicsView()
    {
        q->setFrameShape(QFrame::NoFrame);
        q->setAutoFillBackground(true);
        q->setDragMode(QGraphicsView::NoDrag);
        //setCacheMode(QGraphicsView::CacheBackground);
        q->setInteractive(true);
        q->setAcceptDrops(true);
        q->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        q->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        q->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    bool drawWallpaper;
    bool trackChanges;
    int desktop;
    int viewId;
    Plasma::Containment *containment;
    Plasma::View *q;
    static int s_maxViewId;
};

int View::Private::s_maxViewId(0);

View::View(Containment *containment, QWidget *parent)
    : QGraphicsView(parent),
      d(new Private(this, 0))
{
    Q_ASSERT(containment);
    d->initGraphicsView();
    setScene(containment->scene());
    setContainment(containment);
}

View::View(Containment *containment, int viewId, QWidget *parent)
    : QGraphicsView(parent),
      d(new Private(this, viewId))
{
    Q_ASSERT(containment);
    d->initGraphicsView();
    setScene(containment->scene());
    setContainment(containment);
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
        //remove all the old containment's actions
        d->containment->removeAssociatedWidget(this);
    }

    d->containment = containment;
    if (! containment) {
        return;
    }

    //add keyboard-shortcut actions
    d->containment->addAssociatedWidget(this);

    if (screen > -1) {
        containment->setScreen(screen);
    }

    if (containment->screen() > -1 && d->desktop < -1) {
        // we want to set it to "all desktops" if we get ownership of
        // a screen but don't have a desktop explicitly set
        d->desktop = -1;
    }

    d->updateSceneRect();
    connect(containment, SIGNAL(geometryChanged()), this, SLOT(updateSceneRect()));
}

Containment* View::containment() const
{
    return d->containment;
}

KConfigGroup View::config() const
{
    KConfigGroup views(KGlobal::config(), "PlasmaViews");
    return KConfigGroup(&views, QString::number(d->viewId));
}

int View::id() const
{
    return d->viewId;
}

void View::setWallpaperEnabled(bool draw)
{
    d->drawWallpaper = draw;
}

bool View::isWallpaperEnabled() const
{
    return d->drawWallpaper;
}

void View::setTrackContainmentChanges(bool trackChanges)
{
    d->trackChanges = trackChanges;
}

bool View::trackContainmentChanges()
{
    return d->trackChanges;
}

View * View::topLevelViewAt(const QPoint & pos)
{
    QWidget *w = QApplication::topLevelAt(pos);
    if (w) {
        Plasma::View *v = qobject_cast<Plasma::View *>(w);
        return v;
    } else {
        return 0;
    }
}

} // namespace Plasma

#include "view.moc"

