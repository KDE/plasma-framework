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
#include "svg.h"
#include <QHash>

#include <QDebug>
#include <kcolorscheme.h>
#include <kimagecache.h>
#include <kshareddatacache.h>
#include <kwindowsystem.h>
#include <QTimer>

#include <config-plasma.h>
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
    explicit ThemePrivate(QObject *parent = nullptr);
    ~ThemePrivate() override;

    KConfigGroup &config();

    QString imagePath(const QString &theme, const QString &type, const QString &image);
    QString findInTheme(const QString &image, const QString &theme, bool cache = true);
    void discardCache(CacheTypes caches);
    void scheduleThemeChangeNotification(CacheTypes caches);
    bool useCache();
    void setThemeName(const QString &themeName, bool writeSettings, bool emitChanged);
    void processWallpaperSettings(KConfigBase *metadata);
    void processContrastSettings(KConfigBase *metadata);
    void processBlurBehindSettings(KConfigBase *metadata);

    const QString processStyleSheet(const QString &css, Plasma::Svg::Status status);
    const QString svgStyleSheet(Plasma::Theme::ColorGroup group, Plasma::Svg::Status status);
    QColor color(Theme::ColorRole role, Theme::ColorGroup group = Theme::NormalColorGroup) const;

public Q_SLOTS:
    void compositingChanged(bool active);
    void colorsChanged();
    void settingsFileChanged(const QString &settings);
    void scheduledCacheUpdate();
    void onAppExitCleanup();
    void notifyOfChanged();
    void settingsChanged(bool emitChanges);
    void saveSvgElementsCache();

Q_SIGNALS:
    void themeChanged();
    void defaultFontChanged();
    void smallestFontChanged();
    void applicationPaletteChange();

public:
    static const char defaultTheme[];
    static const char systemColorsTheme[];
    static const char themeRcFile[];
#if HAVE_X11
    static EffectWatcher *s_backgroundContrastEffectWatcher;
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
    KColorScheme selectionColorScheme;
    KColorScheme buttonColorScheme;
    KColorScheme viewColorScheme;
    KColorScheme complementaryColorScheme;
    bool eventFilter(QObject *watched, QEvent *event) override;
    KConfigGroup cfg;
    QString defaultWallpaperTheme;
    QString defaultWallpaperSuffix;
    int defaultWallpaperWidth;
    int defaultWallpaperHeight;
    KImageCache *pixmapCache;
    KSharedConfigPtr svgElementsCache;
    QString cachedDefaultStyleSheet;
    QHash<QString, QSet<QString> > invalidElements;
    QHash<QString, QPixmap> pixmapsToCache;
    QHash<QString, QString> keysToCache;
    QHash<QString, QString> idsToCache;
    QHash<Theme::ColorGroup, QString> cachedSvgStyleSheets;
    QHash<Theme::ColorGroup, QString> cachedSelectedSvgStyleSheets;
    QHash<QString, QString> discoveries;
    QTimer *pixmapSaveTimer;
    QTimer *rectSaveTimer;
    QTimer *updateNotificationTimer;
    unsigned cacheSize;
    CacheTypes cachesToDiscard;
    QString themeVersion;
    QString themeMetadataPath;
    QString iconThemeMetadataPath;

    bool compositingActive : 1;
    bool backgroundContrastActive : 1;
    bool isDefault : 1;
    bool useGlobal : 1;
    bool hasWallpapers : 1;
    bool cacheTheme : 1;
    bool fixedName : 1;

    qreal backgroundContrast;
    qreal backgroundIntensity;
    qreal backgroundSaturation;
    bool backgroundContrastEnabled;
    bool blurBehindEnabled;

    //Version number of Plasma the Theme has been designed for
    int apiMajor;
    int apiMinor;
    int apiRevision;
};

}

#endif

extern const QString s;
