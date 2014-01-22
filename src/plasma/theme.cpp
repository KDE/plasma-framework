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
#include "private/theme_p.h"

#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QFileInfo>
#include <QMutableListIterator>
#include <QPair>
#include <QStringBuilder>
#include <QTimer>

#include "config-plasma.h"

#include <kcolorscheme.h>
#include <kconfiggroup.h>
#include <QDebug>
#include <kdirwatch.h>
#include <kiconloader.h>
#include <kmanagerselection.h>
#include <kimagecache.h>
#include <ksharedconfig.h>
#include <kwindoweffects.h>
#include <kwindowsystem.h>
#include <qstandardpaths.h>

#include "private/packages_p.h"

namespace Plasma
{

Theme::Theme(QObject *parent)
    : QObject(parent)
{
    if (!ThemePrivate::globalTheme) {
        ThemePrivate::globalTheme = new ThemePrivate;
    }
    ThemePrivate::globalThemeRefCount.ref();
    d = ThemePrivate::globalTheme;

    d->settingsChanged();
    if (QCoreApplication::instance()) {
        connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()),
                d, SLOT(onAppExitCleanup()));
    }
    connect(d, &ThemePrivate::themeChanged, this, &Theme::themeChanged);
    connect(d, &ThemePrivate::defaultFontChanged, this, &Theme::defaultFontChanged);
    connect(d, &ThemePrivate::smallestFontChanged, this, &Theme::smallestFontChanged);
}

Theme::Theme(const QString &themeName, QObject *parent)
    : QObject(parent)
{
    if (!ThemePrivate::themes.contains(themeName)) {
        ThemePrivate::themes[themeName] = new ThemePrivate;
        ThemePrivate::themesRefCount[themeName] = QAtomicInt();
    }

    ThemePrivate::themesRefCount[themeName].ref();
    d = ThemePrivate::themes[themeName];

    // turn off caching so we don't accidently trigger unnecessary disk activity at this point
    bool useCache = d->cacheTheme;
    d->cacheTheme = false;
    d->setThemeName(themeName, true);
    d->cacheTheme = useCache;
    if (QCoreApplication::instance()) {
        connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()),
                d, SLOT(onAppExitCleanup()));
    }
    connect(d, &ThemePrivate::themeChanged, this, &Theme::themeChanged);
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

    if (d == ThemePrivate::globalTheme) {
        if (!ThemePrivate::globalThemeRefCount.deref()) {
            disconnect(ThemePrivate::globalTheme, 0, this, 0);
            delete ThemePrivate::globalTheme;
            ThemePrivate::globalTheme = 0;
            d = 0;
        }
    } else {
        if (!ThemePrivate::themesRefCount[d->themeName].deref()) {
            ThemePrivate *themePrivate = ThemePrivate::themes[d->themeName];
            ThemePrivate::themes.remove(d->themeName);
            ThemePrivate::themesRefCount.remove(d->themeName);
            delete themePrivate;
        }
    }
}

void Theme::setThemeName(const QString &themeName)
{
    if (d->themeName == themeName) {
        return;
    }

    if (d != ThemePrivate::globalTheme) {
        disconnect(QCoreApplication::instance(), 0, d, 0);
        if (!ThemePrivate::themesRefCount[d->themeName].deref()) {
            ThemePrivate *themePrivate = ThemePrivate::themes[d->themeName];
            ThemePrivate::themes.remove(d->themeName);
            ThemePrivate::themesRefCount.remove(d->themeName);
            delete themePrivate;
        }
        if (!ThemePrivate::themes.contains(themeName)) {
            ThemePrivate::themes[themeName] = new ThemePrivate;
            ThemePrivate::themesRefCount[themeName] = QAtomicInt();
        }
        ThemePrivate::themesRefCount[themeName].ref();
        d = ThemePrivate::themes[themeName];
        if (QCoreApplication::instance()) {
            connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()),
                    d, SLOT(onAppExitCleanup()));
        }
        connect(d, &ThemePrivate::themeChanged, this, &Theme::themeChanged);
    }

    d->setThemeName(themeName, true);
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
        //qDebug() << "Theme says: bad image path " << name;
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
#ifndef NDEBUG
        // qDebug() << "Theme says: bad image path " << name;
#endif
    }
    */

    return path;
}

QString Theme::styleSheet(const QString &css) const
{
    return d->processStyleSheet(css);
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
        //qDebug() << "looking for" << image;
        fullPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("wallpapers/") + image);
    }

    if (fullPath.isEmpty()) {
        // we still failed to find it in the theme, so look for the default in
        // the standard directories
        //qDebug() << "looking for" << defaultImage;
        fullPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("wallpapers/") + defaultImage);

        if (fullPath.isEmpty()) {
#ifndef NDEBUG
            // qDebug() << "exhausted every effort to find a wallpaper.";
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
    return d->color(role);
}

void Theme::setUseGlobalSettings(bool useGlobal)
{
    if (d->useGlobal == useGlobal) {
        return;
    }

    d->useGlobal = useGlobal;
    d->cfg = KConfigGroup();
    d->themeName.clear();
    d->settingsChanged();
}

bool Theme::useGlobalSettings() const
{
    return d->useGlobal;
}

bool Theme::findInCache(const QString &key, QPixmap &pix, unsigned int lastModified)
{
    if (lastModified != 0 && d->useCache() && lastModified > uint(d->pixmapCache->lastModifiedTime().toTime_t())) {
        return false;
    }

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

        //always start timer in d->saveTimer's thread
        QMetaObject::invokeMethod(d->saveTimer, "start", Qt::QueuedConnection);
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
    d->cacheSize = kbytes;
    delete d->pixmapCache;
    d->pixmapCache = 0;
}

KPluginInfo Theme::pluginInfo() const
{
    return d->pluginInfo;
}

QFont Theme::defaultFont() const
{
    return QApplication::font();
}

QFont Theme::smallestFont() const
{
    return QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont);
}

QColor Theme::textColor() const
{
    return Plasma::Theme::color(Plasma::Theme::TextColor);
}

QColor Theme::highlightColor() const
{
    return Plasma::Theme::color(Plasma::Theme::HighlightColor);
}

QColor Theme::backgroundColor() const
{
    return Plasma::Theme::color(Plasma::Theme::BackgroundColor);
}

QColor Theme::buttonTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::ButtonTextColor);
}

QColor Theme::buttonBackgroundColor() const
{
    return Plasma::Theme::color(Plasma::Theme::ButtonBackgroundColor);
}

QColor Theme::linkColor() const
{
    return Plasma::Theme::color(Plasma::Theme::LinkColor);
}

QColor Theme::visitedLinkColor() const
{
    return Plasma::Theme::color(Plasma::Theme::VisitedLinkColor);
}

QColor Theme::buttonHoverColor() const
{
    return Plasma::Theme::color(Plasma::Theme::ButtonHoverColor);
}

QColor Theme::buttonFocusColor() const
{
    return Plasma::Theme::color(Plasma::Theme::ButtonFocusColor);
}

QColor Theme::viewTextColor() const
{
    return Plasma::Theme::color(Plasma::Theme::ViewTextColor);
}

QColor Theme::viewBackgroundColor() const
{
    return Plasma::Theme::color(Plasma::Theme::ViewBackgroundColor);
}

QColor Theme::viewHoverColor() const
{
    return Plasma::Theme::color(Plasma::Theme::ViewHoverColor);
}

QColor Theme::viewFocusColor() const
{
    return Plasma::Theme::color(Plasma::Theme::ViewFocusColor);
}

int Theme::defaultIconSize() const
{
    return d->defaultIconSize;
}

QSizeF Theme::mSize(const QFont &font) const
{
    return QFontMetrics(font).boundingRect("M").size();
}

int Theme::smallSpacing() const
{
    return d->smallSpacing;
}

int Theme::largeSpacing() const
{
    return d->largeSpacing;
}



}

#include "moc_theme.cpp"
