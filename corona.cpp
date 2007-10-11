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
#include "karambamanager.h"
#include "phase.h"
#include "widgets/freelayout.h"
#include "widgets/boxlayout.h"
#include "widgets/icon.h"

using namespace Plasma;

namespace Plasma
{

class Corona::Private
{
public:
    Private()
        : immutable(false),
          mimetype("text/x-plasmoidservicename")
    {
    }

    ~Private()
    {
    }

    bool immutable;
    QString mimetype;
    QList<Containment*> containments;
};

Corona::Corona(QObject *parent)
    : QGraphicsScene(parent),
      d(new Private)
{
    //d->init(this);
    //setViewport(new QGLWidget(QGLFormat(QGL::StencilBuffer | QGL::AlphaChannel)));
}

Corona::Corona(const QRectF & sceneRect, QObject * parent )
    : QGraphicsScene(sceneRect, parent),
      d(new Private)
{
    //d->init(this);
    //setViewport(new QGLWidget(QGLFormat(QGL::StencilBuffer | QGL::AlphaChannel)));
}

Corona::Corona(qreal x, qreal y, qreal width, qreal height, QObject * parent)
    : QGraphicsScene(x, y, width, height, parent),
      d(new Private)
{
    //d->init(this);
    //setViewport(new QGLWidget(QGLFormat(QGL::StencilBuffer | QGL::AlphaChannel)));
}

Corona::~Corona()
{
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
    foreach (const QString& group, cg.groupList()) {
        cg.deleteGroup(group);
    }

    QStringList containmentIds;
    foreach (const Containment *containment, d->containments) {
        QString cid = QString::number(containment->id());
        KConfigGroup containmentConfig(&cg, cid.append("-containment"));
        containment->saveConstraints(&containmentConfig);
        containment->save(&containmentConfig);
        foreach (const Applet* applet, containment->applets()) {
            KConfigGroup appletConfig(&cg, QString::number(applet->id()).append("-applet"));
            applet->save(&appletConfig);
        }
    }
}

void Corona::saveApplets() const
{
    saveApplets("plasma-appletsrc");
}

void Corona::loadApplets(const QString& configname)
{
    clearApplets();

    KConfig config(configname, KConfig::SimpleConfig);

    QList<KConfigGroup> applets;
    QHash<int, Containment*> containments;
    foreach (const QString& group, config.groupList()) {
        KConfigGroup appletConfig(&config, group);
        if (group.contains("containment")) {
            int cid = group.left(group.indexOf('-')).toUInt();
            Containment *c = addContainment(appletConfig.readEntry("plugin", QString()), QVariantList(),
                                            cid, true);
            if (c) {
                containments.insert(c->id(), c);
                c->initConstraints(&appletConfig);
                kDebug() << "Containment" << c->id() << "geometry is" << c->geometry();
            }
        } else {
            // it's an applet, let's grab the containment association
            kDebug() << "insert multi" << group;
            applets.append(appletConfig);
        }
    }

    //int maxContainment = containments.size();
    //kDebug() << "number of applets?" << applets.count();
    foreach (KConfigGroup cg, applets) {
        int cid = cg.readEntry("containment", 0);
        kDebug() << "trying to load applet " << cg.name() << " in containment " << cid;

        Containment* c = containments.value(cid, 0);

        if (!c) {
            kDebug() << "couldn't find containment " << cid << ", skipping this applet";
            continue;
        }

        kDebug() << "creating applet " << cg.name() << "in containment" << cid;
        int appId = cg.name().left(cg.name().indexOf('-')).toUInt();
        c->addApplet(cg.readEntry("plugin", QString()), QVariantList(),
                     appId, cg.readEntry("geometry", QRectF()), true);
    }

    foreach (Containment* c, containments) {
        QString cid = QString::number(c->id());
        KConfigGroup containmentConfig(&config, cid.append("-containment"));
        c->setImmutable(containmentConfig.isImmutable());
    }

    if (d->containments.count() < 1) {
        loadDefaultSetup();
    } else {
        foreach (Containment* containment, d->containments) {
            containment->init();

            foreach(Applet* applet, containment->applets()) {
                applet->init();
            }
        }
    }

    setImmutable(config.isImmutable());
}

void Corona::loadApplets()
{
    loadApplets("plasma-appletsrc");
}

void Corona::loadDefaultSetup()
{
    //FIXME: implement support for system-wide defaults
    QDesktopWidget desktop;
    int numScreens = desktop.numScreens();
    // create a containment for each screen
    for (int i = 0; i < numScreens; ++i) {
        Containment* c = addContainment("desktop");
        c->setScreen(i);
        c->setFormFactor(Plasma::Planar);
    }

    // make a panel at the bottom
    Containment* panel = addContainment("panel", (QVariantList() << (int)Plasma::BottomEdge));

    return;
    // some default applets to get a usable UI
    QList<Plasma::Applet*> applets;
    Plasma::Applet *tasksApplet = panel->addApplet("tasks");
    Plasma::Applet *systemTrayApplet = panel->addApplet("systemtray");
    Plasma::Applet *clockApplet = panel->addApplet("digital-clock");

    applets << tasksApplet << systemTrayApplet << clockApplet;

}

Containment* Corona::containmentForScreen(int screen) const
{
    foreach (Containment* containment, d->containments) {
        if (containment->screen() == screen) {
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

Containment* Corona::addContainment(const QString& name, const QVariantList& args, uint id, bool delayedInit)
{
    QString pluginName = name;
    Containment* containment = 0;
    Applet* applet = 0;

    kDebug() << "Loading" << name << args << id;

    if (pluginName.isEmpty()) {
        // default to the desktop containment
        pluginName = "desktop";
    }

    applet = Applet::loadApplet(pluginName, id, args);
    containment = dynamic_cast<Containment*>(applet);

    if (!containment) {
        kDebug() << "loading failed.";
        delete applet; // in case we got a non-Containment from Applet::loadApplet
        containment = new Containment;
        containment->setFailedToLaunch(false); // we want to provide something and don't care about the failure to launch
    }

    addItem(containment);

    if (!delayedInit) {
        containment->init();
    }

    d->containments.append(containment);
    connect(containment, SIGNAL(destroyed(QObject*)),
            this, SLOT(containmentDestroyed(QObject*)));

    return containment;
}

Applet* Corona::addApplet(const QString& name, const QVariantList& args, uint id, const QRectF& geometry)
{
    if (d->containments.size() < 1) {
        kDebug() << "No containments to add an applet to!" << endl;
        //FIXME create a containment if one doesn't exist ... ?
        return 0;
    }

    return d->containments[0]->addApplet(name, args, id, geometry);
}

void Corona::addKaramba(const KUrl& path)
{
    //FIXME: i think this is slightly broken now that we have containments?
    //       it should go into a containment...
    QGraphicsItemGroup* karamba = KarambaManager::loadKaramba(path, this);
    if (karamba) {
        addItem(karamba);
        Phase::self()->animateItem(karamba, Phase::Appear);
    } else {
        kDebug() << "Karamba " << path << " could not be loaded.";
    }
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

bool Corona::isImmutable() const
{
    return d->immutable;
}

void Corona::setImmutable(bool immutable)
{
    if (d->immutable == immutable) {
        return;
    }

    d->immutable = immutable;
    foreach (QGraphicsItem* item, items()) {
        item->setFlag(QGraphicsItem::ItemIsMovable, immutable);
    }
}

} // namespace Plasma

#include "corona.moc"

