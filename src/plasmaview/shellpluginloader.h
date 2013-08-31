/*
 *   Copyright 2013 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SHELLPLUGINLOADER_H
#define SHELLPLUGINLOADER_H

#include <Plasma/PluginLoader>
#include <plasmaview/plasmaview_export.h>

/**
 * @class ShellPluginLoader plasmaview/shellpluginloader.h <PlasmaView/ShellPluginLoader>
 *
 * This class loads the shell package. Normally you won't need this class, unless if you
 * want to use the PlasmaView::View and Plasma::Corona directly.
 *
 * This class must be instantiated before the Corona and the View
 * example:
 *
 * <code>
 *     // example 1, it works
 *      Plasma::PluginLoader::setPluginLoader(new ShellPluginLoader);
 *      Plasma::Corona *corona = new Plasma::Corona();
 *      View *view = new View(corona);
 *
 *      // exmaple 2, it doesn't work
 *      Plasma::corona *corona = new plasma::corona();
 *      View *view = new view(corona);
 *      Plasma::PluginLoader::setPluginLoader(new shellpluginloader);
 * </code>
 */

class ShellPluginLoaderPrivate;

class PLASMAVIEW_EXPORT ShellPluginLoader : public Plasma::PluginLoader
{
public:
    ShellPluginLoader();
    ~ShellPluginLoader();

protected:
    Plasma::Package internalLoadPackage(const QString &packageFormat, const QString &specialization);
private:
    ShellPluginLoaderPrivate *const d;

};

#endif
