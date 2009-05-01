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
#include <QtGui/QPainter>
#ifdef Q_WS_X11
#include <QX11Info>
#endif

#include <kcolorscheme.h>
#include <kcomponentdata.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kmanagerselection.h>
#include <kpixmapcache.h>
#include <ksharedconfig.h>
#include <kstandarddirs.h>
#include <kwindowsystem.h>

#include "private/packages_p.h"
#include "private/framebackgroundprovider.h"

namespace Plasma
{

#define DEFAULT_WALLPAPER_THEME "Air"
#define DEFAULT_WALLPAPER_SUFFIX ".jpg"
static const int DEFAULT_WALLPAPER_WIDTH = 1920;
static const int DEFAULT_WALLPAPER_HEIGHT = 1200;

QMap<QString, StandardThemeBackgroundProvider::PatternAlphaPair> StandardThemeBackgroundProvider::m_cachedPatterns;

class ThemePrivate
{
public:
    ThemePrivate(Theme *theme)
        : q(theme),
          colorScheme(QPalette::Active, KColorScheme::Window, KSharedConfigPtr(0)),
          buttonColorScheme(QPalette::Active, KColorScheme::Button, KSharedConfigPtr(0)),
          defaultWallpaperTheme(DEFAULT_WALLPAPER_THEME),
          defaultWallpaperSuffix(DEFAULT_WALLPAPER_SUFFIX),
          defaultWallpaperWidth(DEFAULT_WALLPAPER_WIDTH),
          defaultWallpaperHeight(DEFAULT_WALLPAPER_HEIGHT),
          pixmapCache(0),
          locolor(false),
          compositingActive(KWindowSystem::compositingActive()),
          isDefault(false),
          useGlobal(true),
          hasWallpapers(false),
          useNativeWidgetStyle(false)
    {
        generalFont = QApplication::font();
        cacheTheme = cacheConfig().readEntry("CacheTheme", true);
    }

    ~ThemePrivate()
    {
       delete pixmapCache;
    }

    KConfigGroup cacheConfig()
    {
        return KConfigGroup(KSharedConfig::openConfig(themeRcFile), "CachePolicies");
    }

    const KConfigGroup& config() const {
      return const_cast<ThemePrivate*>(this)->config();
    }

    KConfigGroup &config()
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

            cfg = KConfigGroup(KSharedConfig::openConfig(themeRcFile), groupName);
        }

        return cfg;
    }

    /**
     *  Reads optional configuration, that is specific to the current composite mode:
     *  When composition is active, the configuration entry is prefixed with "composite_".
     *  Optionally, the configuration can also be specific to the specified image path:
     *  Then the image path has to be appended to the configuration name
     */
    template<class T>
    T readOptionalConfig(QString configName, T _default, QString imagePath) {
      if(compositingActive)
          configName = "composite_" + configName;

      T ret = config().readEntry(configName, _default);
      return config().readEntry(configName + "_" + imagePath, ret);
    }

    bool hasOptionalConfig(QString configName, QString imagePath = QString()) {
      if(compositingActive)
          configName = "composite_" + configName;

      return config().hasKey(configName) || config().hasKey(configName + "_" + imagePath);
    }

    QString findInTheme(const QString &image, const QString &theme) const;
    void compositingChanged();
    void discardCache();
    void discardCache(bool recreateElementsCache);
    void colorsChanged();
    bool useCache();
    void settingsFileChanged(const QString &);
    void setThemeName(const QString &themeName, bool writeSettings);

    static const char *defaultTheme;
    static const char *themeRcFile;
    static PackageStructure::Ptr packageStructure;

    Theme *q;
    QString themeName;
    KSharedConfigPtr colors;
    KColorScheme colorScheme;
    KColorScheme buttonColorScheme;
    KConfigGroup cfg;
    QFont generalFont;
    QString defaultWallpaperTheme;
    QString defaultWallpaperSuffix;
    int defaultWallpaperWidth;
    int defaultWallpaperHeight;
    KPixmapCache *pixmapCache;
    KSharedConfigPtr svgElementsCache;
    QHash<QString, QSet<QString> > invalidElements;

#ifdef Q_WS_X11
    KSelectionWatcher *compositeWatch;
#endif
    bool locolor : 1;
    bool compositingActive : 1;
    bool isDefault : 1;
    bool useGlobal : 1;
    bool hasWallpapers : 1;
    bool cacheTheme : 1;
    bool useNativeWidgetStyle :1;
};

PackageStructure::Ptr ThemePrivate::packageStructure(0);
const char *ThemePrivate::defaultTheme = "default";
const char *ThemePrivate::themeRcFile = "plasmarc";

bool ThemePrivate::useCache()
{
    if (cacheTheme && !pixmapCache) {
        pixmapCache = new KPixmapCache("plasma_theme_" + themeName);
        pixmapCache->setCacheLimit(cacheConfig().readEntry("ThemeCacheKb", 80 * 1024));
    }

    return cacheTheme;
}

QString ThemePrivate::findInTheme(const QString &image, const QString &theme) const
{
    //TODO: this should be using Package
    QString search;

    if (locolor) {
        search = "desktoptheme/" + theme + "/locolor/" + image;
        search =  KStandardDirs::locate("data", search);
    } else if (!compositingActive && !config().readEntry<bool>("forceTransparentTheme", false)) {
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

void ThemePrivate::compositingChanged()
{
#ifdef Q_WS_X11
    bool nowCompositingActive = compositeWatch->owner() != None;

    if (compositingActive != nowCompositingActive) {
        compositingActive = nowCompositingActive;
        discardCache(true);
        emit q->themeChanged();
    }
#endif
}

void ThemePrivate::discardCache()
{
    discardCache(true);
}

void ThemePrivate::discardCache(bool recreateElementsCache)
{
    KPixmapCache::deleteCache("plasma_theme_" + themeName);
    delete pixmapCache;
    pixmapCache = 0;
    invalidElements.clear();

    svgElementsCache = 0;

    QString svgElementsFile = KStandardDirs::locateLocal("cache", "plasma-svgelements-" + themeName);
    if (!svgElementsFile.isEmpty()) {
        QFile f(svgElementsFile);
        f.remove();
    }

    if (recreateElementsCache) {
        svgElementsCache = KSharedConfig::openConfig(svgElementsFile);
    }
}

void ThemePrivate::colorsChanged()
{
    discardCache(true);
    colorScheme = KColorScheme(QPalette::Active, KColorScheme::Window, colors);
    buttonColorScheme = KColorScheme(QPalette::Active, KColorScheme::Button, colors);
    emit q->themeChanged();
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

    //FIXME: if/when kconfig gets change notification, this will be unecessary
    KDirWatch::self()->addFile(KStandardDirs::locateLocal("config", ThemePrivate::themeRcFile));
    connect(KDirWatch::self(), SIGNAL(created(QString)), this, SLOT(settingsFileChanged(QString)));
    connect(KDirWatch::self(), SIGNAL(dirty(QString)), this, SLOT(settingsFileChanged(QString)));
}

Theme::~Theme()
{
    QHashIterator<QString, QSet<QString> > it(d->invalidElements);
    while (it.hasNext()) {
        it.next();
        KConfigGroup imageGroup(d->svgElementsCache, it.key());
        imageGroup.writeEntry("invalidElements", it.value().toList()); //FIXME: add QSet support to KConfig
    }

    delete d;
}

PackageStructure::Ptr Theme::packageStructure()
{
    if (!ThemePrivate::packageStructure) {
        ThemePrivate::packageStructure = new ThemePackage();
    }

    return ThemePrivate::packageStructure;
}

void ThemePrivate::settingsFileChanged(const QString &file)
{
    kDebug() << file;
    QMap< QString, QString > oldEntries = config().entryMap();
    
    config().config()->reparseConfiguration();
    
    if(oldEntries != config().entryMap())
      q->settingsChanged();
}

void Theme::settingsChanged()
{
     StandardThemeBackgroundProvider::clearCache(); //So we don't waste memory with background images that are not used

     QString newThemeName = d->config().readEntry("name", ThemePrivate::defaultTheme);
     if(newThemeName != d->themeName) {
      d->setThemeName(newThemeName, false);
     }else{
      ///@todo More precise monitoring of attributes
      d->discardCache(true);
      emit themeChanged();
     }
}

void Theme::setThemeName(const QString &themeName)
{
    d->setThemeName(themeName, true);
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

    //TODO: should we care about names with relative paths in them?
    QString themePath = KStandardDirs::locate("data", "desktoptheme/" + theme + '/');
    if (themePath.isEmpty() && themeName.isEmpty()) {
        themePath = KStandardDirs::locate("data", "desktoptheme/default/");

        if (themePath.isEmpty()) {
            return;
        }

        theme = ThemePrivate::defaultTheme;
    }

    if (themeName == theme) {
        return;
    }

    //discard the old theme cache
    if (!themeName.isEmpty() && pixmapCache) {
        discardCache(false);
    }

    themeName = theme;

    // load the color scheme config
    QString colorsFile = KStandardDirs::locate("data", "desktoptheme/" + theme + "/colors");
    //kDebug() << "we're going for..." << colorsFile << "*******************";

    // load the wallpaper settings, if any
    QString metadataPath(KStandardDirs::locate("data", "desktoptheme/" + theme + "/metadata.desktop"));
    KConfig metadata(metadataPath);
    KConfigGroup cg;
    if (metadata.hasGroup("Wallpaper")) {
        // we have a theme color config, so let's also check to see if
        // there is a wallpaper defined in there.
        cg = KConfigGroup(&metadata, "Wallpaper");
    } else {
        // since we didn't find an entry in the theme, let's look in the main
        // theme config
        cg = config();
    }

    defaultWallpaperTheme = cg.readEntry("defaultWallpaperTheme", DEFAULT_WALLPAPER_THEME);
    defaultWallpaperSuffix = cg.readEntry("defaultFileSuffix", DEFAULT_WALLPAPER_SUFFIX);
    defaultWallpaperWidth = cg.readEntry("defaultWidth", DEFAULT_WALLPAPER_WIDTH);
    defaultWallpaperHeight = cg.readEntry("defaultHeight", DEFAULT_WALLPAPER_HEIGHT);

    cg = KConfigGroup(&metadata, "Settings");
    useNativeWidgetStyle = cg.readEntry("UseNativeWidgetStyle", false);

    QObject::disconnect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
                        q, SLOT(colorsChanged()));

    if (colorsFile.isEmpty()) {
        colors = 0;
        QObject::connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
                         q, SLOT(colorsChanged()));
    } else {
        colors = KSharedConfig::openConfig(colorsFile);
    }

    colorScheme = KColorScheme(QPalette::Active, KColorScheme::Window, colors);
    buttonColorScheme = KColorScheme(QPalette::Active, KColorScheme::Button, colors);
    hasWallpapers = KStandardDirs::exists(KStandardDirs::locateLocal("data", "desktoptheme/" + theme + "/wallpapers/"));

    if (isDefault && writeSettings) {
        // we're the default theme, let's save our state
        KConfigGroup &cg = config();
        if (ThemePrivate::defaultTheme == themeName) {
            cg.deleteEntry("name");
        } else {
            cg.writeEntry("name", themeName);
        }
        cg.sync();
    }

    //check for expired cache
    QFile f(metadataPath);
    QFileInfo info(f);

    if (useCache() && info.lastModified().toTime_t() > pixmapCache->timestamp()) {
        discardCache(false);
    }

    invalidElements.clear();
    QString svgElementsFile = KStandardDirs::locateLocal("cache", "plasma-svgelements-" + themeName);
    svgElementsCache = KSharedConfig::openConfig(svgElementsFile);

    emit q->themeChanged();
}

QString Theme::themeName() const
{
    return d->themeName;
}

QString Theme::imagePath(const QString &name) const
{
    // look for a compressed svg file in the theme
    if (name.contains("../")) {
        // we don't support relative paths
        return QString();
    }

    QString path = d->findInTheme(name + ".svgz", d->themeName);

    if (path.isEmpty()) {
        // try for an uncompressed svg file
        path = d->findInTheme(name + ".svg", d->themeName);

        if (path.isEmpty() && d->themeName != ThemePrivate::defaultTheme) {
            // try a compressed svg file in the default theme
            path = d->findInTheme(name + ".svgz", ThemePrivate::defaultTheme);

            if (path.isEmpty()) {
                // try an uncompressed svg file in the default theme
                path = d->findInTheme(name + ".svg", ThemePrivate::defaultTheme);
            }
        }

    }

    if (path.isEmpty()) {
        kDebug() << "Theme says: bad image path " << name;
    }

    return path;
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
        fullPath = d->findInTheme("wallpapers/" + image, d->themeName);

        if (fullPath.isEmpty()) {
            fullPath = d->findInTheme("wallpapers/" + defaultImage, d->themeName);
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

    return !(d->findInTheme(name + ".svgz", d->themeName).isEmpty()) ||
           !(d->findInTheme(name + ".svg", d->themeName).isEmpty());
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
            break;

        case HighlightColor:
            return d->colorScheme.background(KColorScheme::ActiveBackground).color();
            break;

        case BackgroundColor:
            return d->colorScheme.background().color();
            break;

        case ButtonTextColor:
            return d->buttonColorScheme.foreground(KColorScheme::NormalText).color();
            break;

        case ButtonBackgroundColor:
            return d->buttonColorScheme.background(KColorScheme::ActiveBackground).color();
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
    switch (role) {
    case DesktopFont:
    {
        KConfigGroup cg(KGlobal::config(), "General");
        return cg.readEntry("desktopFont", QFont("Sans Serif", 10));
    }
    break;
    case DefaultFont:
    default:
        return d->generalFont;
        break;
    }
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
    return d->useCache() && d->pixmapCache->find(key, pix);
}

// BIC FIXME: Should be merged with the other findInCache method above when we break BC
bool Theme::findInCache(const QString &key, QPixmap &pix, unsigned int lastModified)
{
    if (d->useCache() && lastModified > d->pixmapCache->timestamp()) {
        d->discardCache();
    }

    return findInCache(key, pix);
}

void Theme::insertIntoCache(const QString& key, const QPixmap& pix)
{
    if (d->useCache()) {
        d->pixmapCache->insert(key, pix);
    }
}

bool Theme::findInRectsCache(const QString &image, const QString &element, QRectF &rect) const
{
    if (!d->pixmapCache) {
        return false;
    }

    KConfigGroup imageGroup(d->svgElementsCache, image);
    rect = imageGroup.readEntry(element + "Size", QRectF());

    if (rect.isValid()) {
        return true;
    }

    //A single _ means the element is empty and we're asked for the size of
    //the whole image, so the whole image is never invalid
    if (element.count('_') == 1) {
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

void Theme::insertIntoRectsCache(const QString& image, const QString &element, const QRectF &rect)
{
    if (!d->pixmapCache) {
        return;
    }

    if (rect.isValid()) {
        KConfigGroup imageGroup(d->svgElementsCache, image);
        imageGroup.writeEntry(element + "Size", rect);
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
    KConfigGroup imageGroup(d->svgElementsCache, image);
    imageGroup.deleteGroup();

    releaseRectsCache(image);
}

void Theme::releaseRectsCache(const QString &image)
{
    QHash<QString, QSet<QString> >::iterator it = d->invalidElements.find(image);
    if (it != d->invalidElements.end()) {
        d->invalidElements.erase(it);
    }
}

void Theme::setCacheLimit(int kbytes)
{
    if (d->useCache()) {
        d->pixmapCache->setCacheLimit(kbytes);
    }
}


void StandardThemeBackgroundProvider::clearCache()
{
    m_cachedPatterns.clear();
}

StandardThemeBackgroundProvider::StandardThemeBackgroundProvider(Theme* theme, QString imagePath) : m_color(Qt::black), m_patternAlpha(0), m_valid(false)
{
    if(theme->d->locolor)
        return;

    if((imagePath.startsWith("widgets/panel-") || imagePath.startsWith("dialogs/")) &&
       (theme->d->hasOptionalConfig("frameBackgroundColor") || theme->d->hasOptionalConfig("frameBackgroundPattern")))
    {
        m_valid = true;
        m_color = theme->d->readOptionalConfig<QColor>("frameBackgroundColor", Qt::black, imagePath);
        if(theme->d->hasOptionalConfig("frameBackgroundColor"))
            m_color.setAlpha(theme->d->readOptionalConfig<int>("frameBackgroundColorAlpha", 255, imagePath));
        else
            m_color.setAlpha(0);

        m_pattern = theme->d->readOptionalConfig<QString>("frameBackgroundPattern", QString(), imagePath);
        m_patternAlpha = theme->d->readOptionalConfig<int>("frameBackgroundPatternAlpha", 255, imagePath);
        m_offsetX = theme->d->readOptionalConfig<int>("frameBackgroundPatternOffsetX", 0, imagePath);
        m_offsetY = theme->d->readOptionalConfig<int>("frameBackgroundPatternOffsetY", 0, imagePath);
        int randomX = theme->d->readOptionalConfig<int>("frameBackgroundPatternOffsetRandomX", 0, imagePath);
        int randomY = theme->d->readOptionalConfig<int>("frameBackgroundPatternOffsetRandomY", 0, imagePath);

        if(randomX || randomY) {
            //Add "this" so the offsets are different after every startup, but stay same for the same image path
            qsrand(qHash(imagePath) + ((size_t)this) + randomX + 11 * randomY);
            if(randomX)
                m_offsetX += qrand() % randomX;
            if(randomY)
                m_offsetY += qrand() % randomY;
        }

    }
}

void StandardThemeBackgroundProvider::apply(QPainter& target) const
{
    if(!m_valid)
        return;

    target.setCompositionMode(QPainter::CompositionMode_DestinationOver);

    //Apply color
    if(m_color.alpha())
        target.fillRect(target.clipRegion().boundingRect(), m_color);

    //Apply pattern
    if(m_patternAlpha && !m_pattern.isEmpty()) {
        if(!m_cachedPatterns.contains(m_pattern) || m_cachedPatterns[m_pattern].second != m_patternAlpha) {
            m_cachedPatterns.remove(m_pattern);
            m_cachedPatterns.insert(m_pattern, PatternAlphaPair(QImage(m_pattern), m_patternAlpha));
            if(m_patternAlpha != 255) {
                PatternAlphaPair& cached(m_cachedPatterns[m_pattern]);
                //Apply lower alpha value to the pattern
                QImage alpha(cached.first.size(), QImage::Format_ARGB32);
                alpha.fill(QColor(cached.second, cached.second, cached.second).rgb());
                cached.first.setAlphaChannel(alpha);
            }
        }

        PatternAlphaPair& cached(m_cachedPatterns[m_pattern]);

        if(!cached.first.isNull()) {
            QBrush brush;
            QColor col(Qt::white);
            col.setAlpha(m_patternAlpha);
            brush.setColor(col);
            brush.setTextureImage(cached.first);
            target.setBrushOrigin(-m_offsetX, -m_offsetY);
            target.fillRect(target.clipRegion().boundingRect(), brush);
        }else{
            kDebug() << "failed to load pattern" << m_pattern;
        }
    }
}

StandardThemeBackgroundProvider::operator bool() const
{
    return m_valid;
}

QString StandardThemeBackgroundProvider::identity() const
{
    if(!m_valid)
        return QString();
    return QString("bgcolor_%1=").arg(m_color.alpha()) + m_color.name()+QString("_pattern_%1=").arg(m_patternAlpha)+m_pattern + QString("_offsets_%1_%2__").arg(m_offsetX).arg(m_offsetY);
}
}

#include <theme.moc>
