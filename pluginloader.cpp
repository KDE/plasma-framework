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
#include <kstandarddirs.h>
#include <kplugininfo.h>

#include "applet.h"
#include "abstractrunner.h"
#include "containment.h"
#include "packagestructure.h"
#include "popupapplet.h"
#include "private/applet_p.h"
#include "private/extenderapplet_p.h"
#include "private/service_p.h" // for NullService
#include "private/storage_p.h"

namespace Plasma {

static PluginLoader* s_pluginLoader = 0;

class PluginLoaderPrivate
{

};

PluginLoader::PluginLoader()
    : d(0)
{
}

PluginLoader::~PluginLoader()
{
    delete d;
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
            if (offer->serviceTypes().contains("Plasma/Containment")) {
                return new Containment(0, allArgs);
            } else if (offer->serviceTypes().contains("Plasma/PopupApplet")) {
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
        kWarning() << "Could not load applet" << name << "! reason given:" << error;
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

AbstractRunner *PluginLoader::loadRunner(const QString &name)
{
    // FIXME: RunnerManager is all wrapped around runner loading; that should be sorted out
    // and the actual plugin loading added here
    return internalLoadRunner(name);
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
    } else if (name == "org.kde.servicestorage") {
        return new Storage(parent);
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

KPluginInfo::List PluginLoader::listAppletInfo(const QString &category, const QString &parentApp)
{
    KPluginInfo::List list;

    if (parentApp.isEmpty() || parentApp == KGlobal::mainComponent().componentName()) {
        list = internalAppletInfo(category);
    }

    QString constraint = AppletPrivate::parentAppConstraint(parentApp);

    //note: constraint guaranteed non-empty from here down
    if (category.isEmpty()) { //use all but the excluded categories
        KConfigGroup group(KGlobal::config(), "General");
        QStringList excluded = group.readEntry("ExcludeCategories", QStringList());
        foreach (const QString &category, excluded) {
            constraint.append(" and [X-KDE-PluginInfo-Category] != '").append(category).append("'");
        }
    } else { //specific category (this could be an excluded one - is that bad?)
        constraint.append(" and ").append("[X-KDE-PluginInfo-Category] == '").append(category).append("'");
        if (category == "Miscellaneous") {
            constraint.append(" or (not exist [X-KDE-PluginInfo-Category] or [X-KDE-PluginInfo-Category] == '')");
        }
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);

    //now we have to do some manual filtering because the constraint can't handle everything
    AppletPrivate::filterOffers(offers);

    //kDebug() << "Applet::listAppletInfo constraint was '" << constraint
    //         << "' which got us " << offers.count() << " matches";
    return KPluginInfo::fromServices(offers);
}

KPluginInfo::List PluginLoader::listDataEngineInfo(const QString &parentApp)
{
    KPluginInfo::List list;

    if (parentApp.isEmpty() || parentApp == KGlobal::mainComponent().componentName()) {
        list = internalDataEngineInfo();
    }

    QString constraint;
    if (parentApp.isEmpty()) {
        constraint.append("not exist [X-KDE-ParentApp]");
    } else {
        constraint.append("[X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/DataEngine", constraint);
    return list + KPluginInfo::fromServices(offers);
}

KPluginInfo::List PluginLoader::listRunnerInfo(const QString &parentApp)
{
    KPluginInfo::List list;

    if (parentApp.isEmpty() || parentApp == KGlobal::mainComponent().componentName()) {
        list = internalRunnerInfo();
    }

    QString constraint;
    if (parentApp.isEmpty()) {
        constraint.append("not exist [X-KDE-ParentApp]");
    } else {
        constraint.append("[X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Runner", constraint);
    return list + KPluginInfo::fromServices(offers);
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

AbstractRunner* PluginLoader::internalLoadRunner(const QString &name)
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

KPluginInfo::List PluginLoader::internalAppletInfo(const QString &category) const
{
    Q_UNUSED(category)
    return KPluginInfo::List();
}

KPluginInfo::List PluginLoader::internalDataEngineInfo() const
{
    return KPluginInfo::List();
}

KPluginInfo::List PluginLoader::internalRunnerInfo() const
{
    return KPluginInfo::List();
}

KPluginInfo::List PluginLoader::internalServiceInfo() const
{
    return KPluginInfo::List();
}

static KPluginInfo::List standardInternalInfo(const QString &type, const QString &category = QString())
{
    QStringList files = KGlobal::dirs()->findAllResources("appdata",
                                                          "plasma/internal/" + type + "/*.desktop",
                                                          KStandardDirs::NoDuplicates);

    KPluginInfo::List allInfo = KPluginInfo::fromFiles(files);

    if (category.isEmpty() || allInfo.isEmpty()) {
        return allInfo;
    }

    KPluginInfo::List matchingInfo;
    foreach (const KPluginInfo &info, allInfo) {
        if (info.category().compare(category, Qt::CaseInsensitive) == 0) {
            matchingInfo << info;
        }
    }

    return matchingInfo;
}

KPluginInfo::List PluginLoader::standardInternalAppletInfo(const QString &category) const
{
    return standardInternalInfo("applets", category);
}

KPluginInfo::List PluginLoader::standardInternalDataEngineInfo() const
{
    return standardInternalInfo("dataengines");
}

KPluginInfo::List PluginLoader::standardInternalRunnerInfo() const
{
    return standardInternalInfo("runners");
}

KPluginInfo::List PluginLoader::standardInternalServiceInfo() const
{
    return standardInternalInfo("services");
}

} // Plasma Namespace

