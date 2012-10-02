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

#include <plasma/plasma.h>
#include <kplugininfo.h>

namespace Plasma {

class Applet;
class DataEngine;
class Service;
class AbstractRunner;

class PluginLoaderPrivate;

//TODO:
// * add support for ContainmentActions plugins
// * add KPluginInfo listing support for Containments (already loaded via the applet loading code)

/**
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
    PluginLoader();

    virtual ~PluginLoader();

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
     * Load a DataEngine plugin.
     *
     * @param name the name of the engine
     * @return the DataEngine that was loaded, or the NullEngine on failure.
     **/
    DataEngine *loadDataEngine(const QString &name);

    /**
     * Load a Runner plugin
     *
     * @return the Runner that was loaded, or 0 on failure.
     */
    AbstractRunner *loadRunner(const QString &name);

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
     **/
    KPluginInfo::List listAppletInfo(const QString &category, const QString &parentApp = QString());

    /**
     * Returns a list of all known DataEngines.
     *
     * @param parentApp the application to filter applets on. Uses the
     *                  X-KDE-ParentApp entry (if any) in the plugin info.
     *                  The default value of QString() will result in a
     *                  list containing only applets not specifically
     *                  registered to an application.
     * @return list of DataEngines
     **/
    KPluginInfo::List listDataEngineInfo(const QString &parentApp = QString());

    /**
     * Returns a list of all known Runner implementations
     *
     * @param parentApp the application to filter applets on. Uses the
     *                  X-KDE-ParentApp entry (if any) in the plugin info.
     *                  The default value of QString() will result in a
     *                  list containing only applets not specifically
     *                  registered to an application.
     * @return list of AbstractRunners
     **/
    KPluginInfo::List listRunnerInfo(const QString &parentApp = QString());

    /**
     * Set the plugin loader which will be queried for all loads.
     *
     * @param loader A subclass of PluginLoader which will be supplied
     * by the application
     **/
    static void setPluginLoader(PluginLoader* loader);

    /**
     * Return the active plugin loader
     **/
    static PluginLoader* pluginLoader();

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
     * the default behaviour of loadRunner. If the runner requested is not recognized,
     * then the implementation should return a NULL pointer. This method is called
     * by loadRunner prior to attempting to load a DataEgine using the standard Plasma
     * plugin mechanisms.
     *
     * @param name the name of the engine
     * @return the data engine that was loaded, or the NullEngine on failure.
     **/
    virtual AbstractRunner *internalLoadRunner(const QString &name);

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
     * A re-implementable method that allows subclasses to provide additional DataEngines
     * for DataEngineManager::listDataEngines.
     *
     * @return list of DataEngines, or an empty list if none
     **/
    virtual KPluginInfo::List internalDataEngineInfo() const;

    /**
     * Returns a list of all known Runner implementations
     *
     * @return list of AbstractRunners, or an empty list if none
     */
    virtual KPluginInfo::List internalRunnerInfo() const;

    /**
     * Returns a list of all known Runner implementations
     *
     * @return list of AbstractRunners, or an empty list if none
     */
    virtual KPluginInfo::List internalServiceInfo() const;

    /**
     * Standardized mechanism for providing internal Applets by install .desktop files
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
     * @return list of Applets, or an empty list if none
     */
    KPluginInfo::List standardInternalAppletInfo(const QString &category) const;

    /**
     * Standardized mechanism for providing internal Applets by install .desktop files
     * in $APPPDATA/plasma/internal/dataengines/
     *
     * For applications that do this, internalDataEngineInfo can be implemented as a one-liner
     * call to this method.
     *
     * @return list of applets
     */
    KPluginInfo::List standardInternalDataEngineInfo() const;

    /**
     * Standardized mechanism for providing internal Applets by install .desktop files
     * in $APPPDATA/plasma/internal/runners/
     *
     * For applications that do this, internalRunnerInfo can be implemented as a one-liner
     * call to this method.
     *
     * @return list of applets
     */
    KPluginInfo::List standardInternalRunnerInfo() const;

    /**
     * Standardized mechanism for providing internal Applets by install .desktop files
     * in $APPPDATA/plasma/internal/services/
     *
     * For applications that do this, internalRunnerInfo can be implemented as a one-liner
     * call to this method.
     *
     * @return list of applets
     */
    KPluginInfo::List standardInternalServiceInfo() const;

private:
    PluginLoaderPrivate * const d;
};

}

Q_DECLARE_METATYPE(Plasma::PluginLoader*)

#endif
