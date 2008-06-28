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
#include <KGlobal>
#include <KSelectionWatcher>
#include <KSharedConfig>
#include <KStandardDirs>
#include <KGlobalSettings>

#include "plasma/packages_p.h"

namespace Plasma
{

#define DEFAULT_WALLPAPER_THEME "Blue_Curl";
#define DEFAULT_WALLPAPER_FORMAT ".jpg";
static const int DEFAULT_WALLPAPER_WIDTH = 1920;
static const int DEFAULT_WALLPAPER_HEIGHT = 1200;

class Theme::Private
{
public:
    Private(Theme *theme)
        : q(theme),
          locolor(false),
          compositingActive(KWindowSystem::compositingActive()),
          isDefault(false),
          useGlobal(true)
    {
        generalFont = QApplication::font();
    }

    KConfigGroup& config()
    {
        if (!cfg.isValid()) {
            QString groupName = "Theme";

            if (!useGlobal) {
                QString app = KGlobal::mainComponent().componentName();

                if (!app.isEmpty() && app != "plasma") {
                    kDebug() << "using theme for app" << app;
                    groupName.append("-").append(app);
                }
            }

            cfg = KConfigGroup(KSharedConfig::openConfig("plasmarc"), groupName);
        }

        return cfg;
    }

    QString findInTheme(const QString &image, const QString &theme) const;
    void compositingChanged();

    static const char *defaultTheme;
    static PackageStructure::Ptr packageStructure;

    Theme *q;
    QString themeName;
    KSharedConfigPtr colors;
    KConfigGroup cfg;
    QFont generalFont;
#ifdef Q_WS_X11
    KSelectionWatcher *compositeWatch;
#endif
    bool locolor : 1;
    bool compositingActive : 1;
    bool isDefault : 1;
    bool useGlobal : 1;
};

PackageStructure::Ptr Theme::Private::packageStructure(0);
const char *Theme::Private::defaultTheme = "default";

QString Theme::Private::findInTheme(const QString &image, const QString &theme) const
{
    //TODO: this should be using Package
    QString search;

    if (locolor) {
        search = "desktoptheme/" + theme + "/locolor/" + image;
        search =  KStandardDirs::locate("data", search);
    } else if (!compositingActive) {
        search = "desktoptheme/" + theme + "/opaque/" + image;
        search =  KStandardDirs::locate("data", search);
    }

    //not found or compositing enabled
    if (search.isEmpty()) {
        search = "desktoptheme/" + theme + '/' + image;
        search =  KStandardDirs::locate("data", search);
    }

    return search;
}

void Theme::Private::compositingChanged()
{
#ifdef Q_WS_X11
    bool nowCompositingActive = compositeWatch->owner() != None;

    if (compositingActive != nowCompositingActive) {
        compositingActive = nowCompositingActive;
        emit q->themeChanged();
    }
#endif
}

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
      d(new Private(this))
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

void Theme::settingsChanged()
{
    setThemeName(d->config().readEntry("name", Private::defaultTheme));
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
    QString themePath = KStandardDirs::locate("data", "desktoptheme/" + theme + '/');
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

    disconnect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), this, SIGNAL(themeChanged()));
    if (colorsFile.isEmpty()) {
        d->colors = 0;
        connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), this, SIGNAL(themeChanged()));
    } else {
        d->colors = KSharedConfig::openConfig(colorsFile);
    }

    if (d->isDefault) {
        // we're the default theme, let's save our state
        KConfigGroup &cg = d->config();
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

QString Theme::imagePath(const QString& name)  const
{
    QString path = d->findInTheme(name + ".svg", d->themeName);

    if (path.isEmpty() && d->themeName != Private::defaultTheme) {
        path = d->findInTheme(name + ".svg", Private::defaultTheme);
    }

    if (path.isEmpty()) {
        kDebug() << "Theme says: bad image path " << name;
    }

    return path;
}

QString Theme::wallpaperPath(const QSize &size) const
{
    QString fullPath;
    QString image = DEFAULT_WALLPAPER_THEME;
    QString format = DEFAULT_WALLPAPER_FORMAT;
    int width = DEFAULT_WALLPAPER_WIDTH;
    int height = DEFAULT_WALLPAPER_HEIGHT;

    if (size.isValid()) {
        width = size.width();
        height = size.height();
    }

    bool checkInTheme = false;
    if (d->colors) {
        // we have a theme color config, so let's also check to see if
        // there is a wallpaper defined in there.
        KConfigGroup cg(d->colors, "Wallpaper");
        if (cg.hasKey("wallpaper")) {
            checkInTheme = true;
            image = cg.readEntry("defaultTheme", image);
            format = cg.readEntry("defaultFileSuffix", format);
            width = cg.readEntry("defaultWidth", width);
            height = cg.readEntry("defaultHeight", height);
        }
    }

    if (!checkInTheme) {
        // since we didn't find an entry in the theme, let's look in the main
        // theme config
        KConfigGroup &cg = d->config();
        image = cg.readEntry("defaultTheme", image);
    }

    image.append("/contents/images/%1x%2").append(format);
    //TODO: this should do better than just fallback to the default size.
    //      a "best fit" matching would be far better, so we don't end
    //      up returning a 1900x1200 wallpaper for a 640x480 request ;)
    QString defaultImage = image.arg(DEFAULT_WALLPAPER_WIDTH).arg(DEFAULT_WALLPAPER_HEIGHT);
    image = image.arg(width).arg(height);

    if (checkInTheme) {
        // check in the theme, since it was defined in the colors config
        fullPath = d->findInTheme("wallpaper/" + image, d->themeName);

        if (fullPath.isEmpty()) {
            fullPath = d->findInTheme("wallpaper/" + defaultImage, d->themeName);
        }
    }

    if (fullPath.isEmpty()) {
        // we failed to find it in the theme, so look in the standard directories
        //kDebug() << "looking for" << image;
        fullPath = KStandardDirs::locate("wallpaper", image);
    }

    if (fullPath.isEmpty()) {
        // we still failed to find it in the theme, so look for the default in
        // the standard directories
        //kDebug() << "looking for" << defaultImage;
        fullPath = KStandardDirs::locate("wallpaper", defaultImage);

        if (fullPath.isEmpty()) {
            kDebug() << "exhausted every effort to find a wallpaper.";
        }
    }

    return fullPath;
}

bool Theme::currentThemeHasImage(const QString& name)  const
{
    return (!d->findInTheme(name + ".svg", d->themeName).isEmpty());
}

KSharedConfigPtr Theme::colorScheme() const
{
    return d->colors;
}

QColor Theme::color(ColorRole role) const
{
    KColorScheme colorScheme(QPalette::Active, KColorScheme::Window, Theme::defaultTheme()->colorScheme());

    switch (role) {
        case TextColor:
            return colorScheme.foreground(KColorScheme::NormalText).color();
            break;

        case HighlightColor:
            return colorScheme.background(KColorScheme::ActiveBackground).color();
            break;

        case BackgroundColor:
            return colorScheme.background().color();
            break;
    }

    return QColor();
}

void Theme::setFont(const QFont &font, FontRole role)
{
    Q_UNUSED(role)
    d->generalFont = font;
}

QFont Theme::font(FontRole role) const
{
    Q_UNUSED(role)
    return d->generalFont;
}

QFontMetrics Theme::fontMetrics() const
{
    //TODO: allow this to be overridden with a plasma specific font?
    return QFontMetrics(d->generalFont);
}

bool Theme::windowTranslucencyEnabled() const
{
    return d->compositingActive;
}

void Theme::setUseGlobalSettings(bool useGlobal)
{
    if (d->useGlobal == useGlobal) {
        return;
    }

    d->useGlobal = useGlobal;
    d->cfg = KConfigGroup();
    d->themeName = QString();
    settingsChanged();
}

bool Theme::useGlobalSettings() const
{
    return d->useGlobal;
}

}

#include <theme.moc>
