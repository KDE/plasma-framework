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

#ifndef PLASMA_THEME_H
#define PLASMA_THEME_H

#include <QtCore/QObject>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>

#include <KDE/KSharedConfig>

#include <plasma/plasma_export.h>
#include <plasma/packagestructure.h>

namespace Plasma
{
/**
 * @short Interface to the Plasma theme
 *
 * Accessed via Plasma::Theme::self() e.g:
 * \code
 * QString image = Plasma::Theme::self()->image("widgets/clock")
 * \endcode
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
    Q_PROPERTY( QString themeName READ themeName )

    public:
        /**
         * Singleton pattern accessor
         **/
        static Theme* self();

        /**
         * Default constructor. Usually you want to use the singleton instead.
         */
        explicit Theme( QObject* parent = 0 );
        ~Theme();

        /**
         * @return a package structure representing a Theme
         */
        static PackageStructure::Ptr packageStructure();

        /**
         * Sets the application the theme setting is associated with. This
         * allows for individual applications that use libplasma to have the
         * theme set independantly.
         *
         * @param appname name of the application
         **/
        void setApplication(const QString &appname);

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
         * @arg name the name of the file in the theme directory (without the
         *           ".svg" part or a leading slash)
         * @return the full path to the requested file for the current theme
         */
        Q_INVOKABLE QString image( const QString& name ) const;

        /**
         * Returns the color scheme configurationthat goes along this theme.
         * This can be used with KStatefulBrush and KColorScheme to determine
         * the proper colours to use along with the visual elements in this theme.
         */
        Q_INVOKABLE KSharedConfigPtr colors() const;

        /**
         * Returns the text color to be used by items resting on the background
         */
        Q_INVOKABLE QColor textColor() const;

        /**
         * Returns the background color to be used by items resting on the background
         */
        Q_INVOKABLE QColor backgroundColor() const;

        /**
         * Sets the default font to be used with themed items. Defaults to
         * the application wide default font.
         */
        Q_INVOKABLE void setFont(const QFont &font);

        /**
         * Returns the font to be used by themed items
         */
        Q_INVOKABLE QFont font() const;

        /**
         * Returns the font metrics for the font to be used by themed items
         */
        Q_INVOKABLE QFontMetrics fontMetrics() const;

    Q_SIGNALS:
        /**
         * Emitted when the user changes the theme. SVGs should be reloaded at
         * that point
         */
        void changed();

    public Q_SLOTS:
        /**
         * Notifies the Theme object that the theme settings have changed
         * and should be read from the config file
         **/
        void settingsChanged();

    private Q_SLOTS:
        void compositingChanged();

    private:
        class Private;
        Private* const d;
};

} // Plasma namespace

#endif // multiple inclusion guard

