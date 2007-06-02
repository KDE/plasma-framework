/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include <plasma/plasma_export.h>

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
         * @return the name of the theme. "default" is none set.
         */
        QString themeName() const;

        /**
         * Retrieve the path for an SVG image in the current theme.
         *
         * @arg name the name of the file in the theme directory (without the
         *           ".svg" part or a leading slash)
         * @return the full path to the requested file for the current theme
         */
        QString image( const QString& name ) const;

    Q_SIGNALS:
        /**
         * Emitted when the user changes the theme. SVGs should be reloaded at
         * that point
         */
        void changed();

    private:
        class Private;
        Private* const d;
};

} // Plasma namespace

#endif // multiple inclusion guard

