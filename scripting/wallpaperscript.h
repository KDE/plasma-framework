/*
 *   Copyright 2009 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2009 by Petri Damsten <damu@iki.fi>
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

#ifndef PLASMA_WALLPAPERSCRIPT_H
#define PLASMA_WALLPAPERSCRIPT_H

#include <kgenericfactory.h>
#include <kplugininfo.h>

#include <plasma/plasma_export.h>
#include <plasma/scripting/scriptengine.h>
#include <plasma/wallpaper.h>

namespace Plasma
{

class WallpaperScriptPrivate;
class Service;

/**
 * @class WallpaperScript plasma/scripting/wallpaperscript.h <Plasma/Scripting/WallpaperScript>
 *
 * @short Provides a restricted interface for scripting a Wallpaper
 */
class PLASMA_EXPORT WallpaperScript : public ScriptEngine
{
    Q_OBJECT

public:
    /**
     * Default constructor for a WallpaperScript.
     * Subclasses should not attempt to access the Plasma::Wallpaper
     * associated with this WallpaperScript in the constructor. All
     * such set up that requires the Wallpaper itself should be done
     * in the init() method.
     */
    explicit WallpaperScript(QObject *parent = 0);
    ~WallpaperScript();

    /**
     * Sets the Plasma::Wallpaper associated with this WallpaperScript
     */
    void setWallpaper(Wallpaper *wallpaper);

    /**
     * Returns the Plasma::Wallpaper associated with this script component
     */
    Wallpaper *wallpaper() const;

    /**
     * This method is called once the wallpaper is loaded or mode is changed.
     *
     * The mode can be retrieved using the renderingMode() method.
     *
     * @param config Config group to load settings
     **/
    virtual void initWallpaper(const KConfigGroup &config);

    /**
     * This method is called when the wallpaper should be painted.
     *
     * @param painter the QPainter to use to do the painting
     * @param exposedRect the rect to paint within
     **/
    virtual void paint(QPainter *painter, const QRectF &exposedRect);

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

protected:
    /**
     * @return absolute path to the main script file for this wallpaper
     */
    QString mainScript() const;

    /**
     * @return the Package associated with this wallpaper which can
     *         be used to request resources, such as images and
     *         interface files.
     */
    const Package *package() const;

    /**
     * @return the KPluginInfo associated with this wallpaper
     */
    KPluginInfo description() const;

    bool isInitialized() const;
    QRectF boundingRect() const;
    DataEngine *dataEngine(const QString &name) const;
    void setResizeMethodHint(Wallpaper::ResizeMethod resizeMethod);
    void setTargetSizeHint(const QSizeF &targetSize);
    void setConfigurationRequired(bool needsConfiguring, const QString &reason = QString());
    void render(const QString &sourceImagePath, const QSize &size,
                Wallpaper::ResizeMethod resizeMethod = Plasma::Wallpaper::ScaledResize,
                const QColor &color = QColor(0, 0, 0));
    void setUsingRenderingCache(bool useCache);
    bool findInCache(const QString &key, QImage &image, unsigned int lastModified = 0);
    void insertIntoCache(const QString& key, const QImage &image);
    void setContextualActions(const QList<QAction*> &actions);
    void update(const QRectF &exposedArea);
    void configNeedsSaving();

protected Q_SLOTS:
    virtual void renderCompleted(const QImage &image);
    virtual void urlDropped(const KUrl &url);

private:
    WallpaperScriptPrivate *const d;
};

#define K_EXPORT_PLASMA_WALLPAPERSCRIPTENGINE(libname, classname) \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("plasma_wallpaperscriptengine_" #libname))

} //Plasma namespace

#endif
