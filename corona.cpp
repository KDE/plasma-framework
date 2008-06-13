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
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QPainter>
#include <QTimer>

#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KMimeType>

#include "containment.h"

using namespace Plasma;

namespace Plasma
{

// constant controlling how long between requesting a configuration sync
// and one happening should occur. currently 2 minutes.
const int CONFIG_SYNC_TIMEOUT = 120000;

class Corona::Private
{
public:
    Private(Corona *corona)
        : q(corona),
          immutability(Mutable),
          mimetype("text/x-plasmoidservicename"),
          config(0)
    {
        if (KGlobal::hasMainComponent()) {
            configName = KGlobal::mainComponent().componentName() + "-appletsrc";
        } else {
            configName = "plasma-appletsrc";
        }
    }

    ~Private()
    {
        qDeleteAll(containments);
    }

    void init()
    {
        configSyncTimer.setSingleShot(true);
        connect(&configSyncTimer, SIGNAL(timeout()), q, SLOT(syncConfig()));
    }

    void saveLayout(KSharedConfigPtr cg) const
    {
        KConfigGroup containmentsGroup(cg, "Containments");
        foreach (const Containment *containment, containments) {
            QString cid = QString::number(containment->id());
            KConfigGroup containmentConfig(&containmentsGroup, cid);
            containment->save(containmentConfig);
        }
    }

    void updateContainmentImmutability()
    {
        foreach (Containment *c, containments) {
            // we need to tell each containment that immutability has been altered
            c->updateConstraints(ImmutableConstraint);
        }
    }

    void containmentDestroyed(QObject* obj)
    {
        // we do a static_cast here since it really isn't an Containment by this
        // point anymore since we are in the qobject dtor. we don't actually
        // try and do anything with it, we just need the value of the pointer
        // so this unsafe looking code is actually just fine.
        Containment* containment = static_cast<Plasma::Containment*>(obj);
        int index = containments.indexOf(containment);

        if (index > -1) {
            containments.removeAt(index);
        }
    }

    void syncConfig()
    {
        q->config()->sync();
    }

    Containment* addContainment(const QString& name, const QVariantList& args, uint id, bool delayedInit)
    {
        QString pluginName = name;
        Containment* containment = 0;
        Applet* applet = 0;

        //kDebug() << "Loading" << name << args << id;

        if (pluginName.isEmpty()) {
            // default to the desktop containment
            pluginName = "desktop";
        } else if (pluginName != "null") {
            applet = Applet::load(pluginName, id, args);
            containment = dynamic_cast<Containment*>(applet);
        }

        if (!containment) {
            kDebug() << "loading of containment" << name << "failed.";

            // in case we got a non-Containment from Applet::loadApplet or a null containment was requested
            delete applet;
            containment = new Containment(0, 0, id);

            // we want to provide something and don't care about the failure to launch
            containment->setFailedToLaunch(false);
            containment->setFormFactor(Plasma::Planar);
        }

        containment->setIsContainment(true);

        if (!delayedInit) {
            q->addItem(containment);
            containment->init();
            containment->updateConstraints(Plasma::StartupCompletedConstraint);
        }

        containments.append(containment);
        connect(containment, SIGNAL(destroyed(QObject*)), q, SLOT(containmentDestroyed(QObject*)));
        connect(containment, SIGNAL(configNeedsSaving()), q, SLOT(scheduleConfigSync()));
        connect(containment, SIGNAL(releaseVisualFocus()), q, SIGNAL(releaseVisualFocus()));
        connect(containment, SIGNAL(screenChanged(int,int,Plasma::Containment*)),
                q, SIGNAL(screenOwnerChanged(int,int,Plasma::Containment*)));

        if (!delayedInit) {
            emit q->containmentAdded(containment);
        }

        return containment;
    }

    /**
     * Called when there have been changes made to configuration that should be saved
     * to disk at the next convenient moment
     */
    void scheduleConfigSync();

    Corona *q;
    ImmutabilityType immutability;
    QString mimetype;
    QString configName;
    KSharedConfigPtr config;
    QTimer configSyncTimer;
    QList<Containment*> containments;
};

Corona::Corona(QObject *parent)
    : QGraphicsScene(parent),
      d(new Private(this))
{
    d->init();
    //setViewport(new QGLWidget(QGLFormat(QGL::StencilBuffer | QGL::AlphaChannel)));
}

Corona::~Corona()
{
    KConfigGroup cg(config(), "General");

    // we call the dptr member directly for locked since isImmutable()
    // also checks kiosk and parent containers
    cg.writeEntry("immutability", (int)d->immutability);
    delete d;
}

void Corona::setAppletMimeType(const QString& type)
{
    d->mimetype = type;
}

QString Corona::appletMimeType()
{
    return d->mimetype;
}

void Corona::saveLayout(const QString &configName) const
{
    KSharedConfigPtr c;

    if (configName.isEmpty() || configName == d->configName) {
        c = config();
    } else {
        c = KSharedConfig::openConfig(configName);
    }

    d->saveLayout(c);
}

void Corona::Private::scheduleConfigSync()
{
    // TODO: should we check into our immutability before doing this?

    //NOTE: this is a pretty simplistic model: we simply save no more than CONFIG_SYNC_TIMEOUT
    //      after the first time this is called. not much of a heuristic for save points, but
    //      it should at least compress these activities a bit and provide a way for applet
    //      authors to ween themselves from the sync() disease. A more interesting/dynamic
    //      algorithm for determining when to actually sync() to disk might be better, though.
    if (!configSyncTimer.isActive()) {
        configSyncTimer.start(CONFIG_SYNC_TIMEOUT);
    }
}

void Corona::initializeLayout(const QString &configName)
{
    clearContainments();
    loadLayout(configName);

    if (d->containments.isEmpty()) {
        loadDefaultLayout();
        if (!d->containments.isEmpty()) {
            d->scheduleConfigSync();
        }
    }

    if (config()->isImmutable()) {
        d->updateContainmentImmutability();
    }

    KConfigGroup coronaConfig(config(), "General");
    setImmutability((ImmutabilityType)coronaConfig.readEntry("immutability", (int)Mutable));
}

void Corona::loadLayout(const QString& configName)
{
    KSharedConfigPtr c;

    if (configName.isEmpty() || configName == d->configName) {
        c = config();
    } else {
        c = KSharedConfig::openConfig(configName);
    }

    KConfigGroup containments(config(), "Containments");

    foreach (const QString& group, containments.groupList()) {
        KConfigGroup containmentConfig(&containments, group);

        if (containmentConfig.entryMap().isEmpty()) {
            continue;
        }

        int cid = group.toUInt();
        //kDebug() << "got a containment in the config, trying to make a" << containmentConfig.readEntry("plugin", QString()) << "from" << group;
        Containment *c = d->addContainment(containmentConfig.readEntry("plugin", QString()), QVariantList(),
                                           cid, true);
        if (!c) {
            continue;
        }

        addItem(c);
        c->init();
        c->restore(containmentConfig);
    }

    foreach (Containment* containment, d->containments) {
        QString cid = QString::number(containment->id());
        KConfigGroup containmentConfig(&containments, cid);

        foreach(Applet* applet, containment->applets()) {
            applet->init();
        }

        containment->updateConstraints(Plasma::StartupCompletedConstraint);
        containment->flushPendingConstraintsEvents();
        emit containmentAdded(containment);
    }
}

Containment* Corona::containmentForScreen(int screen) const
{
    foreach (Containment* containment, d->containments) {
        if (containment->screen() == screen &&
            (containment->containmentType() == Containment::DesktopContainment ||
             containment->containmentType() >= Containment::CustomContainment)) {
            return containment;
        }
    }

    return 0;
}

QList<Containment*> Corona::containments() const
{
    return d->containments;
}

void Corona::clearContainments()
{
    foreach (Containment* containment, d->containments) {
        containment->clearApplets();
    }
}

KSharedConfigPtr Corona::config() const
{
    if (!d->config) {
        d->config = KSharedConfig::openConfig(d->configName);
    }

    return d->config;
}

Containment* Corona::addContainment(const QString& name, const QVariantList& args)
{
    return d->addContainment(name, args, 0, false);
}

void Corona::destroyContainment(Containment *c)
{
    if (!d->containments.contains(c)) {
        return;
    }

    //don't remove a desktop that's in use
    //FIXME allow removal of containments for screens that don't currently exist
    if (c->containmentType() != Containment::PanelContainment && c->screen() != -1) {
        return;
    }

    d->containments.removeAll(c);
    removeItem(c);
    c->config().deleteGroup();
    c->deleteLater();
}

void Corona::loadDefaultLayout()
{
}

void Corona::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsScene::dragEnterEvent(event);
}

void Corona::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsScene::dragLeaveEvent(event);
}

void Corona::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsScene::dragMoveEvent(event);
}

ImmutabilityType Corona::immutability() const
{
    return d->immutability;
}

void Corona::setImmutability(const ImmutabilityType immutable)
{
    if (d->immutability == immutable ||
        d->immutability == SystemImmutable) {
        return;
    }
    
    kDebug() << "setting immutability to" << immutable;
    d->immutability = immutable;
    d->updateContainmentImmutability();
}

} // namespace Plasma

#include "corona.moc"

