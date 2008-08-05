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

#include <KPluginInfo>

#include <plasma/plasma.h>
#include <plasma/version.h>

namespace Plasma
{
class WallpaperPrivate;

/**
 * @short The base Wallpaper class
 *
 * "Wallpapers" are components that paint the background for Containments that
 * do not provide their own background rendering.
 *
 * Wallpaper plugins are registered using .desktop files. These files should be
 * named using the following naming scheme:
 *
 *     plasma-wallpaper-<pluginname>.desktop
 *
 * If a wallpaper plugin provides more than on mode (e.g. Single Image, Wallpaper)
 * it should include a Actions= entry in the .desktop file, listing the possible
 * actions. An actions group should be included to provide for translatable names.
 */

class PLASMA_EXPORT Wallpaper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QRectF boundingRect READ boundingRect WRITE setBoundingRect)
    //Q_PROPERTY(QStringList formFactors READ formFactors WRITE setFormFactors)
    Q_PROPERTY(QString name READ name)

    public:
        ~Wallpaper();

        /**
         * Returns a list of all known wallpapers.
         *
         * @return list of wallpapers
         **/
        static KPluginInfo::List listWallpaperInfo(const QString &formFactor = QString());

        /**
         * Attempts to load an wallpaper
         *
         * Returns a pointer to the wallpaper if successful.
         * The caller takes responsibility for the wallpaper, including
         * deleting it when no longer needed.
         *
         * @param name the plugin name, as returned by KPluginInfo::pluginName()
         * @param args to send the wallpaper extra arguments
         * @return a pointer to the loaded wallpaper, or 0 on load failure
         **/
        static Wallpaper* load(const QString &name, const QVariantList& args = QVariantList());

        /**
         * Attempts to load an wallpaper
         *
         * Returns a pointer to the wallpaper if successful.
         * The caller takes responsibility for the wallpaper, including
         * deleting it when no longer needed.
         *
         * @param info KPluginInfo object for the desired wallpaper
         * @param args to send the wallpaper extra arguments
         * @return a pointer to the loaded wallpaper, or 0 on load failure
         **/
        static Wallpaper* load(const KPluginInfo& info, const QVariantList& args = QVariantList());

        /**
         * Returns the user-visible name for the wallpaper, as specified in the
         * .desktop file.
         *
         * @return the user-visible name for the wallpaper.
         **/
        QString name() const;

        /**
         * Returns the plugin name for the wallpaper
         */
        QString pluginName() const;

        /**
         * Returns the icon related to this wallpaper
         **/
        QString icon() const;

        /**
         * Returns modes the wallpaper has, as specified in the
         * .desktop file.
         */
        QStringList modes() const;

       /**
         * Returns name for the mode.
         */
        QString modeName(const QString& mode) const;

       /**
         * Returns icon for the mode.
         */
        QString modeIcon(const QString& mode) const;

        /**
         * Returns bounding rectangle
         */
        QRectF boundingRect() const;

        /**
         * Sets bounding rectangle
         */
        void setBoundingRect(const QRectF& boundingRect);

       /**
         * This method is called when the wallpaper should be painted.
         *
         * @param painter the QPainter to use to do the painting
         * @param exposedRect the rect to paint within
         **/
        virtual void paint(QPainter *painter, const QRectF& exposedRect) = 0;

        /**
         * This method is called once the wallpaper is loaded.
         * @param action One of the actions spedified in the desktop file (if any).
         **/
        virtual void init(const QString &action = QString());

        /**
         * Returns widget for configuration dialog.
         */
        virtual QWidget *configuration(QWidget *parent);

    Q_SIGNALS:
        /**
         * This signal indicates that wallpaper needs to be repainted.
         */
        void update(const QRectF &exposedArea);

    protected:
        /**
         * This constructor is to be used with the plugin loading systems
         * found in KPluginInfo and KService. The argument list is expected
         * to have one element: the KService service ID for the desktop entry.
         *
         * @param parent a QObject parent; you probably want to pass in 0
         * @param args a list of strings containing one entry: the service id
         */
        Wallpaper(QObject* parent, const QVariantList& args);

    private:
        WallpaperPrivate* const d;
};

} // Plasma namespace

/**
 * Register an wallpaper when it is contained in a loadable module
 */
#define K_EXPORT_PLASMA_WALLPAPER(libname, classname) \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("plasma_wallpaper_" #libname)) \
K_EXPORT_PLUGIN_VERSION(PLASMA_VERSION)

#endif // multiple inclusion guard
