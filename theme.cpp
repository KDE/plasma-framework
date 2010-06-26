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
#include <QTimer>
#include <QPair>
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

#define DEFAULT_WALLPAPER_THEME "Ethais"
#define DEFAULT_WALLPAPER_SUFFIX ".png"
static const int DEFAULT_WALLPAPER_WIDTH = 1920;
static const int DEFAULT_WALLPAPER_HEIGHT = 1200;

class ThemePrivate
{
public:
    ThemePrivate(Theme *theme)
        : q(theme),
          colorScheme(QPalette::Active, KColorScheme::Window, KSharedConfigPtr(0)),
          buttonColorScheme(QPalette::Active, KColorScheme::Button, KSharedConfigPtr(0)),
          viewColorScheme(KColorScheme(QPalette::Active, KColorScheme::View, KSharedConfigPtr(0))),
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
        ThemeConfig config;
        cacheTheme = config.cacheTheme();

#ifdef Q_WS_X11
        Display *dpy = QX11Info::display();
        int screen = DefaultScreen(dpy);
        locolor = DefaultDepth(dpy, screen) < 16;

        if (!locolor) {
            char net_wm_cm_name[100];
            sprintf(net_wm_cm_name, "_NET_WM_CM_S%d", screen);
            compositeWatch = new KSelectionWatcher(net_wm_cm_name, -1, q);
            QObject::connect(compositeWatch, SIGNAL(newOwner(Window)), q, SLOT(compositingChanged()));
            QObject::connect(compositeWatch, SIGNAL(lostOwner()), q, SLOT(compositingChanged()));
        }
#endif

        saveTimer = new QTimer(q);
        saveTimer->setSingleShot(true);
        QObject::connect(saveTimer, SIGNAL(timeout()), q, SLOT(scheduledCacheUpdate()));
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

    QString findInTheme(const QString &image, const QString &theme) const;
    void compositingChanged();
    void discardCache();
    void scheduledCacheUpdate();
    void colorsChanged();
    bool useCache();
    void settingsFileChanged(const QString &);
    void setThemeName(const QString &themeName, bool writeSettings);
    void onAppExitCleanup();
    void processWallpaperSettings(KConfigBase *metadata);
    void processAnimationSettings(const QString &theme, KConfigBase *metadata);

    const QString processStyleSheet(const QString &css);

    static const char *defaultTheme;
    static const char *themeRcFile;
    static PackageStructure::Ptr packageStructure;

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
    QTimer *saveTimer;

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
        ThemeConfig config;
        pixmapCache = new KImageCache("plasma_theme_" + themeName, config.themeCacheKb() * 1024);
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

QString ThemePrivate::findInTheme(const QString &image, const QString &theme) const
{
    //TODO: this should be using Package
    QString search;

    if (locolor) {
        search = "desktoptheme/" + theme + "/locolor/" + image;
        search =  KStandardDirs::locate("data", search);
    } else if (!compositingActive) {
        search = "desktoptheme/" + theme + "/opaque/" + image;
        search =  KStandardDirs::locate("data", search);
    } else if (WindowEffects::isEffectAvailable(WindowEffects::BlurBehind)) {
        search = "desktoptheme/" + theme + "/translucent/" + image;
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
        discardCache();
        emit q->themeChanged();
    }
#endif
}

void ThemePrivate::discardCache()
{
    KSharedDataCache::deleteCache("plasma_theme_" + themeName);
    delete pixmapCache;
    pixmapCache = 0;
    invalidElements.clear();
    pixmapsToCache.clear();
    saveTimer->stop();

    const QString svgElementsFile = KStandardDirs::locateLocal("cache", "plasma-svgelements-" + themeName);
    if (!svgElementsFile.isEmpty()) {
        QFile f(svgElementsFile);
        f.remove();
    }

    svgElementsCache = KSharedConfig::openConfig(svgElementsFile);
}

void ThemePrivate::scheduledCacheUpdate()
{
    QHashIterator<QString, QPixmap> it(pixmapsToCache);
    while (it.hasNext()) {
        it.next();
        pixmapCache->insertPixmap(idsToCache[it.key()], it.value());
    }

    pixmapsToCache.clear();
    keysToCache.clear();
    idsToCache.clear();
}

void ThemePrivate::colorsChanged()
{
    discardCache();
    colorScheme = KColorScheme(QPalette::Active, KColorScheme::Window, colors);
    buttonColorScheme = KColorScheme(QPalette::Active, KColorScheme::Button, colors);
    viewColorScheme = KColorScheme(QPalette::Active, KColorScheme::View, colors);
    emit q->themeChanged();
}

const QString ThemePrivate::processStyleSheet(const QString &css)
{
    QString stylesheet;
    if (css.isEmpty()) {
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
    } else {
        stylesheet = css;
    }

    QHash<QString, QString> elements;
    // If you add elements here, make sure their names are sufficiently unique to not cause
    // clashes between element keys
    elements["%textcolor"] = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor).name();
    elements["%backgroundcolor"] =
                Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor).name();
    elements["%visitedlink"] =
                Plasma::Theme::defaultTheme()->color(Plasma::Theme::VisitedLinkColor).name();
    elements["%activatedlink"] =
                Plasma::Theme::defaultTheme()->color(Plasma::Theme::HighlightColor).name();
    elements["%hoveredlink"] =
                Plasma::Theme::defaultTheme()->color(Plasma::Theme::HighlightColor).name();
    elements["%link"] = Plasma::Theme::defaultTheme()->color(Plasma::Theme::LinkColor).name();
    elements["%buttonbackgroundcolor"] =
                Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonBackgroundColor).name();
    elements["%smallfontsize"] =
                QString("%1pt").arg(KGlobalSettings::smallestReadableFont().pointSize());

    QFont font = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    elements["%fontsize"] =
                QString("%1pt").arg(font.pointSize());
    elements["%fontfamily"] = font.family();

    foreach (const QString &k, elements.keys()) {
        stylesheet.replace(k, elements[k]);
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
    d->setThemeName(d->config().readEntry("name", ThemePrivate::defaultTheme), false);
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
    const QString animDir = "desktoptheme/" + theme + "/animations/";
    foreach (const QString &path, cg.keyList()) {
        const QStringList anims = cg.readEntry(path, QStringList());
        foreach (const QString &anim, anims) {
            if (!animationMapping.contains(anim)) {
                kDebug() << "Registering animation. animDir: " << animDir
                         << "\tanim: " << anim
                         << "\tpath: " << path << "\t*******\n\n\n";
                //key: desktoptheme/default/animations/+ all.js
                //value: ZoomAnimation
                animationMapping.insert(anim, animDir + path);
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
        discardCache();
    }

    themeName = theme;

    // load the color scheme config
    const QString colorsFile = KStandardDirs::locate("data", "desktoptheme/" + theme + "/colors");
    //kDebug() << "we're going for..." << colorsFile << "*******************";

    // load the wallpaper settings, if any
    const QString metadataPath(KStandardDirs::locate("data", "desktoptheme/" + theme + "/metadata.desktop"));
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

        QString metadataPath(KStandardDirs::locate("data", "desktoptheme/" + theme + "/metadata.desktop"));
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
        QString metadataPath(KStandardDirs::locate("data", "desktoptheme/" + theme + "/metadata.desktop"));
        KConfig metadata(metadataPath);
        processAnimationSettings(theme, &metadata);
        processWallpaperSettings(&metadata);
    }

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
    viewColorScheme = KColorScheme(QPalette::Active, KColorScheme::View, colors);
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

    if (useCache() && info.lastModified().toTime_t() > pixmapCache->lastModifiedTime()) {
        discardCache();
    } else {
        QString svgElementsFile = KStandardDirs::locateLocal("cache", "plasma-svgelements-" + themeName);
        svgElementsCache = KSharedConfig::openConfig(svgElementsFile);
    }

    invalidElements.clear();
    emit q->themeChanged();
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
        kDebug() << "Theme says: bad image path " << name;
        return QString();
    }

    QString path = d->findInTheme(name + ".svgz", d->themeName);

    if (path.isEmpty()) {
        // try for an uncompressed svg file
        path = d->findInTheme(name + ".svg", d->themeName);

        // search in fallback themes if necessary
        for (int i = 0; path.isEmpty() && i < d->fallbackThemes.count(); ++i) {
            if (d->themeName == d->fallbackThemes[i]) {
                continue;
            }

            // try a compressed svg file in the fallback theme
            path = d->findInTheme(name + ".svgz", d->fallbackThemes[i]);

            if (path.isEmpty()) {
                // try an uncompressed svg file in the fallback theme
                path = d->findInTheme(name + ".svg", d->fallbackThemes[i]);
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
            return d->colorScheme.decoration(KColorScheme::HoverColor).color();
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
    case DesktopFont:
    {
        KConfigGroup cg(KGlobal::config(), "General");
        return cg.readEntry("desktopFont", d->generalFont);
    }
    break;
    case DefaultFont:
    default:
        return d->generalFont;
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
            return true;
        }

        QPixmap temp;
        if(d->pixmapCache->findPixmap(key, &temp) && !temp.isNull()) {
            pix = temp;
            return true;
        }
        return false;
    }

    return false;
}

// BIC FIXME: Should be merged with the other findInCache method above when we break BC
bool Theme::findInCache(const QString &key, QPixmap &pix, unsigned int lastModified)
{
    if (d->useCache() && lastModified > d->pixmapCache->lastModifiedTime()) {
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
        d->saveTimer->start(600);
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
        KConfigGroup imageGroup(d->svgElementsCache, it.key());
        imageGroup.writeEntry("invalidElements", it.value().toList());
        d->invalidElements.erase(it);
    }
}

void Theme::setCacheLimit(int kbytes)
{
    if (d->useCache()) {
        ;
        // Too late for you bub.
        // d->pixmapCache->setCacheLimit(kbytes);
    }
}

}

#include <theme.moc>
