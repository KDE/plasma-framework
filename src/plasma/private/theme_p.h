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

#ifndef PLASMA_THEME_P_H
#define PLASMA_THEME_P_H

#include <QHash>

#include <kdebug.h>
#include <KColorScheme>
#include <KImageCache>
#include <KWindowSystem>
#include <QTimer>

#if HAVE_X11
#include "private/effectwatcher_p.h"
#endif

#include "libplasma-theme-global.h"

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
          cacheSize(0),
          cachesToDiscard(NoCache),
          locolor(false),
          compositingActive(KWindowSystem::self()->compositingActive()),
          blurActive(false),
          isDefault(false),
          useGlobal(true),
          hasWallpapers(false)
    {
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
#if HAVE_X11
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
                QString app = QCoreApplication::applicationName();

                if (!app.isEmpty()) {
#ifndef NDEBUG
                    kDebug() << "using theme for app" << app;
#endif
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
    void settingsChanged();
    void blurBehindChanged(bool blur);
    bool useCache();
    void settingsFileChanged(const QString &);
    void setThemeName(const QString &themeName, bool writeSettings);
    void onAppExitCleanup();
    void processWallpaperSettings(KConfigBase *metadata);

    const QString processStyleSheet(const QString &css);

    static const char *defaultTheme;
    static const char *systemColorsTheme;
    static const char *themeRcFile;
#if HAVE_X11
    static EffectWatcher *s_blurEffectWatcher;
#endif

    Theme *q;
    QString themeName;
    KPluginInfo pluginInfo;
    QList<QString> fallbackThemes;
    KSharedConfigPtr colors;
    KColorScheme colorScheme;
    KColorScheme buttonColorScheme;
    KColorScheme viewColorScheme;
    KConfigGroup cfg;
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
    QHash<styles, QString> cachedStyleSheets;
    QHash<QString, QString> discoveries;
    QTimer *saveTimer;
    QTimer *updateNotificationTimer;
    unsigned cacheSize;
    CacheTypes cachesToDiscard;

    bool locolor : 1;
    bool compositingActive : 1;
    bool blurActive : 1;
    bool isDefault : 1;
    bool useGlobal : 1;
    bool hasWallpapers : 1;
    bool cacheTheme : 1;
};

}

#endif
