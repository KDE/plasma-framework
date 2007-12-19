/*
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Riccardo Iaconelli <riccardo@kde.org>
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

#include "corona.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QGraphicsView>
#include <QStringList>

#include <KDebug>
#include <KLocale>
#include <KMimeType>
#include <KWindowSystem>

#include "containment.h"
#include "dataengine.h"
#include "phase.h"
#include "layouts/layout.h"
#include "widgets/icon.h"

using namespace Plasma;

namespace Plasma
{

class Corona::Private
{
public:
    Private()
        : immutable(false),
          kioskImmutable(false),
          mimetype("text/x-plasmoidservicename"),
          configName("plasma-appletsrc"),
          config(0)
    {
    }

    ~Private()
    {
        qDeleteAll(containments);
    }

    void init(Corona* q)
    {
        QObject::connect(QApplication::desktop(), SIGNAL(resized(int)), q, SLOT(screenResized(int)));
    }

    bool immutable;
    bool kioskImmutable;
    QString mimetype;
    QString configName;
    KSharedConfigPtr config;
    QList<Containment*> containments;
};

Corona::Corona(QObject *parent)
    : QGraphicsScene(parent),
      d(new Private)
{
    d->init(this);
    //setViewport(new QGLWidget(QGLFormat(QGL::StencilBuffer | QGL::AlphaChannel)));
}

Corona::Corona(const QRectF & sceneRect, QObject * parent )
    : QGraphicsScene(sceneRect, parent),
      d(new Private)
{
    d->init(this);
    //setViewport(new QGLWidget(QGLFormat(QGL::StencilBuffer | QGL::AlphaChannel)));
}

Corona::Corona(qreal x, qreal y, qreal width, qreal height, QObject * parent)
    : QGraphicsScene(x, y, width, height, parent),
      d(new Private)
{
    d->init(this);
    //setViewport(new QGLWidget(QGLFormat(QGL::StencilBuffer | QGL::AlphaChannel)));
}

Corona::~Corona()
{
    KConfigGroup cg(config(), "General");

    // we call the dptr member directly for locked since isImmutable()
    // also checks kiosk and parent containers
    cg.writeEntry("locked", d->immutable);
    delete d;
}

QRectF Corona::maxSizeHint() const
{
    //FIXME: this is a bit of a naive implementation, do you think? =)
    //       we should factor in how much space we actually have left!
    return sceneRect();
}

void Corona::setAppletMimeType(const QString& type)
{
    d->mimetype = type;
}

QString Corona::appletMimeType()
{
    return d->mimetype;
}

void Corona::saveApplets(const QString &config) const
{
    KConfig cg(config);
    d->configName = config;

    KConfigGroup containments(&cg, "Containments");
    foreach (const Containment *containment, d->containments) {
        QString cid = QString::number(containment->id());
        KConfigGroup containmentConfig(&containments, cid);
        containment->saveConstraints(&containmentConfig);
        containment->save(&containmentConfig);
        KConfigGroup applets(&containmentConfig, "Applets");
        foreach (const Applet* applet, containment->applets()) {
            KConfigGroup appletConfig(&applets, QString::number(applet->id()));
            applet->save(&appletConfig);
        }
    }
}

void Corona::saveApplets() const
{
    saveApplets(d->configName);
}

void Corona::loadApplets(const QString& configName)
{
    clearApplets();
    d->configName = configName;

    KConfigGroup containments(config(), "Containments");

    foreach (const QString& group, containments.groupList()) {
        KConfigGroup containmentConfig(&containments, group);

        if (containmentConfig.entryMap().isEmpty()) {
            continue;
        }

        int cid = group.toUInt();
        //kDebug() << "got a containment in the config, trying to make a" << containmentConfig.readEntry("plugin", QString()) << "from" << group;
        Containment *c = addContainment(containmentConfig.readEntry("plugin", QString()), QVariantList(),
                                        cid, true);
        if (!c) {
            continue;
        }

        addItem(c);
        c->init();
        c->loadConstraints(&containmentConfig);
        //kDebug() << "Containment" << c->id() << "geometry is" << c->geometry().toRect() << "config'd with" << appletConfig.name();
        KConfigGroup applets(&containmentConfig, "Applets");

        foreach (const QString &appletGroup, applets.groupList()) {
            //kDebug() << "reading from applet group" << appletGroup;
            int appId = appletGroup.toUInt();
            KConfigGroup appletConfig(&applets, appletGroup);
            //kDebug() << "the name is" << appletConfig.name();
            QString plugin = appletConfig.readEntry("plugin", QString());

            if (plugin.isEmpty()) {
                continue;
            }

            Applet *applet = c->addApplet(plugin, QVariantList(), appId, appletConfig.readEntry("geometry", QRectF()), true);
            Q_UNUSED(applet)
            // FIXME: the transform does not stick; it gets set then almost immediately reset.
            //        find out why and then reenable this
            /*
            QList<qreal> m = cg.readEntry("transform", QList<qreal>());
            if (m.count() == 9) {
                QTransform t(m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8]);
                applet->setTransform(t);
            }
           */
        }
    }

    if (d->containments.count() < 1) {
        loadDefaultSetup();
    } else {
        foreach (Containment* containment, d->containments) {
            QString cid = QString::number(containment->id());
            KConfigGroup containmentConfig(&containments, cid);
            containment->setImmutable(containmentConfig.isImmutable());

            foreach(Applet* applet, containment->applets()) {
                applet->init();
            }

            containment->flushUpdatedConstraints();
        }

        // quick sanity check to ensure we have containments for each screen!
        int numScreens = QApplication::desktop()->numScreens();
        for (int i = 0; i < numScreens; ++i) {
            if (!containmentForScreen(i)) {
                //TODO: should we look for containments that aren't asigned but already exist?
                Containment* c = addContainment("desktop");
                c->setScreen(i);
                c->setFormFactor(Plasma::Planar);
            }
        }
    }

    d->kioskImmutable = config()->isImmutable();
    KConfigGroup coronaConfig(config(), "General");
    setImmutable(coronaConfig.readEntry("locked", false));
}

void Corona::loadApplets()
{
    loadApplets(d->configName);
}

void Corona::loadDefaultSetup()
{
    //FIXME: implement support for system-wide defaults
    QDesktopWidget *desktop = QApplication::desktop();
    int numScreens = desktop->numScreens();
    kDebug() << "number of screens is" << numScreens;
    int topLeftScreen = 0;
    QPoint topLeftCorner = desktop->screenGeometry(0).topLeft();

    // create a containment for each screen
    for (int i = 0; i < numScreens; ++i) {
        QRect g = desktop->screenGeometry(i);
        kDebug() << "     screen " << i << "geometry is" << g;
        Containment* c = addContainment("desktop");
        c->setScreen(i);
        c->setFormFactor(Plasma::Planar);

        if (g.x() <= topLeftCorner.x() && g.y() >= topLeftCorner.y()) {
            topLeftCorner = g.topLeft();
            topLeftScreen = i;
        }
    }

    // make a panel at the bottom
    Containment* panel = addContainment("panel");
    panel->setScreen(topLeftScreen);
    panel->setLocation(Plasma::BottomEdge);

    // some default applets to get a usable UI
    panel->addApplet("launcher");
    panel->addApplet("tasks");
    panel->addApplet("systemtray");
    panel->addApplet("notifier");
    panel->addApplet("digital-clock");

    // trigger an instant layout so we immediately have a proper geometry rather than waiting around
    // for the event loop
    panel->flushUpdatedConstraints();
    if (panel->layout()) {
        panel->layout()->invalidate();
    }

    /*
     * a little snip that adds another panel, this time to the left
     *
    panel = addContainment("panel");
    panel->setScreen(topLeftScreen);
    //TODO: but .. *where* on the left edge?
    panel->setLocation(Plasma::LeftEdge);
    */

    // in case something goes bad during runtime, let's at least save this to disk right away
    config()->sync();
}

Containment* Corona::containmentForScreen(int screen) const
{
    foreach (Containment* containment, d->containments) {
        if (containment->screen() == screen &&
            containment->containmentType() == Containment::DesktopContainment) {
            return containment;
        }
    }

    return 0;
}

QList<Containment*> Corona::containments() const
{
    return d->containments;
}

void Corona::clearApplets()
{
    foreach (Containment* containment, d->containments) {
        containment->clearApplets();
    }
}

KSharedConfigPtr Corona::config()
{
    if (!d->config) {
        d->config = KSharedConfig::openConfig(d->configName);
    }

    return d->config;
}

Containment* Corona::addContainment(const QString& name, const QVariantList& args, uint id, bool delayedInit)
{
    QString pluginName = name;
    Containment* containment = 0;
    Applet* applet = 0;

    //kDebug() << "Loading" << name << args << id;

    if (pluginName.isEmpty()) {
        // default to the desktop containment
        pluginName = "desktop";
    } else if (pluginName != "null") {
        applet = Applet::loadApplet(pluginName, id, args);
        containment = dynamic_cast<Containment*>(applet);
    }

    if (!containment) {
        kDebug() << "loading of containment" << name << "failed.";

        // in case we got a non-Containment from Applet::loadApplet or a null containment was requested
        delete applet;
        containment = new Containment;

        // we want to provide something and don't care about the failure to launch
        containment->setFailedToLaunch(false);
        containment->setFormFactor(Plasma::Planar);
    }

    containment->setIsContainment(true);

    if (!delayedInit) {
        addItem(containment);
        containment->init();
    }

    d->containments.append(containment);
    connect(containment, SIGNAL(destroyed(QObject*)),
            this, SLOT(containmentDestroyed(QObject*)));
    connect(containment, SIGNAL(launchActivated()),
            SIGNAL(launchActivated()));

    return containment;
}

void Corona::destroyContainment(Containment *c)
{
    if (!d->containments.contains(c)) {
        return;
    }

    d->containments.removeAll(c);
    c->config().deleteGroup();
    c->deleteLater();
}

void Corona::dragEnterEvent( QGraphicsSceneDragDropEvent *event)
{
//    kDebug() << "Corona::dragEnterEvent(QGraphicsSceneDragDropEvent* event)";
    if (event->mimeData()->hasFormat(d->mimetype) ||
        KUrl::List::canDecode(event->mimeData())) {
        event->acceptProposedAction();
        //TODO Create the applet, move to mouse position then send the
        //     following event to lock it to the mouse
        //QMouseEvent event(QEvent::MouseButtonPress, event->pos(), Qt::LeftButton, event->mouseButtons(), 0);
        //QApplication::sendEvent(this, &event);
    }

    event->accept();
    //TODO Allow dragging an applet from another Corona into this one while
    //     keeping its settings etc.
}

void Corona::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
   // kDebug() << "Corona::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)";
    //TODO If an established Applet is dragged out of the Corona, remove it and
    //     create a QDrag type thing to keep the Applet's settings

    QGraphicsScene::dragLeaveEvent(event);
}

void Corona::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsScene::dragMoveEvent(event);

    event->accept();
    //kDebug() << "Corona::dragMoveEvent(QDragMoveEvent* event)";
}

void Corona::containmentDestroyed(QObject* obj)
{
    // we do a static_cast here since it really isn't an Containment by this
    // point anymore since we are in the qobject dtor. we don't actually
    // try and do anything with it, we just need the value of the pointer
    // so this unsafe looking code is actually just fine.
    Containment* containment = static_cast<Plasma::Containment*>(obj);
    int index = d->containments.indexOf(containment);

    if (index > -1) {
        d->containments.removeAt(index);
    }
}

void Corona::screenResized(int screen)
{
    bool desktopFound = false;
    foreach (Containment *c, d->containments) {
        if (c->screen() == screen) {
            // trigger a relayout
            c->setScreen(screen);
            desktopFound = desktopFound || c->containmentType() == Containment::DesktopContainment;
        }
    }

    if (desktopFound) {
        return;
    }

    // a new screen appeared. neat.
    // FIXME: apparently QDesktopWidget doesn't do the Right Thing when a new screen is plugged in
    //        at runtime. seems it gets confused and thinks it's all one big screen? need to 
    //        fix this upstream
    Containment* c = addContainment("desktop");
    c->setScreen(screen);
    c->setFormFactor(Plasma::Planar);
    emit newScreen(screen);
}

bool Corona::isImmutable() const
{
    return d->kioskImmutable || d->immutable;
}

bool Corona::isKioskImmutable() const
{
    return d->kioskImmutable;
}

void Corona::setImmutable(bool immutable)
{
    if (d->immutable == immutable) {
        return;
    }

    kDebug() << "setting immutability to" << immutable;
    d->immutable = immutable;
    foreach (Containment *c, d->containments) {
        // we need to tell each containment that immutability has been altered
        // TODO: should we tell the applets too?
        c->updateConstraints(ImmutableConstraint);
    }
}

} // namespace Plasma

#include "corona.moc"

