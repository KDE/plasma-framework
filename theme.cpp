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
#include <QX11Info>

#include <KWindowSystem>
#include <KColorScheme>
#include <KConfigGroup>
#include <KDebug>
#include <KSelectionWatcher>
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
   bool compositingActive;
#ifdef Q_WS_X11
   KSelectionWatcher *compositeWatch;
#endif
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
    d->compositingActive = KWindowSystem::compositingActive();

#ifdef Q_WS_X11
    char net_wm_cm_name[100];
    sprintf(net_wm_cm_name, "_NET_WM_CM_S%d", DefaultScreen(QX11Info::display()));
    d->compositeWatch = new KSelectionWatcher(net_wm_cm_name, -1, this);
    connect(d->compositeWatch, SIGNAL(newOwner(Window)), this, SLOT(compositingChanged()));
    connect(d->compositeWatch, SIGNAL(lostOwner()), this, SLOT(compositingChanged()));
#endif
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

void Theme::compositingChanged()
{
#ifdef Q_WS_X11
    bool compositingActive = d->compositeWatch->owner() != None;

    if (d->compositingActive != compositingActive) {
        d->compositingActive = compositingActive;
        emit changed();
    }
#endif
}

void Theme::setThemeName(const QString &themeName)
{
    QString theme = themeName;
    if (theme.isEmpty() || theme == d->themeName) {
        // let's try and get the default theme at least
        if (d->themeName.isEmpty()) {
            theme = "default";
        } else {
            return;
        }
    }

    //TODO: should we care about names with relative paths in them?
    QString themePath = KStandardDirs::locate("data", "desktoptheme/" + theme + "/");
    if (themePath.isEmpty() && d->themeName.isEmpty()) {
        themePath = KStandardDirs::locate("data", "desktoptheme/default/");

        if (themePath.isEmpty()) {
            return;
        }

        theme = "default";
    }

    d->themeName = theme;

    // load the color scheme config
    QString colorsFile = KStandardDirs::locate("data", "desktoptheme/" + theme + "/colors");
    //kDebug() << "we're going for..." << colorsFile << "*******************";

    if (colorsFile.isEmpty()) {
        d->colors = 0;
    } else {
        d->colors = KSharedConfig::openConfig(colorsFile);
    }

    emit changed();
}

QString Theme::themeName() const
{
    return d->themeName;
}

QString Theme::image( const QString& name ) const
{
    QString search;
    QString path;
    if (!d->compositingActive) {
        search = "desktoptheme/" + d->themeName + "/opaque/" + name + ".svg";
        path =  KStandardDirs::locate( "data", search );
    }
    //not found or compositing enabled
    if (path.isEmpty()) {
        search = "desktoptheme/" + d->themeName + '/' + name + ".svg";
        path =  KStandardDirs::locate( "data", search );
    }

    if (path.isEmpty()) {
        if (d->themeName != "default") {
            if (!d->compositingActive) {
                search = "desktoptheme/default/opaque/" + name + ".svg";
                path = KStandardDirs::locate("data", search);
            }
            if (path.isEmpty()) {
                search = "desktoptheme/default/" + name + ".svg";
                path = KStandardDirs::locate("data", search);
            }
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

QColor Theme::backgroundColor() const
{
    KColorScheme colors(QPalette::Active, KColorScheme::Window, Theme::self()->colors());
    return colors.background().color();
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
