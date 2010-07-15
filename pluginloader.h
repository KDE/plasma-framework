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
 * Applet Loading logic can communicate with a parent application.
 * 
 * @author Ryan Rix <ry@n.rix.si>
 * @since 4.6
 **/
class PLASMA_EXPORT PluginLoader
{
public: 
    virtual ~PluginLoader();
    
    /**
     * Load an external applet and supply it to Plasma.
     *
     * @param name the plugin name, as returned by KPluginInfo::pluginName()
     * @param appletId unique ID to assign the applet, or zero to have one
     *        assigned automatically.
     * @param args to send the applet extra arguments
     * @return a pointer to the loaded applet, or 0 on load failure
     **/
    virtual Applet* loadApplet(const QString &name, uint appletId = 0,
                               const QVariantList &args = QVariantList());
    
    /**
     * Load an external DataEngine and supply it to Plasma.
     *
     * @param name the name of the engine
     * @return the data engine that was loaded, or the NullEngine on failure.
     **/
    virtual DataEngine* loadEngine(const QString &name);
    
    /**
     * Load an external Service and supply it to Plasma.
     *
     * @param name the plugin name of the service to load
     * @param args a list of arguments to supply to the service plugin when loading it
     * @param parent the parent object, if any, for the service
     *
     * @return a Service object, unlike Plasma::Service::loadService, this can return null.
     **/
    virtual Service* loadService(const QString &name, const QVariantList &args, QObject *parent = 0);
    
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

private:
    PluginLoaderPrivate* d;
};

}
Q_DECLARE_METATYPE( Plasma::PluginLoader* ) // so that it can be wrapped in QVariants, etc

#endif 
