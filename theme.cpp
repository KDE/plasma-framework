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
       : themeName("default")
   {
   }

   KConfigGroup config()
   {
       QString groupName = "Theme";
       if (!app.isEmpty() && app != "plasma") {
           groupName.append("-").append(app);
       }

       KSharedConfig::Ptr config = KSharedConfig::openConfig("plasmarc");
       return KConfigGroup(config, groupName);
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
    d->config().writeEntry("name", d->themeName);
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
    setThemeName(d->config().readEntry("name", d->themeName));
}

void Theme::setThemeName(const QString &themeName)
{
    if (themeName != d->themeName &&
        !KStandardDirs::locate("data", "desktoptheme/" + d->themeName).isEmpty()) {
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

    if (path.isEmpty()) {
        if (d->themeName != "default") {
            search = "desktoptheme/default/" + name + ".svg";
            path = KStandardDirs::locate("data", search);
        }

        if (path.isEmpty()) {
            kDebug() << "Theme says: bad image path " << name 
                     << "; looked in: " << search <<  endl;
        }
    }

    return path;
}

}

#include <theme.moc>
