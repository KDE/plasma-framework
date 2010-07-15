/*
 *   Copyright 2010 Ryan Rix <ry@n.rix.si>
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

#include "pluginloader.h"

#include <kdebug.h>
#include <kglobal.h>
#include <kservice.h>
#include <kservicetypetrader.h>

#include "applet.h"
#include "containment.h"
#include "packagestructure.h"
#include "popupapplet.h"
#include "private/applet_p.h"
#include "private/extenderapplet_p.h"
#include "private/service_p.h" // for NullService

namespace Plasma {

static PluginLoader* s_pluginLoader = 0;

PluginLoader::PluginLoader()
    : d(0)
{
}

PluginLoader::~PluginLoader()
{
    //delete d;
}

void PluginLoader::setPluginLoader(PluginLoader* loader)
{
    if (!s_pluginLoader) {
        s_pluginLoader = loader;
    } else {
        kDebug() << "Cannot set pluginLoader, already set!" << s_pluginLoader;
    }
}

PluginLoader *PluginLoader::pluginLoader()
{
    if (!s_pluginLoader) {
        // we have been called before any PluginLoader was set, so just use the default
        // implementation. this prevents plugins from nefariously injecting their own
        // plugin loader if the app doesn't
        s_pluginLoader = new PluginLoader;
    }

    return s_pluginLoader;
}

Applet *PluginLoader::loadApplet(const QString &name, uint appletId, const QVariantList &args)
{ 
    // the application-specific appletLoader failed to create an applet, here we try with our own logic.
    if (name.isEmpty()) {
        return 0;
    }

    Applet *applet = internalLoadApplet(name, appletId, args);
    if (applet) {
        return applet;
    }

    const QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(name);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);

    bool isContainment = false;
    if (offers.isEmpty()) {
        //TODO: what would be -really- cool is offer to try and download the applet
        //      from the network at this point
        offers = KServiceTypeTrader::self()->query("Plasma/Containment", constraint);
        if (offers.count() > 0) {
            isContainment = true;
        }
    }

    /* if (offers.count() > 1) {
        kDebug() << "hey! we got more than one! let's blindly take the first one";
    } */

    AppletPrivate::filterOffers(offers);
    if (offers.isEmpty()) {
        kDebug() << "offers is empty for " << name;
        return 0;
    }

    KService::Ptr offer = offers.first();
 
    if (appletId == 0) {
        appletId = ++AppletPrivate::s_maxAppletId;
    }

    QVariantList allArgs;
    allArgs << offer->storageId() << appletId << args;

    if (!offer->property("X-Plasma-API").toString().isEmpty()) {
        kDebug() << "we have a script using the"
                 << offer->property("X-Plasma-API").toString() << "API";
        if (isContainment) {
            return new Containment(0, allArgs);
        } else {
            if (offer->serviceTypes().contains("Plasma/PopupApplet")) {
                return new PopupApplet(0, allArgs);
            } else {
                return new Applet(0, allArgs);
            }
        }
    }

    KPluginLoader plugin(*offer);

    if (!Plasma::isPluginVersionCompatible(plugin.pluginVersion()) &&
        (name != "internal:extender")) {
        return 0;
    }


    QString error;
    if (name == "internal:extender") {
        applet = new ExtenderApplet(0, allArgs);
    } else {
        applet = offer->createInstance<Plasma::Applet>(0, allArgs, &error);
    }

    if (!applet) {
        kDebug() << "Couldn't load applet \"" << name << "\"! reason given: " << error;
    }

    return applet;
}

DataEngine *PluginLoader::loadDataEngine(const QString &name)
{ 
    DataEngine *engine = internalLoadDataEngine(name);
    if (engine) {
        return engine;
    }

    // load the engine, add it to the engines
    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(name);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/DataEngine",
                                                              constraint);
    QString error;

    if (offers.isEmpty()) {
        kDebug() << "offers are empty for " << name << " with constraint " << constraint;
    } else {
        QVariantList allArgs;
        allArgs << offers.first()->storageId();
        QString api = offers.first()->property("X-Plasma-API").toString();
        if (api.isEmpty()) {
            if (offers.first()) {
                KPluginLoader plugin(*offers.first());
                if (Plasma::isPluginVersionCompatible(plugin.pluginVersion())) {
                    engine = offers.first()->createInstance<Plasma::DataEngine>(0, allArgs, &error);
                }
            }
        } else {
            engine = new DataEngine(0, offers.first());
        }
    }

    if (!engine) {
        kDebug() << "Couldn't load engine \"" << name << "\". Error given: " << error;
    }

    return engine;
}

Service *PluginLoader::loadService(const QString &name, const QVariantList &args, QObject *parent)
{ 
    Service *service = internalLoadService(name, args, parent);
    if (service) {
        return service;
    }

    //TODO: scripting API support
    if (name.isEmpty()) {
        return new NullService(QString(), parent);
    }

    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(name);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Service", constraint);

    if (offers.isEmpty()) {
        kDebug() << "offers is empty for " << name;
        return new NullService(name, parent);
    }

    KService::Ptr offer = offers.first();
    QString error;

    if (Plasma::isPluginVersionCompatible(KPluginLoader(*offer).pluginVersion())) {
        service = offer->createInstance<Plasma::Service>(parent, args, &error);
    }

    if (!service) {
        kDebug() << "Couldn't load Service \"" << name << "\"! reason given: " << error;
        return new NullService(name, parent);
    }

    if (service->name().isEmpty()) {
        service->setName(name);
    }

    return service;
}

Applet* PluginLoader::internalLoadApplet(const QString &name, uint appletId, const QVariantList &args)
{ 
    Q_UNUSED(name)
    Q_UNUSED(appletId)
    Q_UNUSED(args)
    return 0;
}

DataEngine* PluginLoader::internalLoadDataEngine(const QString &name)
{
    Q_UNUSED(name)
    return 0;
}

Service* PluginLoader::internalLoadService(const QString &name, const QVariantList &args, QObject *parent)
{ 
    Q_UNUSED(name)
    Q_UNUSED(args)
    Q_UNUSED(parent)
    return 0;
}

} // Plasma Namespace

