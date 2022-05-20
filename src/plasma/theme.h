/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_THEME_H
#define PLASMA_THEME_H

#include <QFont>
#include <QGuiApplication>
#include <QObject>

#include <plasma/plasma_export.h>
#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 94)
#include <KPluginInfo>
#endif
#include <KSharedConfig>

class KPluginMetaData;

namespace Plasma
{
class ThemePrivate;
class SvgPrivate;

/**
 * @class Theme plasma/theme.h <Plasma/Theme>
 *
 * @short Interface to the Plasma theme
 *
 *
 * Plasma::Theme provides access to a common and standardized set of graphic
 * elements stored in SVG format. This allows artists to create single packages
 * of SVGs that will affect the look and feel of all workspace components.
 *
 * Plasma::Svg uses Plasma::Theme internally to locate and load the appropriate
 * SVG data. Alternatively, Plasma::Theme can be used directly to retrieve
 * file system paths to SVGs by name.
 */
class PLASMA_EXPORT Theme : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString themeName READ themeName NOTIFY themeChanged)
    Q_PROPERTY(bool useGlobalSettings READ useGlobalSettings NOTIFY themeChanged)
    Q_PROPERTY(QString wallpaperPath READ wallpaperPath NOTIFY themeChanged)

    // fonts
    Q_PROPERTY(QFont defaultFont READ defaultFont NOTIFY defaultFontChanged)
    Q_PROPERTY(QFont smallestFont READ smallestFont NOTIFY smallestFontChanged)

    // stylesheet
    Q_PROPERTY(QString styleSheet READ styleSheet NOTIFY themeChanged)

    Q_PROPERTY(QPalette palette READ palette NOTIFY themeChanged)

public:
    enum ColorRole {
        TextColor = 0, /**<  the text color to be used by items resting on the background */
        BackgroundColor = 1, /**< the default background color */
        HighlightColor = 2, /**<  the text highlight color to be used by items resting
                                   on the background */
        HoverColor = 3, /**< color for hover effect on view */
        FocusColor = 4, /**< color for focus effect on view */
        LinkColor = 5, /**< color for clickable links */
        VisitedLinkColor = 6, /**< color visited clickable links */
        HighlightedTextColor = 7, /**< color contrasting with HighlightColor, to be used for instance with */
        PositiveTextColor = 8, /**< color of foreground objects with a "positive message" connotation (usually green) */
        NeutralTextColor = 9, /**< color of foreground objects with a "neutral message" connotation (usually yellow) */
        NegativeTextColor = 10, /**< color of foreground objects with a "negative message" connotation (usually red) */
        DisabledTextColor = 11, /**< color of disabled text @since 5.64 */
    };

    enum ColorGroup {
        NormalColorGroup = 0,
        ButtonColorGroup = 1,
        ViewColorGroup = 2,
        ComplementaryColorGroup = 3,
        HeaderColorGroup,
        ToolTipColorGroup,
    };
    Q_ENUM(ColorGroup)

    /**
     * Default constructor. It will be the global theme configured in plasmarc
     * @param parent the parent object
     */
    explicit Theme(QObject *parent = nullptr);

    /**
     * Construct a theme. It will be a custom theme instance of themeName.
     * @param themeName the name of the theme to create
     * @param parent the parent object
     * @since 4.3
     */
    explicit Theme(const QString &themeName, QObject *parent = nullptr);

    ~Theme() override;

    /**
     * Sets the current theme being used.
     */
    void setThemeName(const QString &themeName);

    /**
     * @return the name of the theme.
     */
    QString themeName() const;

    /**
     * Retrieve the path for an SVG image in the current theme.
     *
     * @param name the name of the file in the theme directory (without the
     *           ".svg" part or a leading slash)
     * @return the full path to the requested file for the current theme
     */
    QString imagePath(const QString &name) const;

    /**
     * Retrieves the default wallpaper associated with this theme.
     *
     * @param size the target height and width of the wallpaper; if an invalid size
     *           is passed in, then a default size will be provided instead.
     * @return the full path to the wallpaper image
     */
    QString wallpaperPath(const QSize &size = QSize()) const;

    Q_INVOKABLE QString wallpaperPathForSize(int width = -1, int height = -1) const;

    /**
     * Checks if this theme has an image named in a certain way
     *
     * @param name the name of the file in the theme directory (without the
     *           ".svg" part or a leading slash)
     * @return true if the image exists for this theme
     */
    bool currentThemeHasImage(const QString &name) const;

    /**
     * Returns the color scheme configurationthat goes along this theme.
     * This can be used with KStatefulBrush and KColorScheme to determine
     * the proper colours to use along with the visual elements in this theme.
     */
    KSharedConfigPtr colorScheme() const;

    /**
     * Returns the text color to be used by items resting on the background
     *
     * @param role which role (usage pattern) to get the color for
     * @param group which group we want a color of
     */
    QColor color(ColorRole role, ColorGroup group = NormalColorGroup) const;

    /**
     * Tells the theme whether to follow the global settings or use application
     * specific settings
     *
     * @param useGlobal pass in true to follow the global settings
     */
    void setUseGlobalSettings(bool useGlobal);

    /**
     * @return true if the global settings are followed, false if application
     * specific settings are used.
     */
    bool useGlobalSettings() const;

    /**
     * Provides a Plasma::Theme-themed stylesheet for hybrid (web / native Plasma) widgets.
     *
     * You can use this method to retrieve a basic default stylesheet, or to theme your
     * custom stylesheet you use for example in Plasma::WebView. The QString you can pass
     * into this method does not have to be a valid stylesheet, in fact you can use this
     * method to replace color placeholders with the theme's color in any QString.
     *
     * In order to use this method with a custom stylesheet, just put for example %textcolor
     * in your QString and it will be replaced with the theme's text (or foreground) color.
     *
     * Just like in many other methods for retrieving theme information, do not forget to
     * update your stylesheet upon the themeChanged() signal.
     *
     * The following tags will be replaced by corresponding colors from Plasma::Theme:
     *
     * %textcolor
     * %backgroundcolor
     * %buttonbackgroundcolor
     *
     * %link
     * %activatedlink
     * %hoveredlink
     * %visitedlink
     *
     * %fontfamily
     * %fontsize
     * %smallfontsize
     *
     * @param css a stylesheet to theme, leave empty for a default stylesheet containing
     * theming for some commonly used elements, body text and links, for example.
     *
     * @return a piece of CSS that sets the most commonly used style elements to a theme
     * matching Plasma::Theme.
     *
     * @since 4.5
     */
    QString styleSheet(const QString &css = QString()) const;

    /**
     * Returns a QPalette with the colors set as defined by the theme.
     * @since 5.68
     */
    QPalette palette() const;

    /**
     * This is an overloaded member provided to check with file timestamp
     * where cache is still valid.
     *
     * @param key the name to use in the cache for this image
     * @param pix the pixmap object to populate with the resulting data if found
     * @param lastModified if non-zero, the time stamp is also checked on the file,
     *                     and must be newer than the timestamp to be loaded
     *
     * @note Since KF 5.75, a lastModified value of 0 is deprecated. If used, it
     *       will now always return false. Use a proper file timestamp instead
     *       so modification can be properly tracked.
     *
     * @return true when pixmap was found and loaded from cache, false otherwise
     * @since 4.3
     **/
    bool findInCache(const QString &key, QPixmap &pix, unsigned int lastModified = 0);

    /**
     * Insert specified pixmap into the cache.
     * If the cache already contains pixmap with the specified key then it is
     * overwritten.
     *
     * @param key the name to use in the cache for this pixmap
     * @param pix the pixmap data to store in the cache
     **/
    void insertIntoCache(const QString &key, const QPixmap &pix);

    /**
     * Insert specified pixmap into the cache.
     * If the cache already contains pixmap with the specified key then it is
     * overwritten.
     * The actual insert is delayed for optimization reasons and the id
     * parameter is used to discard repeated inserts in the delay time, useful
     * when for instance the graphics to insert comes from a quickly resizing
     * object: the frames between the start and destination sizes aren't
     * useful in the cache and just cause overhead.
     *
     * @param key the name to use in the cache for this pixmap
     * @param pix the pixmap data to store in the cache
     * @param id a name that identifies the caller class of this function in an unique fashion.
     *           This is needed to limit disk writes of the cache.
     *           If an image with the same id changes quickly,
     *           only the last size where insertIntoCache was called is actually stored on disk
     * @since 4.3
     **/
    void insertIntoCache(const QString &key, const QPixmap &pix, const QString &id);

    /**
     * Sets the maximum size of the cache (in kilobytes). If cache gets bigger
     * the limit then some entries are removed
     * Setting cache limit to 0 disables automatic cache size limiting.
     *
     * Note that the cleanup might not be done immediately, so the cache might
     *  temporarily (for a few seconds) grow bigger than the limit.
     **/
    void setCacheLimit(int kbytes);

#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 78)
    /**
     * Tries to load the rect of a sub element from a disk cache
     *
     * @param image path of the image we want to check
     * @param element sub element we want to retrieve
     * @param rect output parameter of the element rect found in cache
     *           if not found or if we are sure it doesn't exist it will be QRect()
     * @return true if the element was found in cache or if we are sure the element doesn't exist
     **/
    PLASMA_DEPRECATED_VERSION(5, 78, "Rects Cache public API is deprecated")
    bool findInRectsCache(const QString &image, const QString &element, QRectF &rect) const;

    /**
     * Returns a list of all keys of cached rects for the given image.
     *
     * @param image path of the image for which the keys should be returned
     *
     * @return a QStringList whose elements are the entry keys in the rects cache
     *
     * @since 4.6
     */
    PLASMA_DEPRECATED_VERSION(5, 78, "Rects Cache public API is deprecated")
    QStringList listCachedRectKeys(const QString &image) const;

    /**
     * Inserts a rectangle of a sub element of an image into a disk cache
     *
     * @param image path of the image we want to insert information
     * @param element sub element we want insert the rect
     * @param rect element rectangle
     **/
    PLASMA_DEPRECATED_VERSION(5, 78, "Rects Cache public API is deprecated")
    void insertIntoRectsCache(const QString &image, const QString &element, const QRectF &rect);

    /**
     * Discards all the information about a given image from the rectangle disk cache
     *
     * @param image the path to the image the cache is associated with
     **/
    PLASMA_DEPRECATED_VERSION(5, 78, "Rects Cache public API is deprecated")
    void invalidateRectsCache(const QString &image);

    /**
     * Frees up memory used by cached information for a given image without removing
     * the permanent record of it on disk.
     * @see invalidateRectsCache
     *
     * @param image the path to the image the cache is associated with
     */
    PLASMA_DEPRECATED_VERSION(5, 78, "Rects Cache public API is deprecated")
    void releaseRectsCache(const QString &image);
#endif

#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 67)
    /**
     * @return plugin info for this theme, with information such as
     * name, description, author, website etc
     * @since 5.0
     *
     * @deprecated since 5.67, use KPluginMetaData
     */
    PLASMA_DEPRECATED_VERSION(5, 67, "Use KPluginMetaData metadata()")
    KPluginInfo pluginInfo() const;
#endif

    /**
     * @return plugin metadata for this theme, with information such as
     * name, description, author, website etc
     * @since 5.95
     */
    KPluginMetaData metadata() const;

    /**
     * @return The default application font
     * @since 5.0
     */
    QFont defaultFont() const;

    /**
     * @return The smallest readable font
     * @since 5.0
     */
    QFont smallestFont() const;

    /** This method allows Plasma to enable and disable the background
     * contrast effect for a given theme, improving readability. The
     * value is read from the "enabled" key in the "ContrastEffect"
     * group in the Theme's metadata file.
     * The configuration in the metadata.desktop file of the theme
     * could look like this (for a lighter background):
     * \code
     * [ContrastEffect]
     * enabled=true
     * contrast=0.45
     * intensity=0.45
     * saturation=1.7
     * \endcode
     * @return Whether or not to enable the contrasteffect
     * @since 5.0
     */
    bool backgroundContrastEnabled() const;

    /** This method allows Plasma to enable and disable the adaptive
     * transparency option of the panel, which allows user to decide
     * whether the panel should be always transparent, always opaque
     * or only opaque when a window is maximized.
     * The configuration in the metadata.desktop file of the theme
     * could look like this (for a lighter background):
     * \code
     * [AdaptiveTransparency]
     * enabled=true
     * \endcode
     * @return Whether or not to enable the adaptive transparency
     * @since 5.20
     */
    bool adaptiveTransparencyEnabled() const;

    /** This method allows Plasma to set a background contrast effect
     * for a given theme, improving readability. The value is read
     * from the "contrast" key in the "ContrastEffect" group in the
     * Theme's metadata file.
     * @return The contrast provided to the contrasteffect
     * @since 5.0
     * @see backgroundContrastEnabled
     */
    qreal backgroundContrast() const;

    /** This method allows Plasma to set a background contrast effect
     * for a given theme, improving readability. The value is read
     * from the "intensity" key in the "ContrastEffect" group in the
     * Theme's metadata file.
     * @return The intensity provided to the contrasteffect
     * @since 5.0
     * @see backgroundContrastEnabled
     */
    qreal backgroundIntensity() const;

    /** This method allows Plasma to set a background contrast effect
     * for a given theme, improving readability. The value is read
     * from the "saturation" key in the "ContrastEffect" group in the
     * Theme's metadata file.
     * @return The saturation provided to the contrasteffect
     * @since 5.0
     * @see backgroundContrastEnabled
     */
    qreal backgroundSaturation() const;

    /** This method allows Plasma to enable and disable the blurring
     * of what is behind the background for a given theme. The
     * value is read from the "enabled" key in the "BlurBehindEffect"
     * group in the Theme's metadata file. Default is @c true.
     *
     * The configuration in the metadata.desktop file of the theme
     * could look like this:
     * \code
     * [BlurBehindEffect]
     * enabled=false
     * \endcode
     * @return Whether or not to enable blurring of what is behind
     * @since 5.57
     */
    bool blurBehindEnabled() const;

    /**
     * Returns the size of the letter "M" as rendered on the screen with the given font.
     * This values gives you a base size that:
     * * scales dependent on the DPI of the screen
     * * Scales with the default font as set by the user
     * You can use it like this in QML Items:
     * \code
     * Item {
     *     width: PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height
     *     height: width
     * }
     * \endcode
     * This allows you to dynamically scale elements of your user interface with different font settings and
     * different physical outputs (with different DPI).
     * @param font The font to use for the metrics.
     * @return The size of the letter "M" as rendered on the screen with the given font.
     * @since 5.0
     */
    Q_INVOKABLE QSizeF mSize(const QFont &font = QGuiApplication::font()) const;

    QString backgroundPath(const QString &image) const;

    static QPalette globalPalette();

Q_SIGNALS:
    /**
     * Emitted when the user changes the theme. Stylesheet usage, colors, etc. should
     * be updated at this point. However, SVGs should *not* be repainted in response
     * to this signal; connect to Svg::repaintNeeded() instead for that, as Svg objects
     * need repainting not only when themeChanged() is emitted; moreover Svg objects
     * connect to and respond appropriately to themeChanged() internally, emitting
     * Svg::repaintNeeded() at an appropriate time.
     */
    void themeChanged();

    /** Notifier for change of defaultFont property */
    void defaultFontChanged();
    /** Notifier for change of smallestFont property */
    void smallestFontChanged();

private:
    friend class SvgPrivate;
    friend class FrameSvg;
    friend class FrameSvgPrivate;
    friend class ThemePrivate;
    ThemePrivate *d;
};

} // Plasma namespace

#endif // multiple inclusion guard
