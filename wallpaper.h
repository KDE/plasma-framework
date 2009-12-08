/*
 *   Copyright 2008 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Petri Damsten <damu@iki.fi>

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

#ifndef PLASMA_WALLPAPER_H
#define PLASMA_WALLPAPER_H

#include <kplugininfo.h>

#include <plasma/plasma.h>
#include <plasma/packagestructure.h>
#include <plasma/version.h>

namespace Plasma
{

class DataEngine;
class WallpaperPrivate;
class Package;

/**
 * @class Wallpaper plasma/wallpaper.h <Plasma/Wallpaper>
 *
 * @short The base Wallpaper class
 *
 * "Wallpapers" are components that paint the background for Containments that
 * do not provide their own background rendering.
 *
 * Wallpaper plugins are registered using .desktop files. These files should be
 * named using the following naming scheme:
 *
 *     plasma-wallpaper-\<pluginname\>.desktop
 *
 * If a wallpaper plugin provides more than on mode (e.g. Single Image, Wallpaper)
 * it should include a Actions= entry in the .desktop file, listing the possible
 * actions. An actions group should be included to provide for translatable names.
 */

class PLASMA_EXPORT Wallpaper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QRectF boundingRect READ boundingRect WRITE setBoundingRect)
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString pluginName READ pluginName)
    Q_PROPERTY(QString icon READ icon)
    Q_PROPERTY(KServiceAction renderingMode READ renderingMode)
    Q_PROPERTY(QList<KServiceAction> listRenderingModes READ listRenderingModes)
    Q_PROPERTY(bool usingRenderingCache READ isUsingRenderingCache WRITE setUsingRenderingCache)

    public:
        /**
         * Various resize modes supported by the built in image renderer
         */
        enum ResizeMethod {
            ScaledResize /**< Scales the image to fit the full area*/,
            CenteredResize /**< Centers the image within the area */,
            ScaledAndCroppedResize /**< Scales and crops the image, preserving the aspect ratio */,
            TiledResize /**< Tiles the image to fill the area */,
            CenterTiledResize /**< Tiles the image to fill the area, starting with a centered tile */,
            MaxpectResize /**< Best fit resize */
        };
        Q_ENUMS(ResizeMethod)

        /**
         * Default constructor for an empty or null wallpaper
         */
        explicit Wallpaper(QObject * parent = 0);

        ~Wallpaper();

        /**
         * Returns a list of all known wallpapers.
         *
         * @arg formFactor the format of the wallpaper being search for (e.g. desktop)
         * @return list of wallpapers
         **/
        static KPluginInfo::List listWallpaperInfo(const QString &formFactor = QString());

        /**
         * Returns a list of all known wallpapers that can accept the given mimetype
         * @arg mimetype the mimetype to search for
         * @arg formFactor the format of the wallpaper being search for (e.g. desktop)
         * @return list of wallpapers
         */
        static KPluginInfo::List listWallpaperInfoForMimetype(const QString &mimetype,
                                                              const QString &formFactor = QString());

        /**
         * Attempts to load a wallpaper
         *
         * Returns a pointer to the wallpaper if successful.
         * The caller takes responsibility for the wallpaper, including
         * deleting it when no longer needed.
         *
         * @param name the plugin name, as returned by KPluginInfo::pluginName()
         * @param args to send the wallpaper extra arguments
         * @return a pointer to the loaded wallpaper, or 0 on load failure
         **/
        static Wallpaper *load(const QString &name, const QVariantList &args = QVariantList());

        /**
         * Attempts to load a wallpaper
         *
         * Returns a pointer to the wallpaper if successful.
         * The caller takes responsibility for the wallpaper, including
         * deleting it when no longer needed.
         *
         * @param info KPluginInfo object for the desired wallpaper
         * @param args to send the wallpaper extra arguments
         * @return a pointer to the loaded wallpaper, or 0 on load failure
         **/
        static Wallpaper *load(const KPluginInfo &info, const QVariantList &args = QVariantList());

        /**
         * Returns the Package specialization for wallpapers. May be queried for 'preferred'
         * which will return the preferred wallpaper image path given the associated Wallpaper
         * object, if any.
         *
         * @param paper the Wallpaper object to associated the PackageStructure with,
         *              which will then use the Wallpaper object to define things such as
         *              default size and resize methods.
         */
        static PackageStructure::Ptr packageStructure(Wallpaper *paper = 0);

        /**
         * Returns the user-visible name for the wallpaper, as specified in the
         * .desktop file.
         *
         * @return the user-visible name for the wallpaper.
         **/
        QString name() const;

        /**
         * Accessor for the associated Package object if any.
         *
         * @return the Package object, or 0 if none
         **/
        const Package *package() const;

        /**
         * Returns the plugin name for the wallpaper
         */
        QString pluginName() const;

        /**
         * Returns the icon related to this wallpaper
         **/
        QString icon() const;

        /**
         * @return the currently active rendering mode
         */
        KServiceAction renderingMode() const;


        /**
         * Sets the rendering mode for this wallpaper.
         * @param mode One of the modes supported by the plugin,
         *             or an empty string for the default mode.
         */
        void setRenderingMode(const QString &mode);

        /**
         * Returns modes the wallpaper has, as specified in the
         * .desktop file.
         */
        QList<KServiceAction> listRenderingModes() const;

        /**
         * @return true if initialized (usually by calling restore), false otherwise
         */
        bool isInitialized() const;

        /**
         * Returns bounding rectangle
         */
        QRectF boundingRect() const;

        /**
         * Sets bounding rectangle
         */
        void setBoundingRect(const QRectF &boundingRect);

       /**
         * This method is called when the wallpaper should be painted.
         *
         * @param painter the QPainter to use to do the painting
         * @param exposedRect the rect to paint within
         **/
        virtual void paint(QPainter *painter, const QRectF &exposedRect) = 0;

        /**
         * This method should be called once the wallpaper is loaded or mode is changed.
         * @param config Config group to load settings
         * @see init
         **/
        void restore(const KConfigGroup &config);

        /**
         * This method is called when settings need to be saved.
         * @param config Config group to save settings
         **/
        virtual void save(KConfigGroup &config);

        /**
         * Returns a widget that can be used to configure the options (if any)
         * associated with this wallpaper. It will be deleted by the caller
         * when it complete. The default implementation returns a null pointer.
         *
         * To signal that settings have changed connect to
         * settingsChanged(bool modified) in @p parent.
         *
         * @code connect(this, SIGNAL(settingsChanged(bool), parent, SLOT(settingsChanged(bool)))
         * @endcode
         *
         * Emit settingsChanged(true) when the settings are changed and false when the original state is restored.
         *
         * Implementation detail note: for best visual results, use a QGridLayout with two columns,
         * with the option labels in column 0
         */
        virtual QWidget *createConfigurationInterface(QWidget *parent);

        /**
         * Mouse move event. To prevent further propagation of the event,
         * the event must be accepted.
         *
         * @param event the mouse event object
         */
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

        /**
         * Mouse press event. To prevent further propagation of the even,
         * and to receive mouseMoveEvents, the event must be accepted.
         *
         * @param event the mouse event object
         */
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

        /**
         * Mouse release event. To prevent further propagation of the event,
         * the event must be accepted.
         *
         * @param event the mouse event object
         */
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        /**
         * Mouse wheel event. To prevent further propagation of the event,
         * the event must be accepted.
         *
         * @param event the wheel event object
         */
        virtual void wheelEvent(QGraphicsSceneWheelEvent *event);

        /**
         * Loads the given DataEngine
         *
         * Tries to load the data engine given by @p name.  Each engine is
         * only loaded once, and that instance is re-used on all subsequent
         * requests.
         *
         * If the data engine was not found, an invalid data engine is returned
         * (see DataEngine::isValid()).
         *
         * Note that you should <em>not</em> delete the returned engine.
         *
         * @param name Name of the data engine to load
         * @return pointer to the data engine if it was loaded,
         *         or an invalid data engine if the requested engine
         *         could not be loaded
         *
         * @since 4.3
         */
        Q_INVOKABLE DataEngine *dataEngine(const QString &name) const;

        /**
         * @return true if the wallpaper currently needs to be configured,
         *         otherwise, false
         * @since 4.3
         */
        bool configurationRequired() const;

        /**
         * @return true if disk caching is turned on.
         * @since 4.3
         */
        bool isUsingRenderingCache() const;

        /**
         * Allows one to set rendering hints that may differ from the actualities of the
         * Wallpaper's current state, allowing for better selection of papers from packages,
         * for instance.
         *
         * @arg resizeMethod The resize method to assume will be used for future wallpaper
         *                   scaling; may later be changed by calls to render()
         *
         * @since 4.3
         */
        void setResizeMethodHint(Wallpaper::ResizeMethod resizeMethod);

        /**
         * Allows one to set rendering hints that may differ from the actualities of the
         * Wallpaper's current state, allowing for better selection of papers from packages,
         * for instance.
         *
         * @arg targetSize The size to assume will be used for future wallpaper scaling
         *
         * @since 4.3
         */
        void setTargetSizeHint(const QSizeF &targetSize);

        /**
         * Returns a list of wallpaper contextual actions (nothing by default)
         */
        QList<QAction*> contextualActions() const;

    Q_SIGNALS:
        /**
         * This signal indicates that wallpaper needs to be repainted.
         */
        void update(const QRectF &exposedArea);

        /**
         * Emitted when the user wants to configure/change the wallpaper.
         * @since 4.3
         */
        void configureRequested();

        /**
         * Emitted when the state of the wallpaper requiring configuration
         * changes.
         * @since 4.3
         */
        void configurationRequired(bool needsConfig);

        /**
         * Emitted when the configuration of the wallpaper needs to be saved
         * to disk.
         * @since 4.3
         */
        void configNeedsSaving();

        /**
         * Emitted when a wallpaper image render is completed.
         * @since 4.3
         */
        void renderCompleted(const QImage &image);

        /**
         * Emitted when a URL matching X-Plasma-DropMimeTypes is dropped on the wallpaper
         *
         * @arg url the URL of the dropped file
         * @since 4.4
         */
        void urlDropped(const KUrl &url);

        /**
         * @internal
         */
        void renderHintsChanged();

    protected:
        /**
         * This constructor is to be used with the plugin loading systems
         * found in KPluginInfo and KService. The argument list is expected
         * to have one element: the KService service ID for the desktop entry.
         *
         * @param parent a QObject parent; you probably want to pass in 0
         * @param args a list of strings containing one entry: the service id
         */
        Wallpaper(QObject *parent, const QVariantList &args);

        /**
         * This method is called once the wallpaper is loaded or mode is changed.
         *
         * The mode can be retrieved using the renderingMode() method.
         *
         * @param config Config group to load settings
         **/
        virtual void init(const KConfigGroup &config);

        /**
         * When the wallpaper needs to be configured before being usable, this
         * method can be called to denote that action is required
         *
         * @param needsConfiguring true if the applet needs to be configured,
         *                         or false if it doesn't
         * @param reason a translated message for the user explaining that the
         *               applet needs configuring; this should note what needs
         *               to be configured
         * @since 4.3
         */
        void setConfigurationRequired(bool needsConfiguring, const QString &reason = QString());

        /**
         * Renders the wallpaper asyncronously with the given parameters. When the rendering is
         * complete, the renderCompleted signal is emitted.
         *
         * @param sourceImagePath the path to the image file on disk. Common image formats such as
         *                        PNG, JPEG and SVG are supported
         * @param size the size to render the image as
         * @param resizeMethod the method to use when resizing the image to fit size, @see
         *                     ResizeMethod
         * @param color the color to use to pad the rendered image if it doesn't take up the
         *              entire size with the given ResizeMethod
         * @since 4.3
         */
        void render(const QString &sourceImagePath, const QSize &size,
                    Wallpaper::ResizeMethod resizeMethod = ScaledResize,
                    const QColor &color = QColor(0, 0, 0));

        /**
         * Sets whether or not to cache on disk the results of calls to render. If the wallpaper
         * changes often or is innexpensive to render, then it's probably best not to cache them.
         *
         * The default is not to cache.
         *
         * @see render
         * @param useCache true to cache rendered papers on disk, false not to cache
         * @since 4.3
         */
        void setUsingRenderingCache(bool useCache);

        /**
         * Tries to load pixmap with the specified key from cache.
         *
         * @param key the name to use in the cache for this image
         * @param image the image object to populate with the resulting data if found
         * @param lastModified if non-zero, the time stamp is also checked on the file,
         *                     and must be newer than the timestamp to be loaded
         *
         * @return true when pixmap was found and loaded from cache, false otherwise
         * @since 4.3
         **/
        bool findInCache(const QString &key, QImage &image, unsigned int lastModified = 0);

        /**
         * Insert specified pixmap into the cache if usingRenderingCache.
         * If the cache already contains pixmap with the specified key then it is
         * overwritten.
         *
         * @param key the name use in the cache for this image; if the image is specific
         *            to this wallpaper plugin, consider including the name() as part of
         *            the cache key to avoid collisions with other plugins.
         * @param image the image to store in the cache; passing in a null image will cause
         *              the cached image to be removed from the cache
         *
         * @since 4.3
         **/
        void insertIntoCache(const QString& key, const QImage &image);

        /**
         * Sets the contextual actions for this wallpaper.
         *
         * @param actions A list of contextual actions for this wallpaper
         *
         * @since 4.4
         **/
        void setContextualActions(const QList<QAction*> &actions);

        QList<QAction*> contextActions;

    private:
        Q_PRIVATE_SLOT(d, void renderCompleted(int token, const QImage &image,
                                               const QString &sourceImagePath, const QSize &size,
                                               int resizeMethod, const QColor &color))
        Q_PRIVATE_SLOT(d, void initScript())

        friend class WallpaperPackage;
        friend class WallpaperPrivate;
        friend class WallpaperScript;
        friend class WallpaperWithPaint;
        friend class ContainmentPrivate;
        WallpaperPrivate *const d;
};

} // Plasma namespace

/**
 * Register a wallpaper when it is contained in a loadable module
 */
#define K_EXPORT_PLASMA_WALLPAPER(libname, classname) \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("plasma_wallpaper_" #libname)) \
K_EXPORT_PLUGIN_VERSION(PLASMA_VERSION)

#endif // multiple inclusion guard
