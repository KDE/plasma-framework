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

#include "theme.h"
#include <QHash>

#include <QDebug>
#include <kcolorscheme.h>
#include <kimagecache.h>
#include <kshareddatacache.h>
#include <kwindowsystem.h>
#include <QTimer>

#if HAVE_X11
#include "private/effectwatcher_p.h"
#endif

#include "libplasma-theme-global.h"

namespace Plasma
{

class Theme;

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
    
class ThemePrivate : public QObject
{
    Q_OBJECT

public:
    ThemePrivate(QObject *parent = 0);
    ~ThemePrivate();

    KConfigGroup &config();

    QString findInTheme(const QString &image, const QString &theme, bool cache = true);
    void discardCache(CacheTypes caches);
    void scheduleThemeChangeNotification(CacheTypes caches);
    bool useCache();
    void setThemeName(const QString &themeName, bool writeSettings);
    void processWallpaperSettings(KConfigBase *metadata);
    void updateSpacing();

    const QString processStyleSheet(const QString &css);
    const QString svgStyleSheet();
    QColor color(Theme::ColorRole role) const;

public Q_SLOTS:
    void compositingChanged(bool active);
    void colorsChanged();
    void blurBehindChanged(bool blur);
    void settingsFileChanged(const QString &settings);
    void scheduledCacheUpdate();
    void onAppExitCleanup();
    void notifyOfChanged();
    void settingsChanged();

Q_SIGNALS:
    void themeChanged();
    void defaultFontChanged();
    void smallestFontChanged();

public:
    static const char *defaultTheme;
    static const char *systemColorsTheme;
    static const char *themeRcFile;
#if HAVE_X11
    static EffectWatcher *s_blurEffectWatcher;
#endif
//Ref counting of ThemePrivate instances
    static ThemePrivate *globalTheme;
    static QAtomicInt globalThemeRefCount;
    static QHash<QString, ThemePrivate *> themes;
    static QHash<QString, QAtomicInt> themesRefCount;

    QString themeName;
    KPluginInfo pluginInfo;
    QList<QString> fallbackThemes;
    KSharedConfigPtr colors;
    KColorScheme colorScheme;
    KColorScheme buttonColorScheme;
    KColorScheme viewColorScheme;
    bool eventFilter(QObject *watched, QEvent *event);
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

    int defaultIconSize;
    int smallSpacing;
    int largeSpacing;

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

extern const QString s;
