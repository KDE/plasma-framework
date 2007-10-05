/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
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

#include "theme.h"

#include <KSharedConfig>
#include <KStandardDirs>
#include <kdebug.h>
#include <kconfiggroup.h>

namespace Plasma
{

class Theme::Private
{
public:
   Private()
       : themeName( "default" )
   {
   }

   QString themeName;
   QString app;
};

class ThemeSingleton
{
public:
   Theme self;
};

K_GLOBAL_STATIC( ThemeSingleton, privateThemeSelf )

Theme* Theme::self()
{
    return &privateThemeSelf->self;
}

Theme::Theme(QObject* parent)
    : QObject(parent),
      d(new Private)
{
    settingsChanged();
}

Theme::~Theme()
{
    delete d;
}

void Theme::setApplication(const QString &appname)
{
    if (d->app != appname) {
        d->app = appname;
        settingsChanged();
    }
}

void Theme::settingsChanged()
{
    QString groupName = "Theme";
    if (!d->app.isEmpty() && d->app != "plasma") {
        groupName.append("-").append(d->app);
    }

    KSharedConfig::Ptr config = KSharedConfig::openConfig("plasmarc");
    KConfigGroup group(config, groupName);
    QString themeName = group.readEntry("name", d->themeName);
    if (themeName != d->themeName) {
        d->themeName = themeName;
        emit changed();
    }
}

QString Theme::themeName() const
{
    return d->themeName;
}

QString Theme::image( const QString& name ) const
{
    QString search = "desktoptheme/" + d->themeName + '/' + name + ".svg";
    QString path =  KStandardDirs::locate( "data", search );

    if ( path.isEmpty() ) {
        kDebug() << "Theme says: bad image path " << name 
                 << "; looked in: " << search
                 <<  endl;
    }

    return path;
}

}

#include <theme.moc>
