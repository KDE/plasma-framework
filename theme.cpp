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

#include <QApplication>
#include <QFile>

#include <KColorScheme>
#include <KConfigGroup>
#include <KDebug>
#include <KSharedConfig>
#include <KStandardDirs>

namespace Plasma
{

class Theme::Private
{
public:
   Private()
   {
       generalFont = QApplication::font();
   }

   KConfigGroup config()
   {
       QString groupName = "Theme";
       if (!app.isEmpty() && app != "plasma") {
           groupName.append("-").append(app);
       }

       KSharedConfigPtr config = KSharedConfig::openConfig("plasmarc");
       return KConfigGroup(config, groupName);
   }

   QString themeName;
   QString app;
   KSharedConfigPtr colors;
   QFont generalFont;
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
    setThemeName(d->config().readEntry("name", "default"));
}

void Theme::setThemeName(const QString &themeName)
{
    if (themeName.isEmpty() || themeName == d->themeName) {
        // let's try and get the default theme at least
        if (d->themeName.isEmpty()) {
            setThemeName("default");
        }
        return;
    }

    //TODO: should we care about names with relative paths in them?
    QString themePath = KStandardDirs::locate("data", "desktoptheme/" + themeName + "/");
    if (themePath.isEmpty()) {
        // let's try and get the default theme at least
        if (d->themeName.isEmpty() && themeName != "default") {
            setThemeName("default");
        }
        return;
    }

    d->themeName = themeName;

    // load the color scheme config
    themePath = themePath.append("colors");
    if (QFile::exists(themePath)) {
        d->colors = KSharedConfig::openConfig(themePath);
    } else {
        d->colors = 0;
    }

    emit changed();
}

QString Theme::themeName() const
{
    return d->themeName;
}

QString Theme::image( const QString& name ) const
{
    //TODO: performance ... should we try and cache the results of KStandardDirs::locate?
    //      should we just use whatever theme path was returned and not care about cascades?
    //      (probably "no" on the last question)
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

KSharedConfigPtr Theme::colors() const
{
    return d->colors;
}

QColor Theme::textColor() const
{
    KColorScheme colors(QPalette::Active, KColorScheme::Window, Theme::self()->colors());
    return colors.foreground(KColorScheme::NormalText).color();
}

void Theme::setFont(const QFont &font)
{
    d->generalFont = font;
}

QFont Theme::font() const
{
    //TODO: allow this to be overridden with a plasma specific font?
    return d->generalFont;
}

QFontMetrics Theme::fontMetrics() const
{
    //TODO: allow this to be overridden with a plasma specific font?
    return QFontMetrics(d->generalFont);
}

}

#include <theme.moc>
