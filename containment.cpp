/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
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

#include "containment.h"

#include <QAction>
#include <QDesktopWidget>
#include <QFile>
#include <QGraphicsSceneContextMenuEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include <KApplication>
#include <KAuthorized>
#include <KIcon>
#include <KMenu>
#include <KRun>

#include "workspace/kworkspace.h"

#include "corona.h"
#include "karambamanager.h"
#include "phase.h"
#include "svg.h"

#include "widgets/freelayout.h"
#include "widgets/boxlayout.h"

#include "krunner_interface.h"
#include "ksmserver_interface.h"
#include "screensaver_interface.h"

namespace Plasma
{

class Containment::Private
{
public:
    Private()
        : formFactor(Planar),
          location(Floating),
          layout(0),
          background(0),
          bitmapBackground(0),
          engineExplorerAction(0),
          runCommandAction(0),
          screen(-1),
          immutable(false)
    {
    }

    ~Private()
    {
        qDeleteAll(applets);
        applets.clear();
        delete layout;
        delete bitmapBackground;
    }

    FormFactor formFactor;
    Location location;
    Layout* layout;
    Applet::List applets;
    Plasma::Svg *background;
    QPixmap* bitmapBackground;
    QString wallpaperPath;
    QAction *engineExplorerAction;
    QAction *runCommandAction;
    QAction *lockAction;
    QAction *logoutAction;
    QSize size;
    int screen;
    bool immutable;
};

Containment::Containment(QGraphicsItem* parent,
                         const QString& serviceId,
                         uint containmentId)
    : Applet(parent, serviceId, containmentId),
      d(new Private)
{
}

Containment::Containment(QObject* parent, const QVariantList& args)
    : Applet(parent, args),
      d(new Private)
{
}

Containment::~Containment()
{
    delete d;
}

void Containment::init()
{
//    setCachePaintMode(NoCacheMode);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    KConfigGroup config(KGlobal::config(), "General");
    d->wallpaperPath = config.readEntry("wallpaper", QString());

    //kDebug() << "wallpaperPath is" << d->wallpaperPath << QFile::exists(d->wallpaperPath);
    if (d->wallpaperPath.isEmpty() ||
        !QFile::exists(d->wallpaperPath)) {
        //kDebug() << "SVG wallpaper!";
        d->background = new Plasma::Svg("widgets/wallpaper", this);
    }
}

void Containment::initConstraints(KConfigGroup* group)
{
    //kDebug() << "initConstraints" << group->group();
    setScreen(group->readEntry("screen", -1));
    setFormFactor((Plasma::FormFactor)group->readEntry("formfactor", (int)Plasma::Planar));
    setLocation((Plasma::Location)group->readEntry("location", (int)Plasma::Desktop));
}

void Containment::saveConstraints(KConfigGroup* group) const
{
    group->writeEntry("screen", d->screen);
    group->writeEntry("formfactor", (int)d->formFactor);
    group->writeEntry("location", (int)d->location);
}

void Containment::paintInterface(QPainter *painter,
                                 const QStyleOptionGraphicsItem *option,
                                 const QRect& contentsRect)
{
    //TODO: we should have a way to do this outside of the paint event!
    if (d->background) {
        d->background->resize(contentsRect.size());
    } else if (!d->wallpaperPath.isEmpty()) {
        if (!d->bitmapBackground || !(d->bitmapBackground->size() == contentsRect.size())) {
            delete d->bitmapBackground;
            d->bitmapBackground = new QPixmap(d->wallpaperPath);
            (*d->bitmapBackground) = d->bitmapBackground->scaled(contentsRect.size());
        }
    } else {
        // got nothing to paint!
        painter->drawRect(contentsRect);
        return;
    }

    // draw the background untransformed (saves lots of per-pixel-math)
    painter->save();
    painter->resetTransform();

    // blit the background (saves all the per-pixel-products that blending does)
    painter->setCompositionMode(QPainter::CompositionMode_Source);

    if (d->background) {
        // Plasma::Svg doesn't support drawing only part of the image (it only
        // supports drawing the whole image to a rect), so we blit to 0,0-w,h
        d->background->paint(painter, 0, 0);
    } else if (d->bitmapBackground) {
        // for pixmaps we draw only the exposed part (untransformed since the
        // bitmapBackground already has the size of the viewport)
        painter->drawPixmap(option->exposedRect, *d->bitmapBackground, option->exposedRect);
    }

    // restore transformation and composition mode
    painter->restore();
}

void Containment::launchExplorer()
{
    KRun::run("plasmaengineexplorer", KUrl::List(), 0);
}

void Containment::runCommand()
{
    if (!KAuthorized::authorizeKAction("run_command")) {
        return;
    }

    QString interface("org.kde.krunner");
    org::kde::krunner::Interface krunner(interface, "/Interface",
                                         QDBusConnection::sessionBus());
    if (krunner.isValid()) {
        krunner.display();
    }
}

void Containment::lockScreen()
{
    if (!KAuthorized::authorizeKAction("lock_screen")) {
        return;
    }

    QString interface("org.freedesktop.ScreenSaver");
    org::freedesktop::ScreenSaver screensaver(interface, "/ScreenSaver",
                                              QDBusConnection::sessionBus());
    if (screensaver.isValid()) {
        screensaver.Lock();
    }
}

void Containment::logout()
{
    if (!KAuthorized::authorizeKAction("logout")) {
        return;
    }

    QString interface("org.kde.ksmserver");
    org::kde::KSMServerInterface smserver(interface, "/KSMServer",
                                          QDBusConnection::sessionBus());
    if (smserver.isValid()) {
        smserver.logout(KWorkSpace::ShutdownConfirmDefault,
                        KWorkSpace::ShutdownTypeDefault,
                        KWorkSpace::ShutdownModeDefault);
    }
}

QSizeF Containment::contentSizeHint() const
{
    return d->size;
}

QList<QAction*> Containment::contextActions()
{
    //FIXME: several items here ... probably all junior jobs =)
    //  - engineExplorerAction is going to go away, so the !d->engineExplorerAction below needs to
    //    go
    //  - pretty up the menu with separators
    //  - should we offer "Switch User" here?

    if (!d->engineExplorerAction) {
        d->engineExplorerAction = new QAction(i18n("Engine Explorer"), this);
        connect(d->engineExplorerAction, SIGNAL(triggered(bool)), this, SLOT(launchExplorer()));

        d->runCommandAction = new QAction(i18n("Run Command..."), this);
        connect(d->runCommandAction, SIGNAL(triggered(bool)), this, SLOT(runCommand()));

        d->lockAction = new QAction(i18n("Lock Screen"), this);
        d->lockAction->setIcon(KIcon("system-lock-screen"));
        connect(d->lockAction, SIGNAL(triggered(bool)), this, SLOT(lockScreen()));

        d->logoutAction = new QAction(i18n("Logout"), this);
        d->logoutAction->setIcon(KIcon("system-log-out"));
        connect(d->logoutAction, SIGNAL(triggered(bool)), this, SLOT(logout()));
    }

    QList<QAction*> actions;

    actions.append(d->engineExplorerAction);

    if (KAuthorized::authorizeKAction("run_command")) {
        actions.append(d->runCommandAction);
    }

    if (KAuthorized::authorizeKAction("lock_screen")) {
        actions.append(d->lockAction);
    }

    if (KAuthorized::authorizeKAction("logout")) {
        actions.append(d->logoutAction);
    }

    return actions;
}

void Containment::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    kDebug() << "let's see if we manage to get a context menu here, huh";
    if (!scene() || !KAuthorized::authorizeKAction("desktop_contextmenu")) {
        QGraphicsItem::contextMenuEvent(event);
        return;
    }

    QPointF point = event->scenePos();
    /*
    * example for displaying the SuperKaramba context menu
    QGraphicsItem *item = itemAt(point);
    if(item) {
    QObject *object = dynamic_cast<QObject*>(item->parentItem());
    if(object && object->objectName().startsWith("karamba")) {
    QContextMenuEvent event(QContextMenuEvent::Mouse, point);
    contextMenuEvent(&event);
    return;
}
}
    */
    QGraphicsItem* item = scene()->itemAt(point);
    if (item == this) {
        item = 0;
    }

    Applet* applet = 0;

    while (item) {
        applet = qgraphicsitem_cast<Applet*>(item);
        if (applet) {
            break;
        }

        item = item->parentItem();
    }

    KMenu desktopMenu;
    //kDebug() << "context menu event " << immutable;
    if (!applet) {
        if (!scene() || static_cast<Corona*>(scene())->isImmutable()) {
            QGraphicsItem::contextMenuEvent(event);
            return;
        }

        //FIXME: change this to show this only in debug mode (or not at all?)
        //       before final release
        QList<QAction*> actions = contextActions();
        foreach(QAction* action, actions) {
            desktopMenu.addAction(action);
        }
    } else if (applet->isImmutable()) {
        QGraphicsItem::contextMenuEvent(event);
        return;
    } else {
        bool hasEntries = false;
        if (applet->hasConfigurationInterface()) {
            QAction* configureApplet = new QAction(i18n("%1 Settings...", applet->name()), &desktopMenu);
            connect(configureApplet, SIGNAL(triggered(bool)),
                    applet, SLOT(showConfigurationInterface()));
            desktopMenu.addAction(configureApplet);
            hasEntries = true;
        }

        if (scene() && !static_cast<Corona*>(scene())->isImmutable()) {
            QAction* closeApplet = new QAction(i18n("Remove this %1", applet->name()), &desktopMenu);
            connect(closeApplet, SIGNAL(triggered(bool)),
                    applet, SLOT(destroy()));
            desktopMenu.addAction(closeApplet);
            hasEntries = true;
        }

        QList<QAction*> actions = applet->contextActions();
        if (!actions.isEmpty()) {
            desktopMenu.addSeparator();
            foreach(QAction* action, actions) {
                desktopMenu.addAction(action);
                hasEntries = true;
            }
        }

        if (!hasEntries) {
            QGraphicsItem::contextMenuEvent(event);
            return;
        }
    }

    event->accept();
    desktopMenu.exec(point.toPoint());
}

void Containment::setFormFactor(FormFactor formFactor)
{
    if (d->formFactor == formFactor) {
        return;
    }

//kDebug() << "switching FF to " << formFactor;
    d->formFactor = formFactor;
    delete d->layout;
    d->layout = 0;

    switch (d->formFactor) {
        case Planar:
            d->layout = new FreeLayout;
            break;
        case Horizontal:
            d->layout = new BoxLayout(BoxLayout::LeftToRight);
            break;
        case Vertical:
            d->layout = new BoxLayout(BoxLayout::TopToBottom);
            break;
        case MediaCenter:
            //FIXME: need a layout type here!
            break;
        default:
            kDebug() << "This can't be happening! Or... can it? ;)";
            break;
    }

    foreach (Applet* applet, d->applets) {
        applet->updateConstraints();
    }
}

FormFactor Containment::formFactor() const
{
    return d->formFactor;
}

void Containment::setLocation(Location location)
{
    if (d->location == location) {
        return;
    }

    d->location = location;

    foreach (Applet* applet, d->applets) {
        applet->updateConstraints();
    }
}

Location Containment::location() const
{
    return d->location;
}

void Containment::clearApplets()
{
    qDeleteAll(d->applets);
    d->applets.clear();
}

Applet* Containment::addApplet(const QString& name, const QVariantList& args, uint id, const QRectF& geometry, bool delayInit)
{
    Applet* applet = Applet::loadApplet(name, id, args);
    if (!applet) {
        kDebug() << "Applet" << name << "could not be loaded.";
        applet = new Applet;
    }

    applet->setParentItem(this);
    //kDebug() << "adding applet" << applet->name() << "with a default geometry of" << geometry << geometry.isValid();
    if (geometry.isValid()) {
        applet->setGeometry(geometry);
    } else if (geometry.x() != -1 && geometry.y() != -1) {
        // yes, this means we can't have items start -1, -1
        applet->setGeometry(QRectF(geometry.topLeft() - QPointF(applet->sizeHint().width()/2,
                                                                applet->sizeHint().height()/2),
                                   applet->sizeHint()));
    } else {
        //TODO: Make sure new applets don't overlap with existing ones
        // Center exactly:
        QSizeF size = applet->sizeHint();
        qreal appletWidth = size.width();
        qreal appletHeight = size.height();
        qreal width = this->geometry().width();
        qreal height = this->geometry().height();
        //kDebug() << "measuring geometry with" << appletWidth << appletHeight << width << height;
        applet->setGeometry(QRectF(QPointF((width / 2) - (appletWidth / 2), (height / 2) - (appletHeight / 2)), size));
    }

    if (d->layout) {
        d->layout->addItem(applet);
    }

    applet->updateConstraints();

    if (!delayInit) {
        applet->init();
    }

    d->applets << applet;
    connect(applet, SIGNAL(destroyed(QObject*)),
            this, SLOT(appletDestroyed(QObject*)));
    Phase::self()->animateItem(applet, Phase::Appear);

    return applet;
}

void Containment::addKaramba(const KUrl& path)
{
    QGraphicsItemGroup* karamba = KarambaManager::loadKaramba(path, scene());
    if (karamba) {
        karamba->setParentItem(this);
        Phase::self()->animateItem(karamba, Phase::Appear);
    } else {
        kDebug() << "Karamba " << path << " could not be loaded.";
    }
}

void Containment::appletDestroyed(QObject* object)
{
    // we do a static_cast here since it really isn't an Applet by this
    // point anymore since we are in the qobject dtor. we don't actually
    // try and do anything with it, we just need the value of the pointer
    // so this unsafe looking code is actually just fine.
    Applet* applet = static_cast<Plasma::Applet*>(object);
    int index = d->applets.indexOf(applet);

    if (index > -1) {
        d->applets.removeAt(index);
    }
}

Applet::List Containment::applets() const
{
    return d->applets;
}

void Containment::setScreen(int screen)
{
    //kDebug() << "setting screen to" << screen;
    QDesktopWidget desktop;
    int numScreens = desktop.numScreens();
    if (screen < -1 || screen > numScreens - 1) {
        screen = -1;
    }

    if (screen > -1) {
        setGeometry(desktop.screenGeometry(screen));
        //kDebug() << "setting geometry to" << desktop.screenGeometry(screen);
    }

    d->screen = screen;
}

int Containment::screen() const
{
    return d->screen;
}

}

#include "containment.moc"
