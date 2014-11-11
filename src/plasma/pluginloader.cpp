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

#include <QStandardPaths>

#include <QDebug>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kplugintrader.h>
#include <kpackage/packagetrader.h>

#include "config-plasma.h"

#if !PLASMA_NO_KIO
#include <kio/job.h>
#endif

#include "applet.h"
#include "containment.h"
#include "containmentactions.h"
#include "dataengine.h"
#include "package.h"
#include "private/applet_p.h"
#include "private/packages_p.h"
#include "private/service_p.h" // for NullService
#include "private/storage_p.h"
#include "private/package_p.h"
#include "private/packagestructure_p.h"

namespace Plasma
{

static PluginLoader *s_pluginLoader = 0;

class PluginLoaderPrivate
{
public:
    PluginLoaderPrivate()
        : isDefaultLoader(false),
          dataEnginePluginDir("plasma/dataengine"),
          packageStructurePluginDir("plasma/packagestructure"),
          packageRE("[^a-zA-Z0-9\\-_]")
    {
    }

    static QSet<QString> knownCategories();
    static QString parentAppConstraint(const QString &parentApp = QString());

    static QSet<QString> s_customCategories;
    QHash<QString, QWeakPointer<PackageStructure> > structures;
    bool isDefaultLoader;
    QString dataEnginePluginDir;
    QString packageStructurePluginDir;
    QRegExp packageRE;
};

QSet<QString> PluginLoaderPrivate::s_customCategories;

QSet<QString> PluginLoaderPrivate::knownCategories()
{
    // this is to trick the tranlsation tools into making the correct
    // strings for translation
    QSet<QString> categories = s_customCategories;
    categories << QString(I18N_NOOP("Accessibility")).toLower()
               << QString(I18N_NOOP("Application Launchers")).toLower()
               << QString(I18N_NOOP("Astronomy")).toLower()
               << QString(I18N_NOOP("Date and Time")).toLower()
               << QString(I18N_NOOP("Development Tools")).toLower()
               << QString(I18N_NOOP("Education")).toLower()
               << QString(I18N_NOOP("Environment and Weather")).toLower()
               << QString(I18N_NOOP("Examples")).toLower()
               << QString(I18N_NOOP("File System")).toLower()
               << QString(I18N_NOOP("Fun and Games")).toLower()
               << QString(I18N_NOOP("Graphics")).toLower()
               << QString(I18N_NOOP("Language")).toLower()
               << QString(I18N_NOOP("Mapping")).toLower()
               << QString(I18N_NOOP("Miscellaneous")).toLower()
               << QString(I18N_NOOP("Multimedia")).toLower()
               << QString(I18N_NOOP("Online Services")).toLower()
               << QString(I18N_NOOP("Productivity")).toLower()
               << QString(I18N_NOOP("System Information")).toLower()
               << QString(I18N_NOOP("Utilities")).toLower()
               << QString(I18N_NOOP("Windows and Tasks")).toLower();
    return categories;
}

QString PluginLoaderPrivate::parentAppConstraint(const QString &parentApp)
{
    if (parentApp.isEmpty()) {
        QCoreApplication *app = QCoreApplication::instance();
        if (!app) {
            return QString();
        }

        return QString("((not exist [X-KDE-ParentApp] or [X-KDE-ParentApp] == '') or [X-KDE-ParentApp] == '%1')")
               .arg(app->applicationName());
    }

    return QString("[X-KDE-ParentApp] == '%1'").arg(parentApp);
}

PluginLoader::PluginLoader()
    : d(new PluginLoaderPrivate)
{
}

PluginLoader::~PluginLoader()
{
    typedef QWeakPointer<PackageStructure> pswp;
    foreach (pswp wp, d->structures) {
        delete wp.data();
    }
    delete d;
}

void PluginLoader::setPluginLoader(PluginLoader *loader)
{
    if (!s_pluginLoader) {
        s_pluginLoader = loader;
    } else {
#ifndef NDEBUG
        // qDebug() << "Cannot set pluginLoader, already set!" << s_pluginLoader;
#endif
    }
}

PluginLoader *PluginLoader::self()
{
    if (!s_pluginLoader) {
        // we have been called before any PluginLoader was set, so just use the default
        // implementation. this prevents plugins from nefariously injecting their own
        // plugin loader if the app doesn't
        s_pluginLoader = new PluginLoader;
        s_pluginLoader->d->isDefaultLoader = true;
    }

    return s_pluginLoader;
}

Applet *PluginLoader::loadApplet(const QString &name, uint appletId, const QVariantList &args)
{
    if (name.isEmpty()) {
        return 0;
    }

    Applet *applet = d->isDefaultLoader ? 0 : internalLoadApplet(name, appletId, args);
    if (applet) {
        return applet;
    }

    // the application-specific appletLoader failed to create an applet, here we try with our own logic.
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

    if (offers.isEmpty()) {
#ifndef NDEBUG
        // qDebug() << "offers is empty for " << name;
#endif
        return 0;
    }

#ifndef NDEBUG
    if (offers.count() > 1) {
        // qDebug() << "hey! we got more than one! let's blindly take the first one";
    }
#endif

    KService::Ptr offer = offers.first();

    if (appletId == 0) {
        appletId = ++AppletPrivate::s_maxAppletId;
    }

    QVariantList allArgs;
    allArgs << offer->storageId() << appletId << args;

    if (!offer->property("X-Plasma-API").toString().isEmpty()) {
#ifndef NDEBUG
        // qDebug() << "we have a script using the"
        //         << offer->property("X-Plasma-API").toString() << "API";
#endif
        if (isContainment) {
            return new Containment(0, allArgs);
        } else {
            if (offer->serviceTypes().contains("Plasma/Containment")) {
                return new Containment(0, allArgs);
            } else {
                return new Applet(0, allArgs);
            }
        }
    }

    KPluginLoader plugin(*offer);

    if (!Plasma::isPluginVersionCompatible(plugin.pluginVersion())) {
        return 0;
    }

    QString error;
    applet = offer->createInstance<Plasma::Applet>(0, allArgs, &error);

    if (!applet) {
        qWarning() << "Could not load applet" << name << "! reason given:" << error;
    }

    return applet;
}

DataEngine *PluginLoader::loadDataEngine(const QString &name)
{
    DataEngine *engine = d->isDefaultLoader ? 0 : internalLoadDataEngine(name);
    if (engine) {
        return engine;
    }

    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(name);

    // First check with KServiceTypeTrader as that is where scripted engines will be
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/DataEngine", constraint);

    if (!offers.isEmpty()) {
        const QString api = offers.first()->property("X-Plasma-API").toString();
        if (api.isEmpty()) {
            // it is a C++ plugin, fetch it with KPluginTrader
            engine = KPluginTrader::createInstanceFromQuery<Plasma::DataEngine>(d->dataEnginePluginDir, "Plasma/DataEngine", constraint, 0);
        } else {
            // it is a scripted plugin, load it via a package
            engine = new DataEngine(KPluginInfo(offers.first()), 0);
        }
    }

    return engine;
}

QStringList PluginLoader::listAllEngines(const QString &parentApp)
{
    QString constraint;

    if (parentApp.isEmpty()) {
        constraint.append("(not exist [X-KDE-ParentApp] or [X-KDE-ParentApp] == '')");
    } else {
        constraint.append("[X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/DataEngine", constraint);

    QStringList engines;
    foreach (const KService::Ptr &service, offers) {
        QString name = service->property("X-KDE-PluginInfo-Name").toString();
        if (!name.isEmpty()) {
            engines.append(name);
        }
    }

    return engines;
}

KPluginInfo::List PluginLoader::listEngineInfo(const QString &parentApp)
{
    return PluginLoader::self()->listDataEngineInfo(parentApp);
}

KPluginInfo::List PluginLoader::listEngineInfoByCategory(const QString &category, const QString &parentApp)
{
    QString constraint = QString("[X-KDE-PluginInfo-Category] == '%1'").arg(category);

    if (parentApp.isEmpty()) {
        constraint.append(" and not exist [X-KDE-ParentApp]");
    } else {
        constraint.append(" and [X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/DataEngine", constraint);
    return KPluginInfo::fromServices(offers);
}

Service *PluginLoader::loadService(const QString &name, const QVariantList &args, QObject *parent)
{
    Service *service = d->isDefaultLoader ? 0 : internalLoadService(name, args, parent);
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
#ifndef NDEBUG
        // qDebug() << "offers is empty for " << name;
#endif
        return new NullService(name, parent);
    }

    KService::Ptr offer = offers.first();
    QString error;

    if (Plasma::isPluginVersionCompatible(KPluginLoader(*offer).pluginVersion())) {
        service = offer->createInstance<Plasma::Service>(parent, args, &error);
    }

    if (!service) {
#ifndef NDEBUG
        // qDebug() << "Couldn't load Service \"" << name << "\"! reason given: " << error;
#endif
        return new NullService(name, parent);
    }

    if (service->name().isEmpty()) {
        service->setName(name);
    }

    return service;
}

ContainmentActions *PluginLoader::loadContainmentActions(Containment *parent, const QString &name, const QVariantList &args)
{
    if (name.isEmpty()) {
        return 0;
    }

    ContainmentActions *actions = d->isDefaultLoader ? 0 : internalLoadContainmentActions(parent, name, args);
    if (actions) {
        return actions;
    }

    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(name);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ContainmentActions", constraint);

    if (offers.isEmpty()) {
#ifndef NDEBUG
        qDebug() << "offers is empty for " << name;
#endif
        return 0;
    }

    KService::Ptr offer = offers.first();
    KPluginLoader plugin(*offer);

    if (!Plasma::isPluginVersionCompatible(plugin.pluginVersion())) {
        return 0;
    }

    QVariantList allArgs;
    allArgs << offer->storageId() << args;
    QString error;
    actions = offer->createInstance<Plasma::ContainmentActions>(parent, allArgs, &error);

    if (!actions) {
#ifndef NDEBUG
        // qDebug() << "Couldn't load containmentActions \"" << name << "\"! reason given: " << error;
#endif
    }

    return actions;
}

Package PluginLoader::loadPackage(const QString &packageFormat, const QString &specialization)
{
    if (!d->isDefaultLoader) {
        Package p = internalLoadPackage(packageFormat, specialization);
        if (p.hasValidStructure()) {
            return p;
        }
    }

    if (packageFormat.isEmpty()) {
        return Package();
    }

    const QString hashkey = packageFormat + '%' + specialization;
    PackageStructure *structure = d->structures.value(hashkey).data();
    KPackage::PackageStructure *internalStructure = 0;
    if (structure) {
        return Package(structure);
    }

    if (packageFormat.startsWith("Plasma")) {
        if (packageFormat.endsWith("/Applet")) {
            internalStructure = new PlasmoidPackage();
        } else if (packageFormat.endsWith("/DataEngine")) {
            internalStructure = new DataEnginePackage();
        } else if (packageFormat.endsWith("/Theme")) {
            internalStructure = new ThemePackage();
        } else if (packageFormat.endsWith("/ContainmentActions")) {
            internalStructure = new ContainmentActionsPackage();
        } else if (packageFormat.endsWith("/Generic")) {
            internalStructure = new GenericPackage();
        }

        if (internalStructure) {
            structure = new PackageStructure();
            structure->d->internalStructure = internalStructure;
            d->structures.insert(hashkey, structure);
            return Package(structure);
        }
    }

    internalStructure = KPackage::PackageTrader::self()->loadPackageStructure(packageFormat);
    structure = new PackageStructure();
    structure->d->internalStructure = internalStructure;
    if (structure) {
        d->structures.insert(hashkey, structure);
        return Package(structure);
    }

#ifndef NDEBUG
        // qDebug() << "Couldn't load Package for" << packageFormat << "! reason given: " << error;
#endif

    return Package();
}

KPluginInfo::List PluginLoader::listAppletInfo(const QString &category, const QString &parentApp)
{
    KPluginInfo::List list;

    if (!d->isDefaultLoader && (parentApp.isEmpty() || parentApp == QCoreApplication::instance()->applicationName())) {
        list = internalAppletInfo(category);
    }

    QString constraint = PluginLoaderPrivate::parentAppConstraint(parentApp);

    //note: constraint guaranteed non-empty from here down
    if (category.isEmpty()) { //use all but the excluded categories
        KConfigGroup group(KSharedConfig::openConfig(), "General");
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

    //qDebug() << "Applet::listAppletInfo constraint was '" << constraint
    //         << "' which got us " << offers.count() << " matches";
    return KPluginInfo::fromServices(offers);
}

KPluginInfo::List PluginLoader::listAppletInfoForMimeType(const QString &mimeType)
{
    QString constraint = PluginLoaderPrivate::parentAppConstraint();
    constraint.append(QString(" and '%1' in [X-Plasma-DropMimeTypes]").arg(mimeType));
    //qDebug() << "listAppletInfoForMimetype with" << mimeType << constraint;
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);
    return KPluginInfo::fromServices(offers);
}

KPluginInfo::List PluginLoader::listAppletInfoForUrl(const QUrl &url)
{
    QString constraint = PluginLoaderPrivate::parentAppConstraint();
    constraint.append(" and exist [X-Plasma-DropUrlPatterns]");
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);

    KPluginInfo::List allApplets = KPluginInfo::fromServices(offers);
    KPluginInfo::List filtered;
    foreach (const KPluginInfo &info, allApplets) {
        QStringList urlPatterns = info.property("X-Plasma-DropUrlPatterns").toStringList();
        foreach (const QString &glob, urlPatterns) {
            QRegExp rx(glob);
            rx.setPatternSyntax(QRegExp::Wildcard);
            if (rx.exactMatch(url.toString())) {
#ifndef NDEBUG
                // qDebug() << info.name() << "matches" << glob << url;
#endif
                filtered << info;
            }
        }
    }

    return filtered;
}

QStringList PluginLoader::listAppletCategories(const QString &parentApp, bool visibleOnly)
{
    QString constraint = PluginLoaderPrivate::parentAppConstraint(parentApp);
    constraint.append(" and exist [X-KDE-PluginInfo-Category]");

    KConfigGroup group(KSharedConfig::openConfig(), "General");
    const QStringList excluded = group.readEntry("ExcludeCategories", QStringList());
    foreach (const QString &category, excluded) {
        constraint.append(" and [X-KDE-PluginInfo-Category] != '").append(category).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);

    QStringList categories;
    QSet<QString> known = PluginLoaderPrivate::knownCategories();
    foreach (const KService::Ptr &applet, offers) {
        QString appletCategory = applet->property("X-KDE-PluginInfo-Category").toString();
        if (visibleOnly && applet->noDisplay()) {
            // we don't want to show the hidden category
            continue;
        }

        //qDebug() << "   and we have " << appletCategory;
        if (!appletCategory.isEmpty() && !known.contains(appletCategory.toLower())) {
#ifndef NDEBUG
            // qDebug() << "Unknown category: " << applet->name() << "says it is in the"
            //         << appletCategory << "category which is unknown to us";
#endif
            appletCategory.clear();
        }

        if (appletCategory.isEmpty()) {
            if (!categories.contains(i18nc("misc category", "Miscellaneous"))) {
                categories << i18nc("misc category", "Miscellaneous");
            }
        } else  if (!categories.contains(appletCategory)) {
            categories << appletCategory;
        }
    }

    categories.sort();
    return categories;
}

void PluginLoader::setCustomAppletCategories(const QStringList &categories)
{
    PluginLoaderPrivate::s_customCategories = QSet<QString>::fromList(categories);
}

QStringList PluginLoader::customAppletCategories() const
{
    return PluginLoaderPrivate::s_customCategories.toList();
}

QString PluginLoader::appletCategory(const QString &appletName)
{
    if (appletName.isEmpty()) {
        return QString();
    }

    const QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(appletName);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);

    if (offers.isEmpty()) {
        return QString();
    }

    return offers.first()->property("X-KDE-PluginInfo-Category").toString();
}

KPluginInfo::List PluginLoader::listContainments(const QString &category,
        const QString &parentApp)
{
    return listContainmentsOfType(QString(), category, parentApp);
}

KPluginInfo::List PluginLoader::listContainmentsOfType(const QString &type,
        const QString &category,
        const QString &parentApp)
{
    QString constraint;

    if (parentApp.isEmpty()) {
        constraint.append("(not exist [X-KDE-ParentApp] or [X-KDE-ParentApp] == '')");
    } else {
        constraint.append("[X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    if (!type.isEmpty()) {
        if (!constraint.isEmpty()) {
            constraint.append(" and (");
        }

        //constraint.append("'").append(type).append("' == [X-Plasma-ContainmentType]");
        if (type == "Desktop") {
            constraint += "not exist [X-Plasma-ContainmentType] or ";
        }
        constraint += "[X-Plasma-ContainmentType] == '" + type + "')";
        //by default containments are Desktop, so is not mandatory to specify it
    }

    if (!category.isEmpty()) {
        if (!constraint.isEmpty()) {
            constraint.append(" and ");
        }

        constraint.append("[X-KDE-PluginInfo-Category] == '").append(category).append("'");
        if (category == "Miscellaneous") {
            constraint.append(" or (not exist [X-KDE-PluginInfo-Category] or [X-KDE-PluginInfo-Category] == '')");
        }
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Containment", constraint);
    // qDebug() << "constraint was" << constraint << "which got us" << offers.count() << "matches";
    return KPluginInfo::fromServices(offers);
}

KPluginInfo::List PluginLoader::listContainmentsForMimeType(const QString &mimeType)
{
    const QString constraint = QString("'%1' in [X-Plasma-DropMimeTypes]").arg(mimeType);
    //qDebug() << mimeType << constraint;
    const KService::List offers = KServiceTypeTrader::self()->query("Plasma/Containment", constraint);
    return KPluginInfo::fromServices(offers);
}

QStringList PluginLoader::listContainmentTypes()
{
    KPluginInfo::List containmentInfos = listContainments();
    QSet<QString> types;

    foreach (const KPluginInfo &containmentInfo, containmentInfos) {
        QStringList theseTypes = containmentInfo.service()->property("X-Plasma-ContainmentType").toStringList();
        foreach (const QString &type, theseTypes) {
            types.insert(type);
        }
    }

    return types.toList();
}

KPluginInfo::List PluginLoader::listDataEngineInfo(const QString &parentApp)
{
    KPluginInfo::List list;

    if (!d->isDefaultLoader && (parentApp.isEmpty() || parentApp == QCoreApplication::instance()->applicationName())) {
        list = internalDataEngineInfo();
    }

    QString constraint;
    if (parentApp.isEmpty()) {
        constraint.append("not exist [X-KDE-ParentApp]");
    } else {
        constraint.append("[X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    list.append(KPluginTrader::self()->query(d->dataEnginePluginDir, "Plasma/DataEngine", constraint));
    return list;
}

KPluginInfo::List PluginLoader::listContainmentActionsInfo(const QString &parentApp)
{
    KPluginInfo::List list;

    if (!d->isDefaultLoader && (parentApp.isEmpty() || parentApp == QCoreApplication::instance()->applicationName())) {
        list = internalContainmentActionsInfo();
    }

    QString constraint;
    if (parentApp.isEmpty()) {
        constraint.append("not exist [X-KDE-ParentApp]");
    } else {
        constraint.append("[X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ContainmentActions", constraint);
    return KPluginInfo::fromServices(offers);
}

Applet *PluginLoader::internalLoadApplet(const QString &name, uint appletId, const QVariantList &args)
{
    Q_UNUSED(name)
    Q_UNUSED(appletId)
    Q_UNUSED(args)
    return 0;
}

DataEngine *PluginLoader::internalLoadDataEngine(const QString &name)
{
    Q_UNUSED(name)
    return 0;
}

ContainmentActions *PluginLoader::internalLoadContainmentActions(Containment *containment, const QString &name, const QVariantList &args)
{
    Q_UNUSED(containment)
    Q_UNUSED(name)
    Q_UNUSED(args)
    return 0;
}

Service *PluginLoader::internalLoadService(const QString &name, const QVariantList &args, QObject *parent)
{
    Q_UNUSED(name)
    Q_UNUSED(args)
    Q_UNUSED(parent)
    return 0;
}

Package PluginLoader::internalLoadPackage(const QString &name, const QString &specialization)
{
    Q_UNUSED(name);
    Q_UNUSED(specialization);
    return Package();
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

KPluginInfo::List PluginLoader::internalServiceInfo() const
{
    return KPluginInfo::List();
}

KPluginInfo::List PluginLoader::internalContainmentActionsInfo() const
{
    return KPluginInfo::List();
}

static KPluginInfo::List standardInternalInfo(const QString &type, const QString &category = QString())
{
    QStringList files = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                        PLASMA_RELATIVE_DATA_INSTALL_DIR "/internal/" + type + "/*.desktop",
                        QStandardPaths::LocateFile);

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

KPluginInfo::List PluginLoader::standardInternalServiceInfo() const
{
    return standardInternalInfo("services");
}

} // Plasma Namespace

