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

namespace Plasma {

class Applet;
class DataEngine;
class Service;

class PluginLoaderPrivate;

/**
 * This is an abstract base class which defines an interface to which Plasma's 
 * Applet Loading logic can communicate with a parent application. The plugin loader
 * must be set before any plugins are loaded, otherwise (for safety reasons), the
 * default PluginLoader implementation will be used.
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
     * @return the data engine that was loaded, or the NullEngine on failure.
     **/
    DataEngine *loadDataEngine(const QString &name);

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

private:
    PluginLoaderPrivate * const d;
};

}
Q_DECLARE_METATYPE( Plasma::PluginLoader* ) // so that it can be wrapped in QVariants, etc

#endif 
