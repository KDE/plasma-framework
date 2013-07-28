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

#include "desktopshellhandler.h"

#include <QDebug>

DesktopShellHandler::DesktopShellHandler(QObject *parent)
{
    qDebug() << "Instantiating the desktop shell handler";
}

DesktopShellHandler::~DesktopShellHandler()
{
    qDebug() << "Killing the desktop shell handler";
}

void DesktopShellHandler::load()
{
    qDebug() << "Loading the desktop shell";
}

void DesktopShellHandler::unload()
{
    qDebug() << "Unloading the desktop shell";
}


#include "moc_desktopshellhandler.cpp"
