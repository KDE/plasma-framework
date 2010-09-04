/*
*   Copyright 2010 Ryan Rix <ry@n.rix.si>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2 as
*   published by the Free Software Foundation
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

#include "testshellpluginloader.h"

#include <KDebug>

#include <Plasma/Plasma>
#include <Plasma/Applet>
#include <Plasma/Service>
#include <Plasma/DataEngine>

TestShellPluginLoader::~TestShellPluginLoader()
{
}

Plasma::Applet* TestShellPluginLoader::internalLoadApplet (const QString &name, uint appletId, const QVariantList &args)
{
    kDebug() << "loadApplet called with" << name << appletId << args;
    return 0;
}

Plasma::DataEngine* TestShellPluginLoader::internalLoadDataEngine(const QString &name)
{
    kDebug() << "loadEngine called with" << name;
    return 0;
}

Plasma::Service* TestShellPluginLoader::internalLoadService(const QString &name, const QVariantList &args, QObject *parent)
{
    kDebug() << "loadService called with" << name << args << parent;
    return 0;
}

