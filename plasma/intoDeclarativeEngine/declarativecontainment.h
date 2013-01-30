/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Ménard Alexis <darktears31@gmail.com>
 *   Copyright (c) 2009 Chani Armitage <chani@kde.org>
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

#ifndef PLASMA_CONTAINMENT_H
#define PLASMA_CONTAINMENT_H

#include <QObject>
#include <QWidget>

#include <kplugininfo.h>
#include <ksharedconfig.h>

#include <plasma/applet.h>


namespace Plasma
{

class AccessAppletJob;
class AppletHandle;
class DataEngine;
class Package;
class Corona;
class View;
class Wallpaper;
class DeclarativeContainmentActions;
class DeclarativeContainmentPrivate;
class AbstractToolBox;


class PLASMA_EXPORT DeclarativeContainment : public Applet
{
    Q_OBJECT

    public:

        void focusNextApplet();
        void focusPreviousApplet();

    private:


        Q_PRIVATE_SLOT(d, void showDropZoneDelayed())
        Q_PRIVATE_SLOT(d, void remoteAppletReady(Plasma::AccessAppletJob *))
        /**
        * This slot is called when the 'stat' after a job event has finished.
        */
        Q_PRIVATE_SLOT(d, void mimeTypeRetrieved(KIO::Job *, const QString &))
        Q_PRIVATE_SLOT(d, void dropJobResult(KJob *))

        DeclarativeContainmentPrivate *const d;
};

} // Plasma namespace

#endif // multiple inclusion guard
