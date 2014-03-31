/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2013 Marco Martin <mart@kde.org>
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

#include "theme_p.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>

#include <kdirwatch.h>
#include <kiconloader.h>
#include <kwindoweffects.h>

namespace Plasma
{

const char *ThemePrivate::defaultTheme = "default";
const char *ThemePrivate::themeRcFile = "plasmarc";
// the system colors theme is used to cache unthemed svgs with colorization needs
// these svgs do not follow the theme's colors, but rather the system colors
const char *ThemePrivate::systemColorsTheme = "internal-system-colors";
#if HAVE_X11
EffectWatcher *ThemePrivate::s_blurEffectWatcher = 0;
#endif

ThemePrivate *ThemePrivate::globalTheme = 0;
QAtomicInt ThemePrivate::globalThemeRefCount = QAtomicInt();
QHash<QString, ThemePrivate *> ThemePrivate::themes = QHash<QString, ThemePrivate*>();
QHash<QString, QAtomicInt> ThemePrivate::themesRefCount = QHash<QString, QAtomicInt>();


ThemePrivate::ThemePrivate(QObject *parent)
    : QObject(parent),
      colorScheme(QPalette::Active, KColorScheme::Window, KSharedConfigPtr(0)),
      buttonColorScheme(QPalette::Active, KColorScheme::Button, KSharedConfigPtr(0)),
      viewColorScheme(QPalette::Active, KColorScheme::View, KSharedConfigPtr(0)),
      defaultWallpaperTheme(DEFAULT_WALLPAPER_THEME),
      defaultWallpaperSuffix(DEFAULT_WALLPAPER_SUFFIX),
      defaultWallpaperWidth(DEFAULT_WALLPAPER_WIDTH),
      defaultWallpaperHeight(DEFAULT_WALLPAPER_HEIGHT),
      pixmapCache(0),
      cacheSize(0),
      cachesToDiscard(NoCache),
      locolor(false),
      compositingActive(KWindowSystem::self()->compositingActive()),
      blurActive(false),
      isDefault(true),
      useGlobal(true),
      hasWallpapers(false),
      backgroundContrastEnabled(true)
{
    ThemeConfig config;
    cacheTheme = config.cacheTheme();

    saveTimer = new QTimer(this);
    saveTimer->setSingleShot(true);
    saveTimer->setInterval(600);
    QObject::connect(saveTimer, SIGNAL(timeout()), this, SLOT(scheduledCacheUpdate()));

    updateNotificationTimer = new QTimer(this);
    updateNotificationTimer->setSingleShot(true);
    updateNotificationTimer->setInterval(100);
    QObject::connect(updateNotificationTimer, SIGNAL(timeout()), this, SLOT(notifyOfChanged()));

    if (QPixmap::defaultDepth() > 8) {
#if HAVE_X11
        //watch for blur effect property changes as well
        if (!s_blurEffectWatcher) {
            s_blurEffectWatcher = new EffectWatcher("_KDE_NET_WM_BLUR_BEHIND_REGION");
        }

        QObject::connect(s_blurEffectWatcher, SIGNAL(effectChanged(bool)), this, SLOT(blurBehindChanged(bool)));
#endif
    }
    qApp->installEventFilter(this);

    const QString configFile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + themeRcFile;
    KDirWatch::self()->addFile(configFile);

    // Catch both, direct changes to the config file ...
    connect(KDirWatch::self(), &KDirWatch::dirty, this, &ThemePrivate::settingsFileChanged);
    // ... but also remove/recreate cycles, like KConfig does it
    connect(KDirWatch::self(), &KDirWatch::created, this, &ThemePrivate::settingsFileChanged);

    connect(KWindowSystem::self(), &KWindowSystem::compositingChanged, this, &ThemePrivate::compositingChanged);
}

ThemePrivate::~ThemePrivate()
{
    delete pixmapCache;
}

KConfigGroup &ThemePrivate::config()
    {
        if (!cfg.isValid()) {
            QString groupName = QStringLiteral("Theme");

            if (!useGlobal) {
                QString app = QCoreApplication::applicationName();

                if (!app.isEmpty()) {
#ifndef NDEBUG
                    // qDebug() << "using theme for app" << app;
#endif
                    groupName.append("-").append(app);
                }
            }
            cfg = KConfigGroup(KSharedConfig::openConfig(themeRcFile), groupName);
        }

        return cfg;
    }


bool ThemePrivate::useCache()
{
    bool cachesTooOld = false;

    if (cacheTheme && !pixmapCache) {
        if (cacheSize == 0) {
            ThemeConfig config;
            cacheSize = config.themeCacheKb();
        }
        const bool isRegularTheme = themeName != systemColorsTheme;
        QString cacheFile = QStringLiteral("plasma_theme_") + themeName;

        // clear any cached values from the previous theme cache
        themeVersion.clear();

        if (!themeMetadataPath.isEmpty()) {
            KDirWatch::self()->removeFile(themeMetadataPath);
        }
        themeMetadataPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1Literal("desktoptheme/") % themeName % QLatin1Literal("/metadata.desktop"));


        if (isRegularTheme) {
            const QString cacheFileBase = cacheFile + QStringLiteral("*.kcache");

            QString currentCacheFileName;
            if (!themeMetadataPath.isEmpty()) {
                // now we record the theme version, if we can
                const KPluginInfo pluginInfo(themeMetadataPath);
                themeVersion = pluginInfo.version();
                if (!themeVersion.isEmpty()) {
                    cacheFile += "_v" + themeVersion;
                    currentCacheFileName = cacheFile + QStringLiteral(".kcache");
                }

                // watch the metadata file for changes at runtime
                KDirWatch::self()->addFile(themeMetadataPath);
                QObject::connect(KDirWatch::self(), SIGNAL(created(QString)),
                                 this, SLOT(settingsFileChanged(QString)),
                                 Qt::UniqueConnection);
                QObject::connect(KDirWatch::self(), SIGNAL(dirty(QString)),
                                 this, SLOT(settingsFileChanged(QString)),
                                 Qt::UniqueConnection);
            }

            // now we check for, and remove if necessary, old caches
            foreach (const QString &file, QStandardPaths::locateAll(QStandardPaths::CacheLocation, cacheFileBase)) {
                if (currentCacheFileName.isEmpty() ||
                    !file.endsWith(currentCacheFileName)) {
                    QFile::remove(file);
                }
            }

        }

        // now we do a sanity check: if the metadata.desktop file is newer than the cache, drop the cache
        if (isRegularTheme && !themeMetadataPath.isEmpty()) {
            // now we check to see if the theme metadata file itself is newer than the pixmap cache
            // this is done before creating the pixmapCache object since that can change the mtime
            // on the cache file

            // FIXME: when using the system colors, if they change while the application is not running
            // the cache should be dropped; we need a way to detect system color change when the
            // application is not running.
            // check for expired cache
            const QString cacheFilePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + '/' + cacheFile;
            if (!cacheFilePath.isEmpty()) {
                const QFileInfo cacheFileInfo(cacheFilePath);
                const QFileInfo metadataFileInfo(themeMetadataPath);
                cachesTooOld = cacheFileInfo.lastModified().toTime_t() > metadataFileInfo.lastModified().toTime_t();
            }
        }

        ThemeConfig config;
        pixmapCache = new KImageCache(cacheFile, config.themeCacheKb() * 1024);

        if (cachesTooOld) {
            discardCache(PixmapCache | SvgElementsCache);
        }
    }

    if (cacheTheme && !svgElementsCache) {
        const QString svgElementsFileNameBase = QStringLiteral("plasma-svgelements-") + themeName;
        QString svgElementsFileName = svgElementsFileNameBase;
        if (!themeVersion.isEmpty()) {
            svgElementsFileName += "_v" + themeVersion;
        }

        // now we check for (and remove) old caches
        foreach (const QString &file, QStandardPaths::locateAll(QStandardPaths::CacheLocation, svgElementsFileNameBase + QLatin1Char('*'))) {
            if (cachesTooOld || !file.endsWith(svgElementsFileName)) {
                QFile::remove(file);
            }
        }

        const QString svgElementsFile = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + '/' + svgElementsFileName;
        svgElementsCache = KSharedConfig::openConfig(svgElementsFile);
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
    if (cache) {
        auto it = discoveries.constFind(image);
        if (it != discoveries.constEnd()) {
            return it.value();
        }
    }

    QString search;

    if (locolor) {
        search = QLatin1Literal("desktoptheme/") % theme % QLatin1Literal("/locolor/") % image;
        search = QStandardPaths::locate(QStandardPaths::GenericDataLocation, search);
    } else if (!compositingActive) {
        search = QLatin1Literal("desktoptheme/") % theme % QLatin1Literal("/opaque/") % image;
        search =  QStandardPaths::locate(QStandardPaths::GenericDataLocation, search);
    } else if (KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind)) {
        search = QLatin1Literal("desktoptheme/") % theme % QLatin1Literal("/translucent/") % image;
        search = QStandardPaths::locate(QStandardPaths::GenericDataLocation, search);
    }

    //not found or compositing enabled
    if (search.isEmpty()) {
        search = QLatin1Literal("desktoptheme/") % theme % QLatin1Char('/') % image;
        search = QStandardPaths::locate(QStandardPaths::GenericDataLocation, search);
    }

    if (cache && !search.isEmpty()) {
        discoveries.insert(image, search);
    }

    return search;
}

void ThemePrivate::compositingChanged(bool active)
{
#if HAVE_X11
    if (compositingActive != active) {
        compositingActive = active;
        //qDebug() << QTime::currentTime();
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

        svgElementsCache = 0;
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
    //qDebug() << cachesToDiscard;
    discardCache(cachesToDiscard);
    cachesToDiscard = NoCache;
    emit themeChanged();
}

const QString ThemePrivate::processStyleSheet(const QString &css)
{
    QString stylesheet;
    if (css.isEmpty()) {
        stylesheet = cachedStyleSheets.value(DEFAULTSTYLE);
        if (stylesheet.isEmpty()) {
            stylesheet = QStringLiteral("\n\
                        body {\n\
                            color: %textcolor;\n\
                            generalfont-size: %fontsize;\n\
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
    } else {
        stylesheet = css;
    }

    QHash<QString, QString> elements;
    // If you add elements here, make sure their names are sufficiently unique to not cause
    // clashes between element keys
    elements[QStringLiteral("%textcolor")] = color(Theme::TextColor).name();
    elements[QStringLiteral("%backgroundcolor")] = color(Theme::BackgroundColor).name();
    elements[QStringLiteral("%highlightcolor")] = color(Theme::HighlightColor).name();
    elements[QStringLiteral("%visitedlink")] = color(Theme::VisitedLinkColor).name();
    elements[QStringLiteral("%activatedlink")] = color(Theme::HighlightColor).name();
    elements[QStringLiteral("%hoveredlink")] = color(Theme::HighlightColor).name();
    elements[QStringLiteral("%link")] = color(Theme::LinkColor).name();
    elements[QStringLiteral("%buttontextcolor")] = color(Theme::ButtonTextColor).name();
    elements[QStringLiteral("%buttonbackgroundcolor")] = color(Theme::ButtonBackgroundColor).name();
    elements[QStringLiteral("%buttonhovercolor")] = color(Theme::ButtonHoverColor).name();
    elements[QStringLiteral("%buttonfocuscolor")] = color(Theme::ButtonFocusColor).name();
    elements[QStringLiteral("%viewtextcolor")] = color(Theme::ViewTextColor).name();
    elements[QStringLiteral("%viewbackgroundcolor")] = color(Theme::ViewBackgroundColor).name();
    elements[QStringLiteral("%viewhovercolor")] = color(Theme::ViewHoverColor).name();
    elements[QStringLiteral("%viewfocuscolor")] = color(Theme::ViewFocusColor).name();

    QFont font = QApplication::font();
    elements[QStringLiteral("%fontsize")] = QStringLiteral("%1pt").arg(font.pointSize());
    elements[QStringLiteral("%fontfamily")] = font.family().split('[').first();
    elements[QStringLiteral("%smallfontsize")] = QString("%1pt").arg(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont).pointSize());

    QHash<QString, QString>::const_iterator it = elements.constBegin();
    QHash<QString, QString>::const_iterator itEnd = elements.constEnd();
    for ( ; it != itEnd; ++it) {
        stylesheet.replace(it.key(), it.value());
    }
    return stylesheet;
}

const QString ThemePrivate::svgStyleSheet()
{
    QString stylesheet = cachedStyleSheets.value(SVGSTYLE);
    if (stylesheet.isEmpty()) {
        QString skel = QStringLiteral(".ColorScheme-%1{color:%2;}");

        stylesheet += skel.arg(QStringLiteral("Text"), QStringLiteral("%textcolor"));
        stylesheet += skel.arg(QStringLiteral("Background"), QStringLiteral("%backgroundcolor"));

        stylesheet += skel.arg(QStringLiteral("Highlight"), QStringLiteral("%highlightcolor"));

        stylesheet += skel.arg(QStringLiteral("ButtonText"), QStringLiteral("%buttontextcolor"));
        stylesheet += skel.arg(QStringLiteral("ButtonBackground"), QStringLiteral("%buttonbackgroundcolor"));
        stylesheet += skel.arg(QStringLiteral("ButtonHover"), QStringLiteral("%buttonhovercolor"));
        stylesheet += skel.arg(QStringLiteral("ButtonFocus"), QStringLiteral("%buttonfocuscolor"));

        stylesheet += skel.arg(QStringLiteral("ViewText"), QStringLiteral("%viewtextcolor"));
        stylesheet += skel.arg(QStringLiteral("ViewBackground"), QStringLiteral("%viewbackgroundcolor"));
        stylesheet += skel.arg(QStringLiteral("ViewHover"), QStringLiteral("%viewhovercolor"));
        stylesheet += skel.arg(QStringLiteral("ViewFocus"), QStringLiteral("%viewfocuscolor"));

        stylesheet = processStyleSheet(stylesheet);
        cachedStyleSheets.insert(SVGSTYLE, stylesheet);
    }

    return stylesheet;
}

void ThemePrivate::settingsFileChanged(const QString &file)
{
    qDebug() << "settingsFile: " << file;
    if (file == themeMetadataPath) {
        const KPluginInfo pluginInfo(themeMetadataPath);
        if (themeVersion != pluginInfo.version()) {
            scheduleThemeChangeNotification(SvgElementsCache);
        }
    } else if (file.endsWith(themeRcFile)) {
        config().config()->reparseConfiguration();
        settingsChanged();
    }
}

void ThemePrivate::settingsChanged()
{
    //qDebug() << "Settings Changed!";
    KConfigGroup cg = config();
    setThemeName(cg.readEntry("name", ThemePrivate::defaultTheme), false);
}

QColor ThemePrivate::color(Theme::ColorRole role) const
{
    switch (role) {
        case Theme::TextColor:
            return colorScheme.foreground(KColorScheme::NormalText).color();

        case Theme::HighlightColor:
            return colorScheme.decoration(KColorScheme::HoverColor).color();

        case Theme::BackgroundColor:
            return colorScheme.background(KColorScheme::NormalBackground).color();

        case Theme::ButtonTextColor:
            return buttonColorScheme.foreground(KColorScheme::NormalText).color();

        case Theme::ButtonBackgroundColor:
            return buttonColorScheme.background(KColorScheme::NormalBackground).color();

        case Theme::ButtonHoverColor:
            return buttonColorScheme.decoration(KColorScheme::HoverColor).color();

        case Theme::ButtonFocusColor:
            return buttonColorScheme.decoration(KColorScheme::FocusColor).color();

        case Theme::ViewTextColor:
            return viewColorScheme.foreground(KColorScheme::NormalText).color();

        case Theme::ViewBackgroundColor:
            return viewColorScheme.background(KColorScheme::NormalBackground).color();

        case Theme::ViewHoverColor:
            return viewColorScheme.decoration(KColorScheme::HoverColor).color();

        case Theme::ViewFocusColor:
            return viewColorScheme.decoration(KColorScheme::FocusColor).color();

        case Theme::LinkColor:
            return viewColorScheme.foreground(KColorScheme::LinkText).color();

        case Theme::VisitedLinkColor:
            return viewColorScheme.foreground(KColorScheme::VisitedText).color();
    }

    return QColor();
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

void ThemePrivate::processContrastSettings(KConfigBase* metadata)
{
    KConfigGroup cg;
    if (metadata->hasGroup("ContrastEffect")) {
        cg = KConfigGroup(metadata, "ContrastEffect");
        backgroundContrastEnabled = cg.readEntry(QStringLiteral("enabled"), false);

        //if (backgroundContrastEnabled) {
            // Make up sensible default values, based on the background color
            // This works for a light theme -- lighting up the background
            qreal _contrast = 0.3;
            qreal _intensity = 1.9;
            qreal _saturation = 1.7;

            // If we're using a dark background color, darken the background
            if (qGray(color(Plasma::Theme::BackgroundColor).rgb()) < 127) {
                _contrast = 0.45;
                _intensity = 0.45;
                _saturation = 1.7;
            }
            backgroundContrast = cg.readEntry(QStringLiteral("contrast"), _contrast);
            backgroundIntensity = cg.readEntry(QStringLiteral("intensity"), _intensity);
            backgroundSaturation = cg.readEntry(QStringLiteral("saturation"), _saturation);
        //}
    } else {
        backgroundContrastEnabled = false;
    }
}

void ThemePrivate::setThemeName(const QString &tempThemeName, bool writeSettings)
{
    QString theme = tempThemeName;
    if (theme.isEmpty() || theme == themeName) {
        // let's try and get the default theme at least
        if (themeName.isEmpty()) {
            theme = ThemePrivate::defaultTheme;
        } else {
            return;
        }
    }
    qDebug() << tempThemeName;

    // we have one special theme: essentially a dummy theme used to cache things with
    // the system colors.
    bool realTheme = theme != systemColorsTheme;
    if (realTheme) {
        QString themePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1Literal("desktoptheme/") % theme % QLatin1Char('/'));
        if (themePath.isEmpty() && themeName.isEmpty()) {
            themePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("desktoptheme/default"), QStandardPaths::LocateDirectory);

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
    const QString colorsFile = realTheme ? QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1Literal("desktoptheme/") % theme % QLatin1Literal("/colors"))
                                         : QString();

    //qDebug() << "we're going for..." << colorsFile << "*******************";

    if (colorsFile.isEmpty()) {
        colors = 0;
        if (qApp) {
            qApp->installEventFilter(this);
        }
    } else {
        if (qApp) {
            qApp->removeEventFilter(this);
        }
        colors = KSharedConfig::openConfig(colorsFile);
    }

    colorScheme = KColorScheme(QPalette::Active, KColorScheme::Window, colors);
    buttonColorScheme = KColorScheme(QPalette::Active, KColorScheme::Button, colors);
    viewColorScheme = KColorScheme(QPalette::Active, KColorScheme::View, colors);
    const QString wallpaperPath = QLatin1Literal("desktoptheme/") % theme % QLatin1Literal("/wallpapers/");
    hasWallpapers = !QStandardPaths::locate(QStandardPaths::GenericDataLocation, wallpaperPath, QStandardPaths::LocateDirectory).isEmpty();

    // load the wallpaper settings, if any
    if (realTheme) {
        const QString metadataPath(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1Literal("desktoptheme/") % theme % QLatin1Literal("/metadata.desktop")));
        KConfig metadata(metadataPath);
        pluginInfo = KPluginInfo(metadataPath);

        processContrastSettings(&metadata);

        processWallpaperSettings(&metadata);

        KConfigGroup cg(&metadata, "Settings");
        QString fallback = cg.readEntry("FallbackTheme", QString());

        fallbackThemes.clear();
        while (!fallback.isEmpty() && !fallbackThemes.contains(fallback)) {
            fallbackThemes.append(fallback);

            QString metadataPath(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1Literal("desktoptheme/") % theme % QLatin1Literal("/metadata.desktop")));
            KConfig metadata(metadataPath);
            KConfigGroup cg(&metadata, "Settings");
            fallback = cg.readEntry("FallbackTheme", QString());
        }

        if (!fallbackThemes.contains(ThemePrivate::defaultTheme)) {
            fallbackThemes.append(ThemePrivate::defaultTheme);
        }

        foreach (const QString &theme, fallbackThemes) {
            QString metadataPath(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1Literal("desktoptheme/") % theme % QLatin1Literal("/metadata.desktop")));
            KConfig metadata(metadataPath);
            processWallpaperSettings(&metadata);
        }
    }

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

bool ThemePrivate::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == QCoreApplication::instance()) {
        if (event->type() == QEvent::ApplicationPaletteChange) {
            colorsChanged();
        }
        if (event->type() == QEvent::ApplicationFontChange || event->type() == QEvent::FontChange) {
            defaultFontChanged();
            smallestFontChanged();
        }
    }
    return QObject::eventFilter(watched, event);
}

}

#include "moc_theme_p.cpp"
