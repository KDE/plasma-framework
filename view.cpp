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

#include <QTimer>

#include <kglobal.h>
#include <kwindowsystem.h>
#include <kactioncollection.h>

#include "corona.h"
#include "containment.h"
#include "wallpaper.h"

using namespace Plasma;

namespace Plasma
{

class ViewPrivate
{
public:
    ViewPrivate(View *view, int uniqueId)
        : q(view),
          containment(0),
          viewId(0),
          lastScreen(-1),
          lastDesktop(-2),
          drawWallpaper(true),
          trackChanges(true),
          init(false)
    {
        if (uniqueId > 0 && !viewIds.contains(uniqueId)) {
            s_maxViewId = uniqueId;
            viewId = uniqueId;
        }

        if (viewId == 0) {
            // we didn't get a sane value assigned to us, so lets
            // grab the next available id
            viewId = ++s_maxViewId;
        }
        viewIds.insert(viewId);
    }

    ~ViewPrivate()
    {
    }

    void privateInit()
    {
        q->setContainment(containment);
        init = true;
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
        if (q->transform().isIdentity()) { //we're not zoomed out
            q->setSceneRect(containment->sceneBoundingRect());
        } else {
            //kDebug() << "trying to show the containment nicely";
            q->ensureVisible(containment->sceneBoundingRect());
            //q->centerOn(containment);
        }
        emit q->sceneRectChanged();
    }

    void containmentDestroyed()
    {
        containment = 0;
        emit q->lostContainment();
    }

    void containmentScreenChanged(int wasScreen, int newScreen, Plasma::Containment *containment)
    {
        Q_UNUSED(wasScreen)
        lastScreen = newScreen;
        lastDesktop = containment->desktop();
    }

    void initGraphicsView()
    {
        q->setFrameShape(QFrame::NoFrame);
        q->setAttribute(Qt::WA_TranslucentBackground);
        q->setAutoFillBackground(true);
        q->setDragMode(QGraphicsView::NoDrag);
        //setCacheMode(QGraphicsView::CacheBackground);
        q->setInteractive(true);
        q->setAcceptDrops(true);
        q->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        q->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        q->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    static int s_maxViewId;
    //ugly but the only reliable way to do collision detection of ids
    static QSet<int> viewIds;

    Plasma::View *q;
    Plasma::Containment *containment;
    int viewId;
    int lastScreen;
    int lastDesktop;
    bool drawWallpaper : 1;
    bool trackChanges : 1;
    bool init : 1;
};

int ViewPrivate::s_maxViewId(0);
QSet<int> ViewPrivate::viewIds;

View::View(Containment *containment, QWidget *parent)
    : QGraphicsView(parent),
      d(new ViewPrivate(this, 0))
{
    d->initGraphicsView();

    if (containment) {
        setScene(containment->scene());
        d->containment = containment;
        QTimer::singleShot(0, this, SLOT(privateInit()));
    }
}

View::View(Containment *containment, int viewId, QWidget *parent)
    : QGraphicsView(parent),
      d(new ViewPrivate(this, viewId))
{
    d->initGraphicsView();

    if (containment) {
        setScene(containment->scene());
        d->containment = containment;
        QTimer::singleShot(0, this, SLOT(privateInit()));
    }
}

View::~View()
{
    delete d;
    // FIXME FIX a focus crash but i wasn't able to reproduce in a simple test case for Qt guys
    //       NB: this is also done in Corona
    clearFocus();
}

void View::setScreen(int screen, int desktop)
{
    if (screen < 0) {
        return;
    }

    // handle desktop views
    // -1 == All desktops
    if (desktop < -1 || desktop > KWindowSystem::numberOfDesktops() - 1) {
        desktop = -1;
    }

    d->lastScreen = screen;
    d->lastDesktop = desktop;

    // handle views that are working with panel containment types
    if (d->containment &&
        (d->containment->type() == Containment::PanelContainment ||
         d->containment->type() == Containment::CustomPanelContainment)) {
        d->containment->setScreen(screen, desktop);
        return;
    }

    Plasma::Corona *corona = qobject_cast<Plasma::Corona*>(scene());
    if (corona) {
        Containment *containment = corona->containmentForScreen(screen, desktop);

        if (containment) {
            d->containment = 0; //so that we don't end up on the old containment's screen
            setContainment(containment);
        }
    }
}

int View::screen() const
{
    return d->lastScreen;
}

int View::desktop() const
{
    if (d->containment) {
        return d->containment->desktop();
    }

    return d->lastDesktop;
}

int View::effectiveDesktop() const
{
    int desk = desktop();
    return desk > -1 ? desk : KWindowSystem::currentDesktop();
}

void View::setContainment(Plasma::Containment *containment)
{
    if (d->init && containment == d->containment) {
        return;
    }

    if (d->containment) {
        disconnect(d->containment, SIGNAL(destroyed(QObject*)), this, SLOT(containmentDestroyed()));
        disconnect(d->containment, SIGNAL(geometryChanged()), this, SLOT(updateSceneRect()));
        disconnect(d->containment, SIGNAL(screenChanged(int, int, Plasma::Containment *)), this, SLOT(containmentScreenChanged(int, int, Plasma::Containment *)));
        d->containment->removeAssociatedWidget(this);
    }

    if (!containment) {
        d->containment = 0;
        return;
    }

    Containment *oldContainment = d->containment;

    int screen = -1;
    int desktop = -1;
    if (oldContainment) {
        if (oldContainment != containment) {
            screen = d->containment->screen();
            desktop = d->containment->desktop();
        }
    } else {
        setScene(containment->scene());
    }

    d->containment = containment;

    //add keyboard-shortcut actions
    d->containment->addAssociatedWidget(this);

    int otherScreen = containment->screen();
    int otherDesktop = containment->desktop();

    if (screen > -1) {
        d->lastScreen = screen;
        d->lastDesktop = desktop;
        //kDebug() << "set screen from setContainment due to old containment";
        oldContainment->setScreen(-1, -1);
        containment->setScreen(-1, -1);
        containment->setScreen(screen, desktop);
    } else {
        d->lastScreen = otherScreen;
        d->lastDesktop = otherDesktop;
    }

    if (oldContainment && oldContainment != d->containment && otherScreen > -1) {
        // assign the old containment the old screen/desktop
        //kDebug() << "set screen from setContainment" << screen << otherScreen << desktop << otherDesktop;
        oldContainment->setScreen(otherScreen, otherDesktop);
    }


    /*
    if (oldContainment) {
        kDebug() << "old" << (QObject*)oldContainment << screen << oldContainment->screen()
                 << "new" << (QObject*)containment << otherScreen << containment->screen();
    }
    */

    d->updateSceneRect();
    connect(containment, SIGNAL(destroyed(QObject*)), this, SLOT(containmentDestroyed()));
    connect(containment, SIGNAL(geometryChanged()), this, SLOT(updateSceneRect()));
    connect(containment, SIGNAL(screenChanged(int, int, Plasma::Containment *)), this, SLOT(containmentScreenChanged(int, int, Plasma::Containment *)));
}

Containment *View::containment() const
{
    return d->containment;
}

Containment *View::swapContainment(const QString &name, const QVariantList &args)
{
    return swapContainment(d->containment, name, args);
}

Containment *View::swapContainment(Plasma::Containment *existing, const QString &name, const QVariantList &args)
{
    if (!existing) {
        return 0;
    }

    Containment *old = existing;
    Plasma::Corona *corona = old->corona();
    Plasma::Containment *c = corona->addContainment(name, args);
    if (c) {
        KConfigGroup oldConfig = old->config();
        KConfigGroup newConfig = c->config();

        // ensure that the old containments configuration is up to date
        old->save(oldConfig);

        // Copy configuration to new containment
        oldConfig.copyTo(&newConfig);

        if (old == d->containment) {
            // set our containment to the new one, if the the old containment was us
            setContainment(c);
        }

        // load the configuration of the old containment into the new one
        c->restore(newConfig);
        foreach (Applet *applet, c->applets()) {
            applet->init();
            // We have to flush the applet constraints manually
            applet->flushPendingConstraintsEvents();
        }

        // destroy the old one
        old->destroy(false);

        // and now save the config
        c->save(newConfig);
        corona->requestConfigSync();

        return c;
    }

    return old;
}

KConfigGroup View::config() const
{
    KConfigGroup views(KGlobal::config(), "PlasmaViews");
    return KConfigGroup(&views, QString::number(d->viewId));
}

void View::configNeedsSaving() const
{
    Plasma::Corona *corona = qobject_cast<Plasma::Corona*>(scene());
    if (corona) {
        corona->requestConfigSync();
    } else {
        KGlobal::config()->sync();
    }
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

