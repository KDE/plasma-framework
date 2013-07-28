/*
 *  Copyright (C) 2013 Ivan Cukic <ivan.cukic@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "desktopshellplugin.h"
#include "desktopshellhandler.h"

#include <QtQml>
#include <QDebug>

void DesktopShellPlugin::registerTypes(const char *uri)
{
    qDebug() << "This is me registering a plugin" << uri;
    Q_ASSERT(uri == QLatin1String("org.kde.plasma.shells.desktop"));
    qmlRegisterType<DesktopShellHandler>(uri, 0, 1, "HandlerObject");
}

#include "moc_desktopshellplugin.cpp"

