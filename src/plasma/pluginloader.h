/*
 *   Copyright 2010 by Ryan Rix <ry@n.rix.si>
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

#ifndef PLUGIN_LOADER_H
#define PLUGIN_LOADER_H

#include <plasma/package.h>
#include <plasma/plasma.h>
#include <kplugininfo.h>

namespace Plasma
{

class Applet;
class Containment;
class ContainmentActions;
class DataEngine;
class Service;

class PluginLoaderPrivate;

//TODO:
// * add loadWallpaper
// * add KPluginInfo listing support for Containments (already loaded via the applet loading code)

/**
 * @class PluginLoader plasma/pluginloader.h <Plasma/PluginLoader>
 *
 * This is an abstract base class which defines an interface to which Plasma's
 * Applet Loading logic can communicate with a parent application. The plugin loader
 * must be set before any plugins are loaded, otherwise (for safety reasons), the
 * default PluginLoader implementation will be used. The reimplemented version should
 * not do more than simply returning a loaded plugin. It should not init() it, and it should not
 * hang on to it. The associated methods will be called only when a component of Plasma
 * needs to load a _new_ plugin. (e.g. DataEngine does its own caching).
 *
 * @author Ryan Rix <ry@n.rix.si>
 * @since 4.6
 **/
class PLASMA_EXPORT PluginLoader
{
public:
    /**
     * Load an Applet plugin.
     *
     * @param name the plugin name, as returned by KPluginInfo::pluginName()
     * @param appletId unique ID to assign the applet, or zero to have one
     *        assigned automatically.
     * @param args to send the applet extra arguments
     * @return a pointer to the loaded applet, or 0 on load failure
     **/
    Applet *loadApplet(const QString &name, uint appletId = 0,
                       const QVariantList &args = QVariantList());

    /**
     * Load a dataengine plugin.
     *
     * @param name the name of the engine
     * @return the dataengine that was loaded, or the NullEngine on failure.
     **/
    DataEngine *loadDataEngine(const QString &name);

    /**
     * @return a listing of all known dataengines by name
     *
     * @param parentApp the application to filter dataengines on. Uses the
     *                  X-KDE-ParentApp entry (if any) in the plugin info.
     *                  The default value of QString() will result in a
     *                  list containing only dataengines not specifically
     *                  registered to an application.
     */
    static QStringList listAllEngines(const QString &parentApp = QString());

    /**
     * Returns a list of all known dataengines.
     *
     * @param parentApp the application to filter dataengines on. Uses the
     *                  X-KDE-ParentApp entry (if any) in the plugin info.
     *                  The default value of QString() will result in a
     *                  list containing only dataengines not specifically
     *                  registered to an application.
     * @return list of dataengines
     **/
    static KPluginInfo::List listEngineInfo(const QString &parentApp = QString());

    /**
     * Returns a list of all known dataengines filtering by category.
     *
     * @param category the category to filter dataengines on. Uses the
     *                  X-KDE-PluginInfo-Category entry (if any) in the
     *                  plugin info. The value of QString() will
     *                  result in a list of dataengines with an empty category.
     *
     * @param parentApp the application to filter dataengines on. Uses the
     *                  X-KDE-ParentApp entry (if any) in the plugin info.
     *                  The default value of QString() will result in a
     *                  list containing only dataengines not specifically
     *                  registered to an application.
     * @return list of dataengines
     * @since 4.3
     **/
    static KPluginInfo::List listEngineInfoByCategory(const QString &category, const QString &parentApp = QString());

    /**
     * Load a Service plugin.
     *
     * @param name the plugin name of the service to load
     * @param args a list of arguments to supply to the service plugin when loading it
     * @param parent the parent object, if any, for the service
     *
     * @return a Service object, unlike Plasma::Service::loadService, this can return null.
     **/
    Service *loadService(const QString &name, const QVariantList &args, QObject *parent = 0);

    /**
     * Load a ContainmentActions plugin.
     *
     * Returns a pointer to the containmentactions if successful.
     * The caller takes responsibility for the containmentactions, including
     * deleting it when no longer needed.
     *
     * @param parent the parent containment. @since 4.6 null is allowed.
     * @param name the plugin name, as returned by KPluginInfo::pluginName()
     * @param args to send the containmentactions extra arguments
     * @return a ContaimentActions object
     **/
    ContainmentActions *loadContainmentActions(Containment *parent, const QString &containmentActionsName,
            const QVariantList &args = QVariantList());

    /**
     * Load a Package plugin.
     *
     * @param name the plugin name of the package to load
     * @param specialization used to find script extensions for the given format, e.g. "QML" for "Plasma/Applet"
     *
     * @return a Package object matching name, or an invalid package on failure
     * @deprecated since 5.29 use KPackage::PackageLoader::loadPackage
     **/
#ifndef PLASMA_NO_DEPRECATED
    PLASMA_DEPRECATED Package loadPackage(const QString &packageFormat, const QString &specialization = QString());
#endif

    /**
     * Returns a list of all known applets.
     * This may skip applets based on security settings and ExcludeCategories in the application's config.
     *
     * @param category Only applets matchin this category will be returned.
     *                 Useful in conjunction with knownCategories.
     *                 If "Misc" is passed in, then applets without a
     *                 Categories= entry are also returned.
     *                 If an empty string is passed in, all applets are
     *                 returned.
     * @param parentApp the application to filter applets on. Uses the
     *                  X-KDE-ParentApp entry (if any) in the plugin info.
     *                  The default value of QString() will result in a
     *                  list containing only applets not specifically
     *                  registered to an application.
     * @return list of applets
     *
     * @deprecated use listAppletMetaData. Doesn't support metadata.json packages.
     **/
    PLASMA_DEPRECATED KPluginInfo::List listAppletInfo(const QString &category, const QString &parentApp = QString());

    /**
     * Returns a list of all known applets.
     * This may skip applets based on security settings and ExcludeCategories in the application's config.
     *
     * @param category Only applets matchin this category will be returned.
     *                 Useful in conjunction with knownCategories.
     *                 If "Misc" is passed in, then applets without a
     *                 Categories= entry are also returned.
     *                 If an empty string is passed in, all applets are
     *                 returned.
     * @param parentApp the application to filter applets on. Uses the
     *                  X-KDE-ParentApp entry (if any) in the plugin info.
     *                  The default value of QString() will result in a
     *                  list containing only applets not specifically
     *                  registered to an application.
     * @return list of applets
     *
     * @since 5.28
     **/
    QList<KPluginMetaData> listAppletMetaData(const QString &category, const QString &parentApp = QString());

    /**
     * Returns a list of all known applets associated with a certain mimetype.
     *
     * @return list of applets
     **/
    KPluginInfo::List listAppletInfoForMimeType(const QString &mimetype);

    /**
     * Returns a list of all known applets associated with a certain URL.
     *
     * @return list of applets
     **/
    KPluginInfo::List listAppletInfoForUrl(const QUrl &url);

    /**
     * Returns a list of all the categories used by installed applets.
     *
     * @param parentApp the application to filter applets on. Uses the
     *                  X-KDE-ParentApp entry (if any) in the plugin info.
     *                  The default value of QString() will result in a
     *                  list containing only applets not specifically
     *                  registered to an application.
     * @return list of categories
     * @param visibleOnly true if it should only return applets that are marked as visible
     */
    QStringList listAppletCategories(const QString &parentApp = QString(),
                                     bool visibleOnly = true);

    /**
     * Sets the list of custom categories that are used in addition to the default
     * set of categories known to libplasma for applets.
     * @param categories a list of categories
     * @since 4.3
     */
    void setCustomAppletCategories(const QStringList &categories);

    /**
     * @return the list of custom categories known to libplasma
     * @since 4.3
     */
    QStringList customAppletCategories() const;

    /**
     * Get the category of the given applet
     *
     * @param appletName the name of the applet
     */
    QString appletCategory(const QString &appletName);

    /**
     * Returns a list of all known containments.
     *
     * @param category Only containments matching this category will be returned.
     *                 Useful in conjunction with knownCategories.
     *                 If "Miscellaneous" is passed in, then containments without a
     *                 Categories= entry are also returned.
     *                 If an empty string is passed in, all containments are
     *                 returned.
     * @param parentApp the application to filter containments on. Uses the
     *                  X-KDE-ParentApp entry (if any) in the plugin info.
     *                  The default value of QString() will result in a
     *                  list containing only containments not specifically
     *                  registered to an application.
     * @return list of containments
     **/
    static KPluginInfo::List listContainments(const QString &category = QString(),
            const QString &parentApp = QString());

    /**
     * Returns a list of all known containments that match the parameters.
     *
     * @param type Only containments with this string in X-Plasma-ContainmentType
     *             in their .desktop files will be returned. Common values are panel and
     *             desktop
     * @param category Only containments matchin this category will be returned.
     *                 Useful in conjunction with knownCategories.
     *                 If "Miscellaneous" is passed in, then containments without a
     *                 Categories= entry are also returned.
     *                 If an empty string is passed in, all containments are
     *                 returned.
     * @param parentApp the application to filter containments on. Uses the
     *                  X-KDE-ParentApp entry (if any) in the plugin info.
     *                  The default value of QString() will result in a
     *                  list containing only containments not specifically
     *                  registered to an application.
     * @return list of containments
     **/
    static KPluginInfo::List listContainmentsOfType(const QString &type,
            const QString &category = QString(),
            const QString &parentApp = QString());

    /**
     * @return a list of all known types of containments on this system
     */
    static QStringList listContainmentTypes();

    /**
     * Returns a list of all known containments associated with a certain MimeType
     *
     * @return list of containments
     **/
    static KPluginInfo::List listContainmentsForMimeType(const QString &mimeType);

    /**
     * Returns a list of all known dataengines.
     *
     * @param parentApp the application to filter dataengines on. Uses the
     *                  X-KDE-ParentApp entry (if any) in the plugin info.
     *                  The default value of QString() will result in a
     *                  list containing only dataengines not specifically
     *                  registered to an application.
     * @return list of dataengines
     **/
    KPluginInfo::List listDataEngineInfo(const QString &parentApp = QString());

    /**
     * Returns a list of all known ContainmentActions.
     *
     * @param parentApp the application to filter ContainmentActions on. Uses the
     *                  X-KDE-ParentApp entry (if any) in the plugin info.
     *                  The default value of QString() will result in a
     *                  list containing only ContainmentActions not specifically
     *                  registered to an application.
     * @return list of ContainmentActions
     **/
    KPluginInfo::List listContainmentActionsInfo(const QString &parentApp);

    /**
     * Set the plugin loader which will be queried for all loads.
     *
     * @param loader A subclass of PluginLoader which will be supplied
     * by the application
     **/
    static void setPluginLoader(PluginLoader *loader);

    /**
     * Return the active plugin loader
     **/
    static PluginLoader *self();

protected:
    /**
     * A re-implementable method that allows subclasses to override
     * the default behaviour of loadApplet. If the applet requested is not recognized,
     * then the implementation should return a NULL pointer. This method is called
     * by loadApplet prior to attempting to load an applet using the standard Plasma
     * plugin mechanisms.
     *
     * @param name the plugin name, as returned by KPluginInfo::pluginName()
     * @param appletId unique ID to assign the applet, or zero to have one
     *        assigned automatically.
     * @param args to send the applet extra arguments
     * @return a pointer to the loaded applet, or 0 on load failure
     **/
    virtual Applet *internalLoadApplet(const QString &name, uint appletId = 0,
                                       const QVariantList &args = QVariantList());

    /**
     * A re-implementable method that allows subclasses to override
     * the default behaviour of loadDataEngine. If the engine requested is not recognized,
     * then the implementation should return a NULL pointer. This method is called
     * by loadDataEngine prior to attempting to load a DataEgine using the standard Plasma
     * plugin mechanisms.
     *
     * @param name the name of the engine
     * @return the data engine that was loaded, or the NullEngine on failure.
     **/
    virtual DataEngine *internalLoadDataEngine(const QString &name);

    /**
     * A re-implementable method that allows subclasses to override
     * the default behaviour of loadService. If the service requested is not recognized,
     * then the implementation should return a NULL pointer. This method is called
     * by loadService prior to attempting to load a Service using the standard Plasma
     * plugin mechanisms.
     *
     * @param name the plugin name of the service to load
     * @param args a list of arguments to supply to the service plugin when loading it
     * @param parent the parent object, if any, for the service
     *
     * @return a Service object, unlike Plasma::Service::loadService, this can return null.
     **/
    virtual Service *internalLoadService(const QString &name, const QVariantList &args, QObject *parent = 0);

    /**
     * A re-implementable method that allows subclasses to override
     * the default behaviour of loadContainmentActions. If the ContainmentActions requested is not recognized,
     * then the implementation should return a NULL pointer. This method is called
     * by loadService prior to attempting to load a Service using the standard Plasma
     * plugin mechanisms.
     *
     * Returns a pointer to the containmentactions if successful.
     * The caller takes responsibility for the containmentactions, including
     * deleting it when no longer needed.
     *
     * @param parent the parent containment. @since 4.6 null is allowed.
     * @param name the plugin name, as returned by KPluginInfo::pluginName()
     * @param args to send the containmentactions extra arguments
     * @return a ContaimentActions object
     **/
    virtual ContainmentActions *internalLoadContainmentActions(Containment *parent, const QString &containmentActionsName, const QVariantList &args);

    /**
     * A re-implementable method that allows subclasses to override
     * the default behaviour of loadPackage. If the service requested is not recognized,
     * then the implementation should return a NULL pointer. This method is called
     * by loadService prior to attempting to load a Service using the standard Plasma
     * plugin mechanisms.
     *
     * @param name the plugin name of the service to load
     * @param args a list of arguments to supply to the service plugin when loading it
     * @param parent the parent object, if any, for the service
     *
     * @return a Service object, unlike Plasma::Service::loadService, this can return null.
     * @deprecated since 5.29
     **/
#ifndef PLASMA_NO_DEPRECATED
    virtual PLASMA_DEPRECATED Package internalLoadPackage(const QString &name, const QString &specialization);
#endif

    /**
     * A re-implementable method that allows subclasses to provide additional applets
     * for listAppletInfo. If the application has no applets to give to the application,
     * then the implementation should return an empty list.
     *
     * This method is called by listAppletInfo prior to generating the list of applets installed
     * on the system using the standard Plasma plugin mechanisms, and will try to find .desktop
     * files for your applets.
     *
     * @param category Only applets matching this category will be returned.
     *                 Useful in conjunction with knownCategories.
     *                 If "Misc" is passed in, then applets without a
     *                 Categories= entry are also returned.
     *                 If an empty string is passed in, all applets are
     *                 returned.
     * @return list of applets
     **/
    virtual KPluginInfo::List internalAppletInfo(const QString &category) const;

    /**
     * A re-implementable method that allows subclasses to provide additional dataengines
     * for DataEngine::listDataEngines.
     *
     * @return list of dataengine info, or an empty list if none
     **/
    virtual KPluginInfo::List internalDataEngineInfo() const;

    /**
     * Returns a list of all known Service implementations
     *
     * @return list of Service info, or an empty list if none
     */
    virtual KPluginInfo::List internalServiceInfo() const;

    /**
     * Returns a list of all known ContainmentActions implementations
     *
     * @return list of ContainmentActions info, or an empty list if none
     */
    virtual KPluginInfo::List internalContainmentActionsInfo() const;

    /**
     * Standardized mechanism for providing internal applets by install .desktop files
     * in $APPPDATA/plasma/internal/applets/
     *
     * For applications that do this, internalAppletInfo can be implemented as a one-liner
     * call to this method.
     *
     * @param category Only applets matching this category will be returned.
     *                 Useful in conjunction with knownCategories.
     *                 If "Misc" is passed in, then applets without a
     *                 Categories= entry are also returned.
     *                 If an empty string is passed in, all applets are
     *                 returned.
     * @return list of applets, or an empty list if none
     */
    KPluginInfo::List standardInternalAppletInfo(const QString &category) const;

    /**
     * Standardized mechanism for providing internal dataengines by install .desktop files
     * in $APPPDATA/plasma/internal/dataengines/
     *
     * For applications that do this, internalDataEngineInfo can be implemented as a one-liner
     * call to this method.
     *
     * @return list of dataengines
     */
    KPluginInfo::List standardInternalDataEngineInfo() const;

    /**
     * Standardized mechanism for providing internal services by install .desktop files
     * in $APPPDATA/plasma/internal/services/
     *
     * For applications that do this, internalServiceInfo can be implemented as a one-liner
     * call to this method.
     *
     * @return list of services
     */
    KPluginInfo::List standardInternalServiceInfo() const;

    PluginLoader();
    virtual ~PluginLoader();

private:
    bool isPluginVersionCompatible(KPluginLoader &loader);

    PluginLoaderPrivate *const d;
};

}

Q_DECLARE_METATYPE(Plasma::PluginLoader *)

#endif
