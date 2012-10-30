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
#include <QFileInfo>
#include <QMutableListIterator>
#include <QPair>
#include <QStringBuilder>
#include <QTimer>
#ifdef Q_WS_X11
#include <QX11Info>
#include "private/effectwatcher_p.h"
#endif

#include <kcolorscheme.h>
#include <kcomponentdata.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kmanagerselection.h>
#include <kimagecache.h>
#include <ksharedconfig.h>
#include <kstandarddirs.h>
#include <kwindowsystem.h>


#include "animations/animationscriptengine_p.h"
#include "libplasma-theme-global.h"
#include "private/packages_p.h"
#include "windoweffects.h"

namespace Plasma
{

//NOTE: Default wallpaper can be set from the theme configuration
#define DEFAULT_WALLPAPER_THEME "default"
#define DEFAULT_WALLPAPER_SUFFIX ".png"
static const int DEFAULT_WALLPAPER_WIDTH = 1920;
static const int DEFAULT_WALLPAPER_HEIGHT = 1200;

enum styles {
    DEFAULTSTYLE,
    SVGSTYLE
};

enum CacheType {
    NoCache = 0,
    PixmapCache = 1,
    SvgElementsCache = 2
};
Q_DECLARE_FLAGS(CacheTypes, CacheType)
Q_DECLARE_OPERATORS_FOR_FLAGS(CacheTypes)

class ThemePrivate
{
public:
    ThemePrivate(Theme *theme)
        : q(theme),
          colorScheme(QPalette::Active, KColorScheme::Window, KSharedConfigPtr(0)),
          buttonColorScheme(QPalette::Active, KColorScheme::Button, KSharedConfigPtr(0)),
          viewColorScheme(QPalette::Active, KColorScheme::View, KSharedConfigPtr(0)),
          defaultWallpaperTheme(DEFAULT_WALLPAPER_THEME),
          defaultWallpaperSuffix(DEFAULT_WALLPAPER_SUFFIX),
          defaultWallpaperWidth(DEFAULT_WALLPAPER_WIDTH),
          defaultWallpaperHeight(DEFAULT_WALLPAPER_HEIGHT),
          pixmapCache(0),
          cachesToDiscard(NoCache),
          locolor(false),
          compositingActive(KWindowSystem::self()->compositingActive()),
          blurActive(false),
          isDefault(false),
          useGlobal(true),
          hasWallpapers(false),
          useNativeWidgetStyle(false)
    {
        generalFont = QApplication::font();
        ThemeConfig config;
        cacheTheme = config.cacheTheme();

        saveTimer = new QTimer(q);
        saveTimer->setSingleShot(true);
        saveTimer->setInterval(600);
        QObject::connect(saveTimer, SIGNAL(timeout()), q, SLOT(scheduledCacheUpdate()));

        updateNotificationTimer = new QTimer(q);
        updateNotificationTimer->setSingleShot(true);
        updateNotificationTimer->setInterval(500);
        QObject::connect(updateNotificationTimer, SIGNAL(timeout()), q, SLOT(notifyOfChanged()));

        if (QPixmap::defaultDepth() > 8) {
            QObject::connect(KWindowSystem::self(), SIGNAL(compositingChanged(bool)), q, SLOT(compositingChanged(bool)));
#ifdef Q_WS_X11
            //watch for blur effect property changes as well
            if (!s_blurEffectWatcher) {
                s_blurEffectWatcher = new EffectWatcher("_KDE_NET_WM_BLUR_BEHIND_REGION");
            }

            QObject::connect(s_blurEffectWatcher, SIGNAL(effectChanged(bool)), q, SLOT(blurBehindChanged(bool)));
#endif
        }
    }

    ~ThemePrivate()
    {
       delete pixmapCache;
    }

    KConfigGroup &config()
    {
        if (!cfg.isValid()) {
            QString groupName = "Theme";

            if (!useGlobal) {
                QString app = KGlobal::mainComponent().componentName();

                if (!app.isEmpty()) {
                    kDebug() << "using theme for app" << app;
                    groupName.append("-").append(app);
                }
            }

            cfg = KConfigGroup(KSharedConfig::openConfig(themeRcFile), groupName);
        }

        return cfg;
    }

    QString findInTheme(const QString &image, const QString &theme, bool cache = true);
    void compositingChanged(bool active);
    void discardCache(CacheTypes caches);
    void scheduledCacheUpdate();
    void scheduleThemeChangeNotification(CacheTypes caches);
    void notifyOfChanged();
    void colorsChanged();
    void blurBehindChanged(bool blur);
    bool useCache();
    void settingsFileChanged(const QString &);
    void setThemeName(const QString &themeName, bool writeSettings);
    void onAppExitCleanup();
    void processWallpaperSettings(KConfigBase *metadata);
    void processAnimationSettings(const QString &theme, KConfigBase *metadata);

    const QString processStyleSheet(const QString &css);

    static const char *defaultTheme;
    static const char *systemColorsTheme;
    static const char *themeRcFile;
    static PackageStructure::Ptr packageStructure;
#ifdef Q_WS_X11
    static EffectWatcher *s_blurEffectWatcher;
#endif

    Theme *q;
    QString themeName;
    QList<QString> fallbackThemes;
    KSharedConfigPtr colors;
    KColorScheme colorScheme;
    KColorScheme buttonColorScheme;
    KColorScheme viewColorScheme;
    KConfigGroup cfg;
    QFont generalFont;
    QString defaultWallpaperTheme;
    QString defaultWallpaperSuffix;
    int defaultWallpaperWidth;
    int defaultWallpaperHeight;
    KImageCache *pixmapCache;
    KSharedConfigPtr svgElementsCache;
    QHash<QString, QSet<QString> > invalidElements;
    QHash<QString, QPixmap> pixmapsToCache;
    QHash<QString, QString> keysToCache;
    QHash<QString, QString> idsToCache;
    QHash<QString, QString> animationMapping;
    QHash<styles, QString> cachedStyleSheets;
    QHash<QString, QString> discoveries;
    QTimer *saveTimer;
    QTimer *updateNotificationTimer;
    int toolTipDelay;
    CacheTypes cachesToDiscard;

    bool locolor : 1;
    bool compositingActive : 1;
    bool blurActive : 1;
    bool isDefault : 1;
    bool useGlobal : 1;
    bool hasWallpapers : 1;
    bool cacheTheme : 1;
    bool useNativeWidgetStyle :1;
};

PackageStructure::Ptr ThemePrivate::packageStructure(0);
const char *ThemePrivate::defaultTheme = "default";
const char *ThemePrivate::themeRcFile = "plasmarc";
// the system colors theme is used to cache unthemed svgs with colorization needs
// these svgs do not follow the theme's colors, but rather the system colors
const char *ThemePrivate::systemColorsTheme = "internal-system-colors";
#ifdef Q_WS_X11
EffectWatcher *ThemePrivate::s_blurEffectWatcher = 0;
#endif

bool ThemePrivate::useCache()
{
    if (cacheTheme && !pixmapCache) {
        ThemeConfig config;
        pixmapCache = new KImageCache("plasma_theme_" + themeName, config.themeCacheKb() * 1024);
        if (themeName != systemColorsTheme) {
            //check for expired cache
            // FIXME: when using the system colors, if they change while the application is not running
            // the cache should be dropped; we need a way to detect system color change when the
            // application is not running.
            QFile f(KStandardDirs::locate("data", "desktoptheme/" + themeName + "/metadata.desktop"));
            QFileInfo info(f);
            if (info.lastModified().toTime_t() > uint(pixmapCache->lastModifiedTime())) {
                pixmapCache->clear();
            }
        }
    }

    return cacheTheme;
}

void ThemePrivate::onAppExitCleanup()
{
    pixmapsToCache.clear();
    delete pixmapCache;
    pixmapCache = 0;
    cacheTheme = false;
}

QString ThemePrivate::findInTheme(const QString &image, const QString &theme, bool cache)
{
    if (cache && discoveries.contains(image)) {
        return discoveries[image];
    }

    QString search;

    if (locolor) {
        search = QLatin1Literal("desktoptheme/") % theme % QLatin1Literal("/locolor/") % image;
        search =  KStandardDirs::locate("data", search);
    } else if (!compositingActive) {
        search = QLatin1Literal("desktoptheme/") % theme % QLatin1Literal("/opaque/") % image;
        search =  KStandardDirs::locate("data", search);
    } else if (WindowEffects::isEffectAvailable(WindowEffects::BlurBehind)) {
        search = QLatin1Literal("desktoptheme/") % theme % QLatin1Literal("/translucent/") % image;
        search =  KStandardDirs::locate("data", search);
    }

    //not found or compositing enabled
    if (search.isEmpty()) {
        search = QLatin1Literal("desktoptheme/") % theme % QLatin1Char('/') % image;
        search =  KStandardDirs::locate("data", search);
    }

    if (cache && !search.isEmpty()) {
        discoveries.insert(image, search);
    }

    return search;
}

void ThemePrivate::compositingChanged(bool active)
{
#ifdef Q_WS_X11
    if (compositingActive != active) {
        compositingActive = active;
        //kDebug() << QTime::currentTime();
        scheduleThemeChangeNotification(PixmapCache | SvgElementsCache);
    }
#endif
}

void ThemePrivate::discardCache(CacheTypes caches)
{
    if (caches & PixmapCache) {
        pixmapsToCache.clear();
        saveTimer->stop();
        if (pixmapCache) {
            pixmapCache->clear();
        }
    } else {
        // This deletes the object but keeps the on-disk cache for later use
        delete pixmapCache;
        pixmapCache = 0;
    }

    cachedStyleSheets.clear();

    if (caches & SvgElementsCache) {
        discoveries.clear();
        invalidElements.clear();

        if (svgElementsCache) {
            QFile f(svgElementsCache->name());
            svgElementsCache = 0;
            f.remove();
        }

        const QString svgElementsFile = KStandardDirs::locateLocal("cache", "plasma-svgelements-" + themeName);
        svgElementsCache = KSharedConfig::openConfig(svgElementsFile);
    }
}

void ThemePrivate::scheduledCacheUpdate()
{
    if (useCache()) {
        QHashIterator<QString, QPixmap> it(pixmapsToCache);
        while (it.hasNext()) {
            it.next();
            pixmapCache->insertPixmap(idsToCache[it.key()], it.value());
        }
    }

    pixmapsToCache.clear();
    keysToCache.clear();
    idsToCache.clear();
}

void ThemePrivate::colorsChanged()
{
    colorScheme = KColorScheme(QPalette::Active, KColorScheme::Window, colors);
    buttonColorScheme = KColorScheme(QPalette::Active, KColorScheme::Button, colors);
    viewColorScheme = KColorScheme(QPalette::Active, KColorScheme::View, colors);
    scheduleThemeChangeNotification(PixmapCache);
}

void ThemePrivate::blurBehindChanged(bool blur)
{
    if (blurActive != blur) {
        blurActive = blur;
        scheduleThemeChangeNotification(PixmapCache | SvgElementsCache);
    }
}

void ThemePrivate::scheduleThemeChangeNotification(CacheTypes caches)
{
    cachesToDiscard |= caches;
    updateNotificationTimer->start();
}

void ThemePrivate::notifyOfChanged()
{
    //kDebug() << cachesToDiscard;
    discardCache(cachesToDiscard);
    cachesToDiscard = NoCache;
    emit q->themeChanged();
}

const QString ThemePrivate::processStyleSheet(const QString &css)
{
    QString stylesheet;
    if (css.isEmpty()) {
        stylesheet = cachedStyleSheets.value(DEFAULTSTYLE);
        if (stylesheet.isEmpty()) {
            stylesheet = QString("\n\
                        body {\n\
                            color: %textcolor;\n\
                            font-size: %fontsize;\n\
                            font-family: %fontfamily;\n\
                        }\n\
                        a:active  { color: %activatedlink; }\n\
                        a:link    { color: %link; }\n\
                        a:visited { color: %visitedlink; }\n\
                        a:hover   { color: %hoveredlink; text-decoration: none; }\n\
                        ");
            stylesheet = processStyleSheet(stylesheet);
            cachedStyleSheets.insert(DEFAULTSTYLE, stylesheet);
        }

        return stylesheet;
    } else if (css == "SVG") {
        stylesheet = cachedStyleSheets.value(SVGSTYLE);
        if (stylesheet.isEmpty()) {
            QString skel = ".ColorScheme-%1{color:%2;}";

            stylesheet += skel.arg("Text","%textcolor");
            stylesheet += skel.arg("Background","%backgroundcolor");

            stylesheet += skel.arg("ButtonText","%buttontextcolor");
            stylesheet += skel.arg("ButtonBackground","%buttonbackgroundcolor");
            stylesheet += skel.arg("ButtonHover","%buttonhovercolor");
            stylesheet += skel.arg("ButtonFocus","%buttonfocuscolor");

            stylesheet += skel.arg("ViewText","%viewtextcolor");
            stylesheet += skel.arg("ViewBackground","%viewbackgroundcolor");
            stylesheet += skel.arg("ViewHover","%viewhovercolor");
            stylesheet += skel.arg("ViewFocus","%viewfocuscolor");

            stylesheet = processStyleSheet(stylesheet);
            cachedStyleSheets.insert(SVGSTYLE, stylesheet);
        }

        return stylesheet;
    } else {
        stylesheet = css;
    }

    QHash<QString, QString> elements;
    // If you add elements here, make sure their names are sufficiently unique to not cause
    // clashes between element keys
    elements["%textcolor"] = q->color(Theme::TextColor).name();
    elements["%backgroundcolor"] = q->color(Theme::BackgroundColor).name();
    elements["%visitedlink"] = q->color(Theme::VisitedLinkColor).name();
    elements["%activatedlink"] = q->color(Theme::HighlightColor).name();
    elements["%hoveredlink"] = q->color(Theme::HighlightColor).name();
    elements["%link"] = q->color(Theme::LinkColor).name();
    elements["%buttontextcolor"] = q->color(Theme::ButtonTextColor).name();
    elements["%buttonbackgroundcolor"] = q->color(Theme::ButtonBackgroundColor).name();
    elements["%buttonhovercolor"] = q->color(Theme::ButtonHoverColor).name();
    elements["%buttonfocuscolor"] = q->color(Theme::ButtonFocusColor).name();
    elements["%viewtextcolor"] = q->color(Theme::ViewTextColor).name();
    elements["%viewbackgroundcolor"] = q->color(Theme::ViewBackgroundColor).name();
    elements["%viewhovercolor"] = q->color(Theme::ViewHoverColor).name();
    elements["%viewfocuscolor"] = q->color(Theme::ViewFocusColor).name();

    QFont font = q->font(Theme::DefaultFont);
    elements["%fontsize"] = QString("%1pt").arg(font.pointSize());
    elements["%fontfamily"] = font.family().split('[').first();
    elements["%smallfontsize"] = QString("%1pt").arg(KGlobalSettings::smallestReadableFont().pointSize());

    QHash<QString, QString>::const_iterator it = elements.constBegin();
    QHash<QString, QString>::const_iterator itEnd = elements.constEnd();
    for ( ; it != itEnd; ++it) {
        stylesheet.replace(it.key(), it.value());
    }
    return stylesheet;
}

class ThemeSingleton
{
public:
    ThemeSingleton()
    {
        self.d->isDefault = true;

        //FIXME: if/when kconfig gets change notification, this will be unnecessary
        KDirWatch::self()->addFile(KStandardDirs::locateLocal("config", ThemePrivate::themeRcFile));
        QObject::connect(KDirWatch::self(), SIGNAL(created(QString)), &self, SLOT(settingsFileChanged(QString)));
        QObject::connect(KDirWatch::self(), SIGNAL(dirty(QString)), &self, SLOT(settingsFileChanged(QString)));
    }

   Theme self;
};

K_GLOBAL_STATIC(ThemeSingleton, privateThemeSelf)

Theme *Theme::defaultTheme()
{
    return &privateThemeSelf->self;
}

Theme::Theme(QObject *parent)
    : QObject(parent),
      d(new ThemePrivate(this))
{
    settingsChanged();
    if (QCoreApplication::instance()) {
        connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()),
                this, SLOT(onAppExitCleanup()));
    }
}

Theme::Theme(const QString &themeName, QObject *parent)
    : QObject(parent),
      d(new ThemePrivate(this))
{
    // turn off caching so we don't accidently trigger unnecessary disk activity at this point
    bool useCache = d->cacheTheme;
    d->cacheTheme = false;
    setThemeName(themeName);
    d->cacheTheme = useCache;
    if (QCoreApplication::instance()) {
        connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()),
                this, SLOT(onAppExitCleanup()));
    }
}

Theme::~Theme()
{
    if (d->svgElementsCache) {
        QHashIterator<QString, QSet<QString> > it(d->invalidElements);
        while (it.hasNext()) {
            it.next();
            KConfigGroup imageGroup(d->svgElementsCache, it.key());
            imageGroup.writeEntry("invalidElements", it.value().toList()); //FIXME: add QSet support to KConfig
        }
    }

    d->onAppExitCleanup();
    delete d;
}

PackageStructure::Ptr Theme::packageStructure()
{
    if (!ThemePrivate::packageStructure) {
        ThemePrivate::packageStructure = new ThemePackage();
    }

    return ThemePrivate::packageStructure;
}

KPluginInfo::List Theme::listThemeInfo()
{
    const QStringList themes = KGlobal::dirs()->findAllResources("data", "desktoptheme/*/metadata.desktop",
                                                           KStandardDirs::NoDuplicates);
    return KPluginInfo::fromFiles(themes);
}

void ThemePrivate::settingsFileChanged(const QString &file)
{
    if (file.endsWith(themeRcFile)) {
        config().config()->reparseConfiguration();
        q->settingsChanged();
    }
}

void Theme::settingsChanged()
{
    KConfigGroup cg = d->config();
    d->setThemeName(cg.readEntry("name", ThemePrivate::defaultTheme), false);
    cg = KConfigGroup(cg.config(), "PlasmaToolTips");
    d->toolTipDelay = cg.readEntry("Delay", 700);
}

void Theme::setThemeName(const QString &themeName)
{
    d->setThemeName(themeName, true);
}

void ThemePrivate::processWallpaperSettings(KConfigBase *metadata)
{
    if (!defaultWallpaperTheme.isEmpty() && defaultWallpaperTheme != DEFAULT_WALLPAPER_THEME) {
        return;
    }

    KConfigGroup cg;
    if (metadata->hasGroup("Wallpaper")) {
        // we have a theme color config, so let's also check to see if
        // there is a wallpaper defined in there.
        cg = KConfigGroup(metadata, "Wallpaper");
    } else {
        // since we didn't find an entry in the theme, let's look in the main
        // theme config
        cg = config();
    }

    defaultWallpaperTheme = cg.readEntry("defaultWallpaperTheme", DEFAULT_WALLPAPER_THEME);
    defaultWallpaperSuffix = cg.readEntry("defaultFileSuffix", DEFAULT_WALLPAPER_SUFFIX);
    defaultWallpaperWidth = cg.readEntry("defaultWidth", DEFAULT_WALLPAPER_WIDTH);
    defaultWallpaperHeight = cg.readEntry("defaultHeight", DEFAULT_WALLPAPER_HEIGHT);
}

void ThemePrivate::processAnimationSettings(const QString &theme, KConfigBase *metadata)
{
    KConfigGroup cg(metadata, "Animations");
    const QString animDir = QLatin1Literal("desktoptheme/") % theme % QLatin1Literal("/animations/");
    foreach (const QString &path, cg.keyList()) {
        const QStringList anims = cg.readEntry(path, QStringList());
        foreach (const QString &anim, anims) {
            if (!animationMapping.contains(anim)) {
                kDebug() << "Registering animation. animDir: " << animDir
                         << "\tanim: " << anim
                         << "\tpath: " << path << "\t*******\n\n\n";
                //key: desktoptheme/default/animations/+ all.js
                //value: ZoomAnimation
                animationMapping.insert(anim, animDir % path);
            } else {
                kDebug() << "************Animation already registered!\n\n\n";
            }
        }
    }

}

void ThemePrivate::setThemeName(const QString &tempThemeName, bool writeSettings)
{
    //kDebug() << tempThemeName;
    QString theme = tempThemeName;
    if (theme.isEmpty() || theme == themeName) {
        // let's try and get the default theme at least
        if (themeName.isEmpty()) {
            theme = ThemePrivate::defaultTheme;
        } else {
            return;
        }
    }

    // we have one special theme: essentially a dummy theme used to cache things with
    // the system colors.
    bool realTheme = theme != systemColorsTheme;
    if (realTheme) {
        QString themePath = KStandardDirs::locate("data", QLatin1Literal("desktoptheme/") % theme % QLatin1Char('/'));
        if (themePath.isEmpty() && themeName.isEmpty()) {
            themePath = KStandardDirs::locate("data", "desktoptheme/default/");

            if (themePath.isEmpty()) {
                return;
            }

            theme = ThemePrivate::defaultTheme;
        }
    }

    // check again as ThemePrivate::defaultTheme might be empty
    if (themeName == theme) {
        return;
    }

    themeName = theme;

    // load the color scheme config
    const QString colorsFile = realTheme ? KStandardDirs::locate("data", QLatin1Literal("desktoptheme/") % theme % QLatin1Literal("/colors"))
                                         : QString();

    //kDebug() << "we're going for..." << colorsFile << "*******************";

    // load the wallpaper settings, if any
    if (realTheme) {
        const QString metadataPath(KStandardDirs::locate("data", QLatin1Literal("desktoptheme/") % theme % QLatin1Literal("/metadata.desktop")));
        KConfig metadata(metadataPath);

        processWallpaperSettings(&metadata);

        AnimationScriptEngine::clearAnimations();
        animationMapping.clear();
        processAnimationSettings(themeName, &metadata);

        KConfigGroup cg(&metadata, "Settings");
        useNativeWidgetStyle = cg.readEntry("UseNativeWidgetStyle", false);
        QString fallback = cg.readEntry("FallbackTheme", QString());

        fallbackThemes.clear();
        while (!fallback.isEmpty() && !fallbackThemes.contains(fallback)) {
            fallbackThemes.append(fallback);

            QString metadataPath(KStandardDirs::locate("data", QLatin1Literal("desktoptheme/") % theme % QLatin1Literal("/metadata.desktop")));
            KConfig metadata(metadataPath);
            KConfigGroup cg(&metadata, "Settings");
            fallback = cg.readEntry("FallbackTheme", QString());
        }

        if (!fallbackThemes.contains("oxygen")) {
            fallbackThemes.append("oxygen");
        }

        if (!fallbackThemes.contains(ThemePrivate::defaultTheme)) {
            fallbackThemes.append(ThemePrivate::defaultTheme);
        }

        foreach (const QString &theme, fallbackThemes) {
            QString metadataPath(KStandardDirs::locate("data", QLatin1Literal("desktoptheme/") % theme % QLatin1Literal("/metadata.desktop")));
            KConfig metadata(metadataPath);
            processAnimationSettings(theme, &metadata);
            processWallpaperSettings(&metadata);
        }
    }

    if (colorsFile.isEmpty()) {
        colors = 0;
        QObject::connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
                         q, SLOT(colorsChanged()), Qt::UniqueConnection);
    } else {
        QObject::disconnect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
                            q, SLOT(colorsChanged()));
        colors = KSharedConfig::openConfig(colorsFile);
    }

    colorScheme = KColorScheme(QPalette::Active, KColorScheme::Window, colors);
    buttonColorScheme = KColorScheme(QPalette::Active, KColorScheme::Button, colors);
    viewColorScheme = KColorScheme(QPalette::Active, KColorScheme::View, colors);
    hasWallpapers = KStandardDirs::exists(KStandardDirs::locateLocal("data", QLatin1Literal("desktoptheme/") % theme % QLatin1Literal("/wallpapers/")));

    if (realTheme && isDefault && writeSettings) {
        // we're the default theme, let's save our state
        KConfigGroup &cg = config();
        if (ThemePrivate::defaultTheme == themeName) {
            cg.deleteEntry("name");
        } else {
            cg.writeEntry("name", themeName);
        }
        cg.sync();
    }

    scheduleThemeChangeNotification(SvgElementsCache);
}

QString Theme::themeName() const
{
    return d->themeName;
}

QString Theme::imagePath(const QString &name) const
{
    // look for a compressed svg file in the theme
    if (name.contains("../") || name.isEmpty()) {
        // we don't support relative paths
        //kDebug() << "Theme says: bad image path " << name;
        return QString();
    }

    const QString svgzName = name % QLatin1Literal(".svgz");
    QString path = d->findInTheme(svgzName, d->themeName);

    if (path.isEmpty()) {
        // try for an uncompressed svg file
        const QString svgName = name % QLatin1Literal(".svg");
        path = d->findInTheme(svgName, d->themeName);

        // search in fallback themes if necessary
        for (int i = 0; path.isEmpty() && i < d->fallbackThemes.count(); ++i) {
            if (d->themeName == d->fallbackThemes[i]) {
                continue;
            }

            // try a compressed svg file in the fallback theme
            path = d->findInTheme(svgzName, d->fallbackThemes[i]);

            if (path.isEmpty()) {
                // try an uncompressed svg file in the fallback theme
                path = d->findInTheme(svgName, d->fallbackThemes[i]);
            }
        }
    }

    /*
    if (path.isEmpty()) {
        kDebug() << "Theme says: bad image path " << name;
    }
    */

    return path;
}

QString Theme::styleSheet(const QString &css) const
{
    return d->processStyleSheet(css);
}

QString Theme::animationPath(const QString &name) const
{
    const QString path = d->animationMapping.value(name);
    if (path.isEmpty()) {
        //kError() << "****** FAILED TO FIND IN MAPPING!";
        return path;
    }

    return KStandardDirs::locate("data", path);
}

QString Theme::wallpaperPath(const QSize &size) const
{
    QString fullPath;
    QString image = d->defaultWallpaperTheme;

    image.append("/contents/images/%1x%2").append(d->defaultWallpaperSuffix);
    QString defaultImage = image.arg(d->defaultWallpaperWidth).arg(d->defaultWallpaperHeight);

    if (size.isValid()) {
        // try to customize the paper to the size requested
        //TODO: this should do better than just fallback to the default size.
        //      a "best fit" matching would be far better, so we don't end
        //      up returning a 1920x1200 wallpaper for a 640x480 request ;)
        image = image.arg(size.width()).arg(size.height());
    } else {
        image = defaultImage;
    }

    //TODO: the theme's wallpaper overrides regularly installed wallpapers.
    //      should it be possible for user installed (e.g. locateLocal) wallpapers
    //      to override the theme?
    if (d->hasWallpapers) {
        // check in the theme first
        fullPath = d->findInTheme(QLatin1Literal("wallpapers/") % image, d->themeName);

        if (fullPath.isEmpty()) {
            fullPath = d->findInTheme(QLatin1Literal("wallpapers/") % defaultImage, d->themeName);
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

bool Theme::currentThemeHasImage(const QString &name) const
{
    if (name.contains("../")) {
        // we don't support relative paths
        return false;
    }

    return !(d->findInTheme(name % QLatin1Literal(".svgz"), d->themeName, false).isEmpty()) ||
           !(d->findInTheme(name % QLatin1Literal(".svg"), d->themeName, false).isEmpty());
}

KSharedConfigPtr Theme::colorScheme() const
{
    return d->colors;
}

QColor Theme::color(ColorRole role) const
{
    switch (role) {
        case TextColor:
            return d->colorScheme.foreground(KColorScheme::NormalText).color();

        case HighlightColor:
            return d->colorScheme.decoration(KColorScheme::HoverColor).color();

        case BackgroundColor:
            return d->colorScheme.background(KColorScheme::NormalBackground).color();

        case ButtonTextColor:
            return d->buttonColorScheme.foreground(KColorScheme::NormalText).color();

        case ButtonBackgroundColor:
            return d->buttonColorScheme.background(KColorScheme::NormalBackground).color();

        case ButtonHoverColor:
            return d->buttonColorScheme.decoration(KColorScheme::HoverColor).color();

        case ButtonFocusColor:
            return d->buttonColorScheme.decoration(KColorScheme::FocusColor).color();

        case ViewTextColor:
            return d->viewColorScheme.foreground(KColorScheme::NormalText).color();

        case ViewBackgroundColor:
            return d->viewColorScheme.background(KColorScheme::NormalBackground).color();

        case ViewHoverColor:
            return d->viewColorScheme.decoration(KColorScheme::HoverColor).color();

        case ViewFocusColor:
            return d->viewColorScheme.decoration(KColorScheme::FocusColor).color();

        case LinkColor:
            return d->viewColorScheme.foreground(KColorScheme::LinkText).color();

        case VisitedLinkColor:
            return d->viewColorScheme.foreground(KColorScheme::VisitedText).color();
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
    switch (role) {
    case DesktopFont: {
        KConfigGroup cg(KGlobal::config(), "General");
        return cg.readEntry("desktopFont", d->generalFont);
        }
        break;

    case DefaultFont:
    default:
        return d->generalFont;
        break;

    case SmallestFont:
        return KGlobalSettings::smallestReadableFont();
        break;
    }

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
    d->themeName.clear();
    settingsChanged();
}

bool Theme::useGlobalSettings() const
{
    return d->useGlobal;
}

bool Theme::useNativeWidgetStyle() const
{
    return d->useNativeWidgetStyle;
}

bool Theme::findInCache(const QString &key, QPixmap &pix)
{
    if (d->useCache()) {
        const QString id = d->keysToCache.value(key);
        if (d->pixmapsToCache.contains(id)) {
            pix = d->pixmapsToCache.value(id);
            return !pix.isNull();
        }

        QPixmap temp;
        if (d->pixmapCache->findPixmap(key, &temp) && !temp.isNull()) {
            pix = temp;
            return true;
        }
    }

    return false;
}

// BIC FIXME: Should be merged with the other findInCache method above when we break BC
bool Theme::findInCache(const QString &key, QPixmap &pix, unsigned int lastModified)
{
    if (d->useCache() && lastModified > uint(d->pixmapCache->lastModifiedTime())) {
        return false;
    }

    return findInCache(key, pix);
}

void Theme::insertIntoCache(const QString& key, const QPixmap& pix)
{
    if (d->useCache()) {
        d->pixmapCache->insertPixmap(key, pix);
    }
}

void Theme::insertIntoCache(const QString& key, const QPixmap& pix, const QString& id)
{
    if (d->useCache()) {
        d->pixmapsToCache.insert(id, pix);

        if (d->idsToCache.contains(id)) {
            d->keysToCache.remove(d->idsToCache[id]);
        }

        d->keysToCache.insert(key, id);
        d->idsToCache.insert(id, key);
        d->saveTimer->start();
    }
}

bool Theme::findInRectsCache(const QString &image, const QString &element, QRectF &rect) const
{
    if (!d->svgElementsCache) {
        return false;
    }

    KConfigGroup imageGroup(d->svgElementsCache, image);
    rect = imageGroup.readEntry(element % QLatin1Literal("Size"), QRectF());

    if (rect.isValid()) {
        return true;
    }

    //Name starting by _ means the element is empty and we're asked for the size of
    //the whole image, so the whole image is never invalid
    if (element.indexOf('_') <= 0) {
        return false;
    }

    bool invalid = false;

    QHash<QString, QSet<QString> >::iterator it = d->invalidElements.find(image);
    if (it == d->invalidElements.end()) {
        QSet<QString> elements = imageGroup.readEntry("invalidElements", QStringList()).toSet();
        d->invalidElements.insert(image, elements);
        invalid = elements.contains(element);
    } else {
        invalid = it.value().contains(element);
    }

    return invalid;
}

QStringList Theme::listCachedRectKeys(const QString &image) const
{
    if (!d->svgElementsCache) {
        return QStringList();
    }

    KConfigGroup imageGroup(d->svgElementsCache, image);
    QStringList keys = imageGroup.keyList();

    QMutableListIterator<QString> i(keys);
    while (i.hasNext()) {
        QString key = i.next();
        if (key.endsWith("Size")) {
            // The actual cache id used from outside doesn't end on "Size".
            key.resize(key.size() - 4);
            i.setValue(key);
        } else {
            i.remove();
        }
    }
    return keys;
}

void Theme::insertIntoRectsCache(const QString& image, const QString &element, const QRectF &rect)
{
    if (!d->svgElementsCache) {
        return;
    }

    if (rect.isValid()) {
        KConfigGroup imageGroup(d->svgElementsCache, image);
        imageGroup.writeEntry(element % QLatin1Literal("Size"), rect);
    } else {
        QHash<QString, QSet<QString> >::iterator it = d->invalidElements.find(image);
        if (it == d->invalidElements.end()) {
            d->invalidElements[image].insert(element);
        } else if (!it.value().contains(element)) {
            if (it.value().count() > 1000) {
                it.value().erase(it.value().begin());
            }

            it.value().insert(element);
        }
    }
}

void Theme::invalidateRectsCache(const QString& image)
{
    if (d->svgElementsCache) {
        KConfigGroup imageGroup(d->svgElementsCache, image);
        imageGroup.deleteGroup();
    }

    d->invalidElements.remove(image);
}

void Theme::releaseRectsCache(const QString &image)
{
    QHash<QString, QSet<QString> >::iterator it = d->invalidElements.find(image);
    if (it != d->invalidElements.end()) {
        if (!d->svgElementsCache) {
            KConfigGroup imageGroup(d->svgElementsCache, it.key());
            imageGroup.writeEntry("invalidElements", it.value().toList());
        }

        d->invalidElements.erase(it);
    }
}

void Theme::setCacheLimit(int kbytes)
{
    Q_UNUSED(kbytes)
    if (d->useCache()) {
        ;
        // Too late for you bub.
        // d->pixmapCache->setCacheLimit(kbytes);
    }
}

KUrl Theme::homepage() const
{
    const QString metadataPath(KStandardDirs::locate("data", QLatin1Literal("desktoptheme/") % d->themeName % QLatin1Literal("/metadata.desktop")));
    KConfig metadata(metadataPath);
    KConfigGroup brandConfig(&metadata, "Branding");
    return brandConfig.readEntry("homepage", KUrl("http://www.kde.org"));
}

int Theme::toolTipDelay() const
{
    return d->toolTipDelay;
}

}

#include <theme.moc>
