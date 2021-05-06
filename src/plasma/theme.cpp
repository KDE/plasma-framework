/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "theme.h"
#include "private/svg_p.h"
#include "private/theme_p.h"

#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QMutableListIterator>
#include <QPair>
#include <QStringBuilder>
#include <QThread>
#include <QTimer>

#include "config-plasma.h"

#include <KColorScheme>
#include <KConfigGroup>
#include <KDirWatch>
#include <KImageCache>
#include <KWindowEffects>
#include <KWindowSystem>
#include <QDebug>
#include <QStandardPaths>

#include "debug_p.h"

namespace Plasma
{
Theme::Theme(QObject *parent)
    : QObject(parent)
{
    if (!ThemePrivate::globalTheme) {
        ThemePrivate::globalTheme = new ThemePrivate;
        ThemePrivate::globalTheme->settingsChanged(false);
    }
    ThemePrivate::globalTheme->ref.ref();
    d = ThemePrivate::globalTheme;

    if (QCoreApplication::instance()) {
        connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, d, &ThemePrivate::onAppExitCleanup);
    }
    connect(d, &ThemePrivate::themeChanged, this, &Theme::themeChanged);
    connect(d, &ThemePrivate::defaultFontChanged, this, &Theme::defaultFontChanged);
    connect(d, &ThemePrivate::smallestFontChanged, this, &Theme::smallestFontChanged);
}

Theme::Theme(const QString &themeName, QObject *parent)
    : QObject(parent)
{
    auto &priv = ThemePrivate::themes[themeName];
    if (!priv) {
        priv = new ThemePrivate;
    }

    priv->ref.ref();
    d = priv;

    // turn off caching so we don't accidentally trigger unnecessary disk activity at this point
    bool useCache = d->cacheTheme;
    d->cacheTheme = false;
    d->setThemeName(themeName, false, false);
    d->cacheTheme = useCache;
    d->fixedName = true;
    if (QCoreApplication::instance()) {
        connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, d, &ThemePrivate::onAppExitCleanup);
    }
    connect(d, &ThemePrivate::themeChanged, this, &Theme::themeChanged);
}

Theme::~Theme()
{
    if (d == ThemePrivate::globalTheme) {
        if (!d->ref.deref()) {
            disconnect(ThemePrivate::globalTheme, nullptr, this, nullptr);
            delete ThemePrivate::globalTheme;
            ThemePrivate::globalTheme = nullptr;
            d = nullptr;
        }
    } else {
        if (!d->ref.deref()) {
            delete ThemePrivate::themes.take(d->themeName);
        }
    }
}

void Theme::setThemeName(const QString &themeName)
{
    if (d->themeName == themeName) {
        return;
    }

    if (d != ThemePrivate::globalTheme) {
        disconnect(QCoreApplication::instance(), nullptr, d, nullptr);
        if (!d->ref.deref()) {
            delete ThemePrivate::themes.take(d->themeName);
        }

        auto &priv = ThemePrivate::themes[themeName];
        if (!priv) {
            priv = new ThemePrivate;
        }
        priv->ref.ref();
        d = priv;
        if (QCoreApplication::instance()) {
            connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, d, &ThemePrivate::onAppExitCleanup);
        }
        connect(d, &ThemePrivate::themeChanged, this, &Theme::themeChanged);
    }

    d->setThemeName(themeName, true, true);
}

QString Theme::themeName() const
{
    return d->themeName;
}

QString Theme::imagePath(const QString &name) const
{
    // look for a compressed svg file in the theme
    if (name.contains(QLatin1String("../")) || name.isEmpty()) {
        // we don't support relative paths
        // qCDebug(LOG_PLASMA) << "Theme says: bad image path " << name;
        return QString();
    }

    const QString svgzName = name % QLatin1String(".svgz");
    QString path = d->findInTheme(svgzName, d->themeName);

    if (path.isEmpty()) {
        // try for an uncompressed svg file
        const QString svgName = name % QLatin1String(".svg");
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
    #ifndef NDEBUG
        // qCDebug(LOG_PLASMA) << "Theme says: bad image path " << name;
    #endif
    }
    */

    return path;
}

QString Theme::backgroundPath(const QString &image) const
{
    return d->imagePath(themeName(), QStringLiteral("/appbackgrounds/"), image);
}

QString Theme::styleSheet(const QString &css) const
{
    return d->processStyleSheet(css, Svg::Status::Normal);
}

QPalette Theme::palette() const
{
    return d->palette;
}

QString Theme::wallpaperPath(const QSize &size) const
{
    QString fullPath;
    QString image = d->defaultWallpaperTheme + QStringLiteral("/contents/images/%1x%2") + d->defaultWallpaperSuffix;
    QString defaultImage = image.arg(d->defaultWallpaperWidth).arg(d->defaultWallpaperHeight);

    if (size.isValid()) {
        // try to customize the paper to the size requested
        // TODO: this should do better than just fallback to the default size.
        //      a "best fit" matching would be far better, so we don't end
        //      up returning a 1920x1200 wallpaper for a 640x480 request ;)
        image = image.arg(size.width()).arg(size.height());
    } else {
        image = defaultImage;
    }

    // TODO: the theme's wallpaper overrides regularly installed wallpapers.
    //      should it be possible for user installed (e.g. locateLocal) wallpapers
    //      to override the theme?
    if (d->hasWallpapers) {
        // check in the theme first
        fullPath = d->findInTheme(QLatin1String("wallpapers/") % image, d->themeName);

        if (fullPath.isEmpty()) {
            fullPath = d->findInTheme(QLatin1String("wallpapers/") % defaultImage, d->themeName);
        }
    }

    if (fullPath.isEmpty()) {
        // we failed to find it in the theme, so look in the standard directories
        // qCDebug(LOG_PLASMA) << "looking for" << image;
        fullPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("wallpapers/") + image);
    }

    if (fullPath.isEmpty()) {
        // we still failed to find it in the theme, so look for the default in
        // the standard directories
        // qCDebug(LOG_PLASMA) << "looking for" << defaultImage;
        fullPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("wallpapers/") + defaultImage);

        if (fullPath.isEmpty()) {
#ifndef NDEBUG
            // qCDebug(LOG_PLASMA) << "exhausted every effort to find a wallpaper.";
#endif
        }
    }

    return fullPath;
}

QString Theme::wallpaperPathForSize(int width, int height) const
{
    return Plasma::Theme::wallpaperPath(QSize(width, height));
}

bool Theme::currentThemeHasImage(const QString &name) const
{
    if (name.contains(QLatin1String("../"))) {
        // we don't support relative paths
        return false;
    }

    return !(d->findInTheme(name % QLatin1String(".svgz"), d->themeName, false).isEmpty())
        || !(d->findInTheme(name % QLatin1String(".svg"), d->themeName, false).isEmpty());
}

KSharedConfigPtr Theme::colorScheme() const
{
    return d->colors;
}

QColor Theme::color(ColorRole role, ColorGroup group) const
{
    return d->color(role, group);
}

void Theme::setUseGlobalSettings(bool useGlobal)
{
    if (d->useGlobal == useGlobal) {
        return;
    }

    d->useGlobal = useGlobal;
    d->cfg = KConfigGroup();
    d->themeName.clear();
    d->settingsChanged(true);
}

bool Theme::useGlobalSettings() const
{
    return d->useGlobal;
}

bool Theme::findInCache(const QString &key, QPixmap &pix, unsigned int lastModified)
{
    // TODO KF6: Make lastModified non-optional.
    if (lastModified == 0) {
        qCWarning(LOG_PLASMA) << "findInCache with a lastModified timestamp of 0 is deprecated";
        return false;
    }

    if (!d->useCache()) {
        return false;
    }

    if (lastModified > uint(d->pixmapCache->lastModifiedTime().toSecsSinceEpoch())) {
        return false;
    }

    const QString id = d->keysToCache.value(key);
    const auto it = d->pixmapsToCache.constFind(id);
    if (it != d->pixmapsToCache.constEnd()) {
        pix = *it;
        return !pix.isNull();
    }

    QPixmap temp;
    if (d->pixmapCache->findPixmap(key, &temp) && !temp.isNull()) {
        pix = temp;
        return true;
    }

    return false;
}

void Theme::insertIntoCache(const QString &key, const QPixmap &pix)
{
    if (d->useCache()) {
        d->pixmapCache->insertPixmap(key, pix);
    }
}

void Theme::insertIntoCache(const QString &key, const QPixmap &pix, const QString &id)
{
    if (d->useCache()) {
        d->pixmapsToCache[id] = pix;
        d->keysToCache[key] = id;
        d->idsToCache[id] = key;

        // always start timer in d->pixmapSaveTimer's thread
        QMetaObject::invokeMethod(d->pixmapSaveTimer, "start", Qt::QueuedConnection);
    }
}

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 78)
bool Theme::findInRectsCache(const QString &image, const QString &element, QRectF &rect) const
{
    if (!d->useCache()) {
        return false;
    }

    bool ok = false;
    uint id = element.toLong(&ok);
    if (!ok) {
        return false;
    }

    return SvgRectsCache::instance()->findElementRect(id, image, rect);
}

QStringList Theme::listCachedRectKeys(const QString &image) const
{
    if (!d->useCache()) {
        return QStringList();
    }

    return SvgRectsCache::instance()->cachedKeysForPath(image);
}

void Theme::insertIntoRectsCache(const QString &image, const QString &element, const QRectF &rect)
{
    if (!d->useCache()) {
        return;
    }

    bool ok = false;
    uint id = element.toLong(&ok);
    if (!ok) {
        return;
    }

    uint secs = QDateTime::currentSecsSinceEpoch();
    SvgRectsCache::instance()->insert(id, image, rect, secs);
}

void Theme::invalidateRectsCache(const QString &image)
{
    SvgRectsCache::instance()->dropImageFromCache(image);
}

void Theme::releaseRectsCache(const QString &image)
{
    Q_UNUSED(image);
    // No op: the internal svg cache always writes the invalid elements in the proper place
}
#endif

void Theme::setCacheLimit(int kbytes)
{
    d->cacheSize = kbytes;
    delete d->pixmapCache;
    d->pixmapCache = nullptr;
}

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 67)
KPluginInfo Theme::pluginInfo() const
{
    return KPluginInfo(d->pluginMetaData);
}
#endif

QFont Theme::defaultFont() const
{
    return QGuiApplication::font();
}

QFont Theme::smallestFont() const
{
    return QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont);
}

QSizeF Theme::mSize(const QFont &font) const
{
    return QFontMetrics(font).boundingRect(QStringLiteral("M")).size();
}

bool Theme::backgroundContrastEnabled() const
{
    return d->backgroundContrastEnabled;
}

bool Theme::adaptiveTransparencyEnabled() const
{
    return d->adaptiveTransparencyEnabled;
}

qreal Theme::backgroundContrast() const
{
    if (qIsNaN(d->backgroundContrast)) {
        // Make up sensible default values, based on the background color
        // If we're using a dark background color, darken the background
        if (qGray(color(Plasma::Theme::BackgroundColor).rgb()) < 127) {
            return 0.45;
            // for a light theme lighten up the background
        } else {
            return 0.3;
        }
    }
    return d->backgroundContrast;
}

qreal Theme::backgroundIntensity() const
{
    if (qIsNaN(d->backgroundIntensity)) {
        if (qGray(color(Plasma::Theme::BackgroundColor).rgb()) < 127) {
            return 0.45;
        } else {
            return 1.9;
        }
    }
    return d->backgroundIntensity;
}

qreal Theme::backgroundSaturation() const
{
    if (qIsNaN(d->backgroundSaturation)) {
        return 1.7;
    }
    return d->backgroundSaturation;
}

bool Theme::blurBehindEnabled() const
{
    return d->blurBehindEnabled;
}

}

#include "moc_theme.cpp"
