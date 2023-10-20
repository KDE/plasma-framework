/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "theme_p.h"
#include "debug_p.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <QGuiApplication>

#include <KDirWatch>
#include <KIconLoader>
#include <KIconTheme>
#include <KSharedConfig>
#include <KWindowEffects>
#include <KX11Extras>
#include <kpluginmetadata.h>

namespace Plasma
{
const char ThemePrivate::defaultTheme[] = "default";
const char ThemePrivate::themeRcFile[] = "plasmarc";
// the system colors theme is used to cache unthemed svgs with colorization needs
// these svgs do not follow the theme's colors, but rather the system colors
const char ThemePrivate::systemColorsTheme[] = "internal-system-colors";
#if HAVE_X11
EffectWatcher *ThemePrivate::s_backgroundContrastEffectWatcher = nullptr;
#endif

ThemePrivate *ThemePrivate::globalTheme = nullptr;
QHash<QString, ThemePrivate *> ThemePrivate::themes = QHash<QString, ThemePrivate *>();
using QSP = QStandardPaths;

KSharedConfig::Ptr configForTheme(const QString &theme)
{
    const QString baseName = QLatin1String(PLASMA_RELATIVE_DATA_INSTALL_DIR "/desktoptheme/") % theme;
    QString configPath = QSP::locate(QSP::GenericDataLocation, baseName + QLatin1String("/plasmarc"));
    if (!configPath.isEmpty()) {
        return KSharedConfig::openConfig(configPath, KConfig::SimpleConfig);
    }
    QString metadataPath = QSP::locate(QSP::GenericDataLocation, baseName + QLatin1String("/metadata.desktop"));
    return KSharedConfig::openConfig(metadataPath, KConfig::SimpleConfig);
}

KPluginMetaData metaDataForTheme(const QString &theme)
{
    const QString packageBasePath =
        QSP::locate(QSP::GenericDataLocation, QLatin1String(PLASMA_RELATIVE_DATA_INSTALL_DIR "/desktoptheme/") % theme, QSP::LocateDirectory);
    if (packageBasePath.isEmpty()) {
        qWarning(LOG_PLASMA) << "Could not locate plasma theme" << theme << "in" << PLASMA_RELATIVE_DATA_INSTALL_DIR "/desktoptheme/"
                             << "using search path" << QSP::standardLocations(QSP::GenericDataLocation);
        return {};
    }
    if (QFileInfo::exists(packageBasePath + QLatin1String("/metadata.json"))) {
        return KPluginMetaData::fromJsonFile(packageBasePath + QLatin1String("/metadata.json"));
    } else if (QFileInfo::exists(packageBasePath + QLatin1String("/metadata.desktop"))) {
        QString metadataPath = packageBasePath + QLatin1String("/metadata.desktop");
        KConfigGroup cg(KSharedConfig::openConfig(packageBasePath + QLatin1String("/metadata.desktop"), KConfig::SimpleConfig),
                        QStringLiteral("Desktop Entry"));
        QJsonObject obj = {};
        for (const QString &key : cg.keyList()) {
            obj[key] = cg.readEntry(key);
        }
        qWarning(LOG_PLASMA) << "The theme" << theme << "uses the legacy metadata.desktop. Consider contacting the author and asking them update it to use the newer JSON format.";
        return KPluginMetaData(obj, packageBasePath + QLatin1String("/metadata.desktop"));
    } else {
        qCWarning(LOG_PLASMA) << "Could not locate metadata for theme" << theme;
        return {};
    }
}

ThemePrivate::ThemePrivate(QObject *parent)
    : QObject(parent)
    , colorScheme(QPalette::Active, KColorScheme::Window, KSharedConfigPtr(nullptr))
    , selectionColorScheme(QPalette::Active, KColorScheme::Selection, KSharedConfigPtr(nullptr))
    , buttonColorScheme(QPalette::Active, KColorScheme::Button, KSharedConfigPtr(nullptr))
    , viewColorScheme(QPalette::Active, KColorScheme::View, KSharedConfigPtr(nullptr))
    , complementaryColorScheme(QPalette::Active, KColorScheme::Complementary, KSharedConfigPtr(nullptr))
    , headerColorScheme(QPalette::Active, KColorScheme::Header, KSharedConfigPtr(nullptr))
    , tooltipColorScheme(QPalette::Active, KColorScheme::Tooltip, KSharedConfigPtr(nullptr))
    , defaultWallpaperTheme(QStringLiteral(DEFAULT_WALLPAPER_THEME))
    , defaultWallpaperSuffix(QStringLiteral(DEFAULT_WALLPAPER_SUFFIX))
    , defaultWallpaperWidth(DEFAULT_WALLPAPER_WIDTH)
    , defaultWallpaperHeight(DEFAULT_WALLPAPER_HEIGHT)
    , cacheSize(0)
    , cachesToDiscard(NoCache)
    , compositingActive(KX11Extras::self()->compositingActive())
    , backgroundContrastActive(KWindowEffects::isEffectAvailable(KWindowEffects::BackgroundContrast))
    , isDefault(true)
    , useGlobal(true)
    , hasWallpapers(false)
    , fixedName(false)
    , backgroundContrast(qQNaN())
    , backgroundIntensity(qQNaN())
    , backgroundSaturation(qQNaN())
    , backgroundContrastEnabled(true)
    , adaptiveTransparencyEnabled(false)
    , blurBehindEnabled(true)
    , apiMajor(1)
    , apiMinor(0)
    , apiRevision(0)
{
    ThemeConfig config;
    cacheTheme = config.cacheTheme();
    kSvgImageSet = std::unique_ptr<KSvg::ImageSet>(new KSvg::ImageSet);
    kSvgImageSet->setBasePath(QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/desktoptheme/"));

    pixmapSaveTimer = new QTimer(this);
    pixmapSaveTimer->setSingleShot(true);
    pixmapSaveTimer->setInterval(600);
    QObject::connect(pixmapSaveTimer, &QTimer::timeout, this, &ThemePrivate::scheduledCacheUpdate);

    updateNotificationTimer = new QTimer(this);
    updateNotificationTimer->setSingleShot(true);
    updateNotificationTimer->setInterval(100);
    QObject::connect(updateNotificationTimer, &QTimer::timeout, this, &ThemePrivate::notifyOfChanged);

    if (QPixmap::defaultDepth() > 8) {
#if HAVE_X11
        // watch for background contrast effect property changes as well
        if (!s_backgroundContrastEffectWatcher) {
            s_backgroundContrastEffectWatcher = new EffectWatcher(QStringLiteral("_KDE_NET_WM_BACKGROUND_CONTRAST_REGION"));
        }

        QObject::connect(s_backgroundContrastEffectWatcher, &EffectWatcher::effectChanged, this, [this](bool active) {
            if (backgroundContrastActive != active) {
                backgroundContrastActive = active;
                scheduleThemeChangeNotification(PixmapCache | SvgElementsCache);
                kSvgImageSet->setSelectors({QStringLiteral("translucent")});
            }
        });
#endif
    }
    QCoreApplication::instance()->installEventFilter(this);

    const QString configFile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + QLatin1String(themeRcFile);
    KDirWatch::self()->addFile(configFile);

    // Catch both, direct changes to the config file ...
    connect(KDirWatch::self(), &KDirWatch::dirty, this, &ThemePrivate::settingsFileChanged);
    // ... but also remove/recreate cycles, like KConfig does it
    connect(KDirWatch::self(), &KDirWatch::created, this, &ThemePrivate::settingsFileChanged);

    QObject::connect(KIconLoader::global(), &KIconLoader::iconChanged, this, [this]() {
        scheduleThemeChangeNotification(PixmapCache | SvgElementsCache);
    });

    connect(KX11Extras::self(), &KX11Extras::compositingChanged, this, &ThemePrivate::compositingChanged);
    compositingChanged(KX11Extras::compositingActive());
}

ThemePrivate::~ThemePrivate()
{
}

KConfigGroup &ThemePrivate::config()
{
    if (!cfg.isValid()) {
        QString groupName = QStringLiteral("Theme");

        if (!useGlobal) {
            QString app = QCoreApplication::applicationName();

            if (!app.isEmpty()) {
#ifndef NDEBUG
                // qCDebug(LOG_PLASMA) << "using theme for app" << app;
#endif
                groupName.append(QLatin1Char('-')).append(app);
            }
        }
        cfg = KConfigGroup(KSharedConfig::openConfig(QFile::decodeName(themeRcFile)), groupName);
    }

    return cfg;
}

bool ThemePrivate::useCache()
{
    bool cachesTooOld = false;

    if (cacheTheme) {
        if (cacheSize == 0) {
            ThemeConfig config;
            cacheSize = config.themeCacheKb();
        }
        const bool isRegularTheme = themeName != QLatin1String(systemColorsTheme);
        QString cacheFile = QLatin1String("plasma_theme_") + themeName;

        // clear any cached values from the previous theme cache
        themeVersion.clear();

        if (!themeMetadataPath.isEmpty()) {
            KDirWatch::self()->removeFile(themeMetadataPath);
        }
        themeMetadataPath = configForTheme(themeName)->name();
        if (isRegularTheme) {
            const auto *iconTheme = KIconLoader::global()->theme();
            if (iconTheme) {
                iconThemeMetadataPath = iconTheme->dir() + QStringLiteral("index.theme");
            }

            const QString cacheFileBase = cacheFile + QLatin1String("*.kcache");

            QString currentCacheFileName;
            if (!themeMetadataPath.isEmpty()) {
                // now we record the theme version, if we can
                const KPluginMetaData data = metaDataForTheme(themeName);
                if (data.isValid()) {
                    themeVersion = data.version();
                }
                if (!themeVersion.isEmpty()) {
                    cacheFile += QLatin1String("_v") + themeVersion;
                    currentCacheFileName = cacheFile + QLatin1String(".kcache");
                }

                // watch the metadata file for changes at runtime
                KDirWatch::self()->addFile(themeMetadataPath);
                QObject::connect(KDirWatch::self(), &KDirWatch::created, this, &ThemePrivate::settingsFileChanged, Qt::UniqueConnection);
                QObject::connect(KDirWatch::self(), &KDirWatch::dirty, this, &ThemePrivate::settingsFileChanged, Qt::UniqueConnection);

                if (!iconThemeMetadataPath.isEmpty()) {
                    KDirWatch::self()->addFile(iconThemeMetadataPath);
                }
            }

            // now we check for, and remove if necessary, old caches
            QDir cacheDir(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation));
            cacheDir.setNameFilters(QStringList({cacheFileBase}));

            const auto files = cacheDir.entryInfoList();
            for (const QFileInfo &file : files) {
                if (currentCacheFileName.isEmpty() //
                    || !file.absoluteFilePath().endsWith(currentCacheFileName)) {
                    QFile::remove(file.absoluteFilePath());
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
            const QString cacheFilePath =
                QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QLatin1Char('/') + cacheFile + QLatin1String(".kcache");
            if (!cacheFilePath.isEmpty()) {
                const QFileInfo cacheFileInfo(cacheFilePath);
                const QFileInfo metadataFileInfo(themeMetadataPath);
                const QFileInfo iconThemeMetadataFileInfo(iconThemeMetadataPath);

                cachesTooOld = (cacheFileInfo.lastModified().toSecsSinceEpoch() < metadataFileInfo.lastModified().toSecsSinceEpoch())
                    || (cacheFileInfo.lastModified().toSecsSinceEpoch() < iconThemeMetadataFileInfo.lastModified().toSecsSinceEpoch());
            }
        }

        ThemeConfig config;

        if (cachesTooOld) {
            discardCache(PixmapCache | SvgElementsCache);
        }
    }

    if (cacheTheme) {
        QString currentIconThemePath;
        const auto *iconTheme = KIconLoader::global()->theme();
        if (iconTheme) {
            currentIconThemePath = iconTheme->dir();
        }
    }

    return cacheTheme;
}

void ThemePrivate::onAppExitCleanup()
{
    cacheTheme = false;
}

QString ThemePrivate::imagePath(const QString &theme, const QString &type, const QString &image)
{
    QString subdir = QLatin1String(PLASMA_RELATIVE_DATA_INSTALL_DIR "/desktoptheme/") % theme % type % image;
    return QStandardPaths::locate(QStandardPaths::GenericDataLocation, subdir);
}

QString ThemePrivate::findInTheme(const QString &image, const QString &theme, bool cache)
{
    if (cache) {
        auto it = discoveries.constFind(image);
        if (it != discoveries.constEnd()) {
            return it.value();
        }
    }

    QString type = QStringLiteral("/");
    if (!compositingActive) {
        type = QStringLiteral("/opaque/");
    } else if (backgroundContrastActive) {
        type = QStringLiteral("/translucent/");
    }

    QString search = imagePath(theme, type, image);

    // not found or compositing enabled
    if (search.isEmpty()) {
        search = imagePath(theme, QStringLiteral("/"), image);
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
        // qCDebug(LOG_PLASMA) << QTime::currentTime();
        scheduleThemeChangeNotification(PixmapCache | SvgElementsCache);
        if (active) {
            kSvgImageSet->setSelectors({});
        } else {
            kSvgImageSet->setSelectors({QStringLiteral("opaque")});
        }
    }
#endif
}

void ThemePrivate::discardCache(CacheTypes caches)
{
    if (caches & SvgElementsCache) {
        discoveries.clear();
    }
}

void ThemePrivate::scheduledCacheUpdate()
{
}

void ThemePrivate::colorsChanged()
{
    // in the case the theme follows the desktop settings, refetch the colorschemes
    // and discard the svg pixmap cache
    if (!colors) {
        KSharedConfig::openConfig()->reparseConfiguration();
    }
    colorScheme = KColorScheme(QPalette::Active, KColorScheme::Window, colors);
    buttonColorScheme = KColorScheme(QPalette::Active, KColorScheme::Button, colors);
    viewColorScheme = KColorScheme(QPalette::Active, KColorScheme::View, colors);
    selectionColorScheme = KColorScheme(QPalette::Active, KColorScheme::Selection, colors);
    complementaryColorScheme = KColorScheme(QPalette::Active, KColorScheme::Complementary, colors);
    headerColorScheme = KColorScheme(QPalette::Active, KColorScheme::Header, colors);
    tooltipColorScheme = KColorScheme(QPalette::Active, KColorScheme::Tooltip, colors);
    palette = KColorScheme::createApplicationPalette(colors);
    scheduleThemeChangeNotification(PixmapCache | SvgElementsCache);
    Q_EMIT applicationPaletteChange();
}

void ThemePrivate::scheduleThemeChangeNotification(CacheTypes caches)
{
    cachesToDiscard |= caches;
    updateNotificationTimer->start();
}

void ThemePrivate::notifyOfChanged()
{
    // qCDebug(LOG_PLASMA) << cachesToDiscard;
    discardCache(cachesToDiscard);
    cachesToDiscard = NoCache;
    Q_EMIT themeChanged();
}

void ThemePrivate::settingsFileChanged(const QString &file)
{
    qCDebug(LOG_PLASMA) << "settingsFile: " << file;
    if (file == themeMetadataPath) {
        const KPluginMetaData data = metaDataForTheme(themeName);
        if (!data.isValid() || themeVersion != data.version()) {
            scheduleThemeChangeNotification(SvgElementsCache);
        }
    } else if (file.endsWith(QLatin1String(themeRcFile))) {
        config().config()->reparseConfiguration();
        settingsChanged(true);
    }
}

void ThemePrivate::settingsChanged(bool emitChanges)
{
    if (fixedName) {
        return;
    }
    // qCDebug(LOG_PLASMA) << "Settings Changed!";
    KConfigGroup cg = config();
    setThemeName(cg.readEntry("name", ThemePrivate::defaultTheme), false, emitChanges);
}

QColor ThemePrivate::color(Theme::ColorRole role, Theme::ColorGroup group) const
{
    const KColorScheme *scheme = nullptr;

    // Before 5.0 Plasma theme really only used Normal and Button
    // many old themes are built on this assumption and will break
    // otherwise
    if (apiMajor < 5 && group != Theme::NormalColorGroup) {
        group = Theme::ButtonColorGroup;
    }

    switch (group) {
    case Theme::ButtonColorGroup: {
        scheme = &buttonColorScheme;
        break;
    }

    case Theme::ViewColorGroup: {
        scheme = &viewColorScheme;
        break;
    }

    // this doesn't have a real kcolorscheme
    case Theme::ComplementaryColorGroup: {
        scheme = &complementaryColorScheme;
        break;
    }

    case Theme::HeaderColorGroup: {
        scheme = &headerColorScheme;
        break;
    }

    case Theme::ToolTipColorGroup: {
        scheme = &tooltipColorScheme;
        break;
    }

    case Theme::NormalColorGroup:
    default: {
        scheme = &colorScheme;
        break;
    }
    }

    switch (role) {
    case Theme::TextColor:
        return scheme->foreground(KColorScheme::NormalText).color();

    case Theme::BackgroundColor:
        return scheme->background(KColorScheme::NormalBackground).color();

    case Theme::HoverColor:
        return scheme->decoration(KColorScheme::HoverColor).color();

    case Theme::HighlightColor:
        return selectionColorScheme.background(KColorScheme::NormalBackground).color();

    case Theme::FocusColor:
        return scheme->decoration(KColorScheme::FocusColor).color();

    case Theme::LinkColor:
        return scheme->foreground(KColorScheme::LinkText).color();

    case Theme::VisitedLinkColor:
        return scheme->foreground(KColorScheme::VisitedText).color();

    case Theme::HighlightedTextColor:
        return selectionColorScheme.foreground(KColorScheme::NormalText).color();

    case Theme::PositiveTextColor:
        return scheme->foreground(KColorScheme::PositiveText).color();
    case Theme::NeutralTextColor:
        return scheme->foreground(KColorScheme::NeutralText).color();
    case Theme::NegativeTextColor:
        return scheme->foreground(KColorScheme::NegativeText).color();
    case Theme::DisabledTextColor:
        return scheme->foreground(KColorScheme::InactiveText).color();
    }

    return QColor();
}

void ThemePrivate::processWallpaperSettings(const KSharedConfigPtr &metadata)
{
    if (!defaultWallpaperTheme.isEmpty() && defaultWallpaperTheme != QLatin1String(DEFAULT_WALLPAPER_THEME)) {
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

void ThemePrivate::processContrastSettings(const KSharedConfigPtr &metadata)
{
    KConfigGroup cg;
    if (metadata->hasGroup("ContrastEffect")) {
        cg = KConfigGroup(metadata, "ContrastEffect");
        backgroundContrastEnabled = cg.readEntry("enabled", false);

        backgroundContrast = cg.readEntry("contrast", qQNaN());
        backgroundIntensity = cg.readEntry("intensity", qQNaN());
        backgroundSaturation = cg.readEntry("saturation", qQNaN());
    } else {
        backgroundContrastEnabled = false;
    }
}

void ThemePrivate::processAdaptiveTransparencySettings(const KSharedConfigPtr &metadata)
{
    KConfigGroup cg;
    if (metadata->hasGroup("AdaptiveTransparency")) {
        cg = KConfigGroup(metadata, "AdaptiveTransparency");
        adaptiveTransparencyEnabled = cg.readEntry("enabled", false);
    } else {
        adaptiveTransparencyEnabled = false;
    }
}

void ThemePrivate::processBlurBehindSettings(const KSharedConfigPtr &metadata)
{
    KConfigGroup cg;
    if (metadata->hasGroup("BlurBehindEffect")) {
        cg = KConfigGroup(metadata, "BlurBehindEffect");
        blurBehindEnabled = cg.readEntry("enabled", true);
    } else {
        blurBehindEnabled = true;
    }
}

void ThemePrivate::setThemeName(const QString &tempThemeName, bool writeSettings, bool emitChanged)
{
    kSvgImageSet->setImageSetName(tempThemeName);
    QString theme = tempThemeName;
    if (theme.isEmpty() || theme == themeName) {
        // let's try and get the default theme at least
        if (themeName.isEmpty()) {
            theme = QLatin1String(ThemePrivate::defaultTheme);
        } else {
            return;
        }
    }

    // we have one special theme: essentially a dummy theme used to cache things with
    // the system colors.
    bool realTheme = theme != QLatin1String(systemColorsTheme);
    if (realTheme) {
        KPluginMetaData data = metaDataForTheme(theme);
        if (!data.isValid()) {
            data = metaDataForTheme(QStringLiteral("default"));
            if (!data.isValid()) {
                return;
            }

            theme = QLatin1String(ThemePrivate::defaultTheme);
        }
    }

    // check again as ThemePrivate::defaultTheme might be empty
    if (themeName == theme) {
        return;
    }

    themeName = theme;

    // load the color scheme config
    const QString colorsFile = realTheme
        ? QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                 QLatin1String(PLASMA_RELATIVE_DATA_INSTALL_DIR "/desktoptheme/") % theme % QLatin1String("/colors"))
        : QString();

    // qCDebug(LOG_PLASMA) << "we're going for..." << colorsFile << "*******************";

    if (colorsFile.isEmpty()) {
        colors = nullptr;
    } else {
        colors = KSharedConfig::openConfig(colorsFile);
    }

    colorScheme = KColorScheme(QPalette::Active, KColorScheme::Window, colors);
    selectionColorScheme = KColorScheme(QPalette::Active, KColorScheme::Selection, colors);
    buttonColorScheme = KColorScheme(QPalette::Active, KColorScheme::Button, colors);
    viewColorScheme = KColorScheme(QPalette::Active, KColorScheme::View, colors);
    complementaryColorScheme = KColorScheme(QPalette::Active, KColorScheme::Complementary, colors);
    headerColorScheme = KColorScheme(QPalette::Active, KColorScheme::Header, colors);
    tooltipColorScheme = KColorScheme(QPalette::Active, KColorScheme::Tooltip, colors);
    palette = KColorScheme::createApplicationPalette(colors);
    const QString wallpaperPath = QLatin1String(PLASMA_RELATIVE_DATA_INSTALL_DIR "/desktoptheme/") % theme % QLatin1String("/wallpapers/");
    hasWallpapers = !QStandardPaths::locate(QStandardPaths::GenericDataLocation, wallpaperPath, QStandardPaths::LocateDirectory).isEmpty();

    // load the wallpaper settings, if any
    if (realTheme) {
        pluginMetaData = metaDataForTheme(theme);
        KSharedConfigPtr metadata = configForTheme(theme);

        processContrastSettings(metadata);
        processBlurBehindSettings(metadata);
        processAdaptiveTransparencySettings(metadata);

        processWallpaperSettings(metadata);

        KConfigGroup cg(metadata, "Settings");
        QString fallback = cg.readEntry("FallbackTheme", QString());

        fallbackThemes.clear();
        while (!fallback.isEmpty() && !fallbackThemes.contains(fallback)) {
            fallbackThemes.append(fallback);

            KSharedConfigPtr metadata = configForTheme(fallback);
            KConfigGroup cg(metadata, "Settings");
            fallback = cg.readEntry("FallbackTheme", QString());
        }

        if (!fallbackThemes.contains(QLatin1String(ThemePrivate::defaultTheme))) {
            fallbackThemes.append(QLatin1String(ThemePrivate::defaultTheme));
        }

        for (const QString &theme : std::as_const(fallbackThemes)) {
            KSharedConfigPtr metadata = configForTheme(theme);
            processWallpaperSettings(metadata);
        }

        // Check for what Plasma version the theme has been done
        // There are some behavioral differences between KDE4 Plasma and Plasma 5
        const QString apiVersion = pluginMetaData.value(QStringLiteral("X-Plasma-API"));
        apiMajor = 1;
        apiMinor = 0;
        apiRevision = 0;
        if (!apiVersion.isEmpty()) {
            const QList<QStringView> parts = QStringView(apiVersion).split(QLatin1Char('.'));
            if (!parts.isEmpty()) {
                apiMajor = parts.value(0).toInt();
            }
            if (parts.count() > 1) {
                apiMinor = parts.value(1).toInt();
            }
            if (parts.count() > 2) {
                apiRevision = parts.value(2).toInt();
            }
        }
    }

    if (realTheme && isDefault && writeSettings) {
        // we're the default theme, let's save our status
        KConfigGroup &cg = config();
        cg.writeEntry("name", themeName);
        cg.sync();
    }

    if (emitChanged) {
        scheduleThemeChangeNotification(PixmapCache | SvgElementsCache);
    }
}

bool ThemePrivate::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == QCoreApplication::instance()) {
        if (event->type() == QEvent::ApplicationPaletteChange) {
            colorsChanged();
        }
        if (event->type() == QEvent::ApplicationFontChange || event->type() == QEvent::FontChange) {
            Q_EMIT defaultFontChanged();
            Q_EMIT smallestFontChanged();
        }
    }
    return QObject::eventFilter(watched, event);
}

}

#include "moc_theme_p.cpp"
