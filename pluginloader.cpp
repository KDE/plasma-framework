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
#include <kglobal.h>
#include <kdebug.h>

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
    if(!s_pluginLoader) {
        s_pluginLoader = loader;
    } else {
        kDebug() << "Cannot set pluginLoader, already set!";
    }
}

PluginLoader* PluginLoader::pluginLoader()
{
    return s_pluginLoader;
}

Applet* PluginLoader::loadApplet(const QString &name, uint appletId, const QVariantList &args)
{ Q_UNUSED(name) Q_UNUSED(appletId) Q_UNUSED(args) return 0;
}

DataEngine* PluginLoader::loadDataEngine(const QString &name)
{ Q_UNUSED(name) return 0;
}

Service* PluginLoader::loadService(const QString &name, const QVariantList &args, QObject *parent)
{ Q_UNUSED(name) Q_UNUSED(args) Q_UNUSED(parent) return 0;
}

} // Plasma Namespace

