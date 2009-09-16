/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
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

#ifndef PLASMA_ASSOCIATEDAPPLICATIONMANAGER_P_H
#define PLASMA_ASSOCIATEDAPPLICATIONMANAGER_P_H

#include <QObject>

#include <kurl.h>

namespace Plasma
{
class Applet;

class AssociatedApplicationManagerPrivate;

class AssociatedApplicationManager : public QObject
{
    Q_OBJECT

public:
    static AssociatedApplicationManager *self();

    //set an application name for an applet
    void setApplication(Plasma::Applet *applet, const QString &application);
    //returns the application name associated to an applet
    QString application(const Plasma::Applet *applet) const;

    //sets the urls associated to an applet
    void setUrls(Plasma::Applet *applet, const KUrl::List &urls);
    //returns the urls associated to an applet
    KUrl::List urls(const Plasma::Applet *applet) const;

    //run the associated application or the urls if no app is associated
    void run(Plasma::Applet *applet);
    //returns true if the applet has a valid associated application or urls
    bool appletHasValidAssociatedApplication(const Plasma::Applet *applet) const;

private:
    AssociatedApplicationManager(QObject *parent = 0);
    ~AssociatedApplicationManager();

    AssociatedApplicationManagerPrivate *const d;
    friend class AssociatedApplicationManagerSingleton;

    Q_PRIVATE_SLOT(d, void cleanupApplet(QObject *obj))
};

} // namespace Plasma

#endif // multiple inclusion guard
