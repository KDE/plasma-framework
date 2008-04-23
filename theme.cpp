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
#ifdef Q_WS_X11
#include <QX11Info>
#endif

#include <KWindowSystem>
#include <KColorScheme>
#include <KConfigGroup>
#include <KDebug>
#include <KSelectionWatcher>
#include <KSharedConfig>
#include <KStandardDirs>
#include <KGlobalSettings>

#include "plasma/packages_p.h"

namespace Plasma
{

class Theme::Private
{
public:
   Private()
       : locolor(false),
         isDefault(false),
         compositingActive(KWindowSystem::compositingActive())
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

    QString findInTheme(const QString &image, const QString &theme) const;
    static const char *defaultTheme;

    static PackageStructure::Ptr packageStructure;
    QString themeName;
    QString app;
    KSharedConfigPtr colors;
    QFont generalFont;
    bool locolor;
    bool compositingActive;
    bool isDefault;
#ifdef Q_WS_X11
    KSelectionWatcher *compositeWatch;
#endif
};

PackageStructure::Ptr Theme::Private::packageStructure(0);
const char *Theme::Private::defaultTheme = "default";

class ThemeSingleton
{
public:
    ThemeSingleton()
    {
        self.d->isDefault = true;
    }

   Theme self;
};

K_GLOBAL_STATIC( ThemeSingleton, privateThemeSelf )

Theme* Theme::defaultTheme()
{
    return &privateThemeSelf->self;
}

Theme::Theme(QObject* parent)
    : QObject(parent),
      d(new Private)
{
    settingsChanged();

#ifdef Q_WS_X11
    Display *dpy = QX11Info::display();
    int screen = DefaultScreen(dpy);
    d->locolor = DefaultDepth(dpy, screen) < 16;

    if (!d->locolor) {
        char net_wm_cm_name[100];
        sprintf(net_wm_cm_name, "_NET_WM_CM_S%d", screen);
        d->compositeWatch = new KSelectionWatcher(net_wm_cm_name, -1, this);
        connect(d->compositeWatch, SIGNAL(newOwner(Window)), this, SLOT(compositingChanged()));
        connect(d->compositeWatch, SIGNAL(lostOwner()), this, SLOT(compositingChanged()));
    }
#endif
}

Theme::~Theme()
{
    delete d;
}

PackageStructure::Ptr Theme::packageStructure()
{
    if (!Private::packageStructure) {
        Private::packageStructure = new ThemePackage();
    }

    return Private::packageStructure;
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
    setThemeName(d->config().readEntry("name", Private::defaultTheme));
}

void Theme::compositingChanged()
{
#ifdef Q_WS_X11
    bool compositingActive = d->compositeWatch->owner() != None;

    if (d->compositingActive != compositingActive) {
        d->compositingActive = compositingActive;
        emit themeChanged();
    }
#endif
}

void Theme::colorsChanged()
{
    emit themeChanged();
}

void Theme::setThemeName(const QString &themeName)
{
    QString theme = themeName;
    if (theme.isEmpty() || theme == d->themeName) {
        // let's try and get the default theme at least
        if (d->themeName.isEmpty()) {
            theme = Private::defaultTheme;
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

        theme = Private::defaultTheme;
    }

    d->themeName = theme;

    // load the color scheme config
    QString colorsFile = KStandardDirs::locate("data", "desktoptheme/" + theme + "/colors");
    //kDebug() << "we're going for..." << colorsFile << "*******************";

    disconnect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), this, SLOT(colorsChanged()));
    if (colorsFile.isEmpty()) {
        d->colors = 0;
        connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), this, SLOT(colorsChanged()));
    } else {
        d->colors = KSharedConfig::openConfig(colorsFile);
    }

    if (d->isDefault) {
        // we're the default theme, let's save our state
        KConfigGroup cg = d->config();
        if (Private::defaultTheme == d->themeName) {
            cg.deleteEntry("name");
        } else {
            cg.writeEntry("name", d->themeName);
        }
    }

    emit themeChanged();
}

QString Theme::themeName() const
{
    return d->themeName;
}

QString Theme::Private::findInTheme(const QString &image, const QString &theme) const
{
    //TODO: this should be using Package
    QString search;

    if (locolor) {
        search = "desktoptheme/" + theme + "/locolor/" + image + ".svg";
        search =  KStandardDirs::locate("data", search);
    } else if (!compositingActive) {
        search = "desktoptheme/" + theme + "/opaque/" + image + ".svg";
        search =  KStandardDirs::locate("data", search);
    }

    //not found or compositing enabled
    if (search.isEmpty()) {
        search = "desktoptheme/" + theme + '/' + image + ".svg";
        search =  KStandardDirs::locate("data", search);
    }

    return search;
}

QString Theme::imagePath(const QString& name)  const
{
    QString path = d->findInTheme(name, d->themeName);

    if (path.isEmpty() && d->themeName != Private::defaultTheme) {
        path = d->findInTheme(name, Private::defaultTheme);
    }

    if (path.isEmpty()) {
        kDebug() << "Theme says: bad image path " << name;
    }

    return path;
}

KSharedConfigPtr Theme::colorScheme() const
{
    return d->colors;
}

QColor Theme::textColor() const
{
    KColorScheme colorScheme(QPalette::Active, KColorScheme::Window, Theme::defaultTheme()->colorScheme());
    return colorScheme.foreground(KColorScheme::NormalText).color();
}

QColor Theme::backgroundColor() const
{
    KColorScheme colorScheme(QPalette::Active, KColorScheme::Window, Theme::defaultTheme()->colorScheme());
    return colorScheme.background().color();
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

bool Theme::compositingActive() const
{
    return d->compositingActive;
}

}

#include <theme.moc>
