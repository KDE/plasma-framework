/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Ménard Alexis <darktears31@gmail.com>
 *   Copyright 2012 by Marco MArtin <mart@kde.org>
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

#ifndef DECLARATIVECONTAINMENT_P_H
#define DECLARATIVECONTAINMENT_P_H

#include <kactioncollection.h>
#include <kmenu.h>

#include "plasma.h"
#include "applet.h"
#include "corona.h"


class KJob;

namespace KIO
{
    class Job;
}

namespace Plasma
{

class AccessAppletJob;
class Containment;

class DeclarativeContainmentPrivate
{
public:
    DeclarativeContainmentPrivate(Containment *c)
        : q(c),
          focusedApplet(0),
          wallpaper(0),
          showDropZoneDelayTimer(0),
          dropZoneStarted(false),
    {
    }

    ~DeclarativeContainmentPrivate()
    {
        qDeleteAll(dropMenus);
        dropMenus.clear();
    }


    void clearDataForMimeJob(KIO::Job *job);
    void mimeTypeRetrieved(KIO::Job *job, const QString &mimetype);
    void dropJobResult(KJob *);
    void remoteAppletReady(Plasma::AccessAppletJob *job);


    /**
     * give keyboard focus to applet within this containment
     */
    void focusApplet(Plasma::Applet *applet);

    /**
     * Handles dropped/pasted mimetype data
     * @param screenPos screen-relative position
     * @param dropEvent the drop event (if null, the clipboard is used instead)
     */
    void dropData(QPoint screenPos, QDropEvent *dropEvent = 0);

    /**
     * Delayed drop zone display
     */
    void showDropZoneDelayed();

    DeclarativeContainment *q;
    Applet *focusedApplet;
    Plasma::Wallpaper *wallpaper;
    QHash<KJob*, QPointF> dropPoints;
    QHash<KJob*, KMenu*> dropMenus;
    QTimer *showDropZoneDelayTimer;
    bool dropZoneStarted : 1;

    static const char defaultWallpaper[];
    static const char defaultWallpaperMode[];
};

} // Plasma namespace

#endif
