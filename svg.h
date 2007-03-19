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

#ifndef PLASMA_SVG_H
#define PLASMA_SVG_H

#include <QtCore/QObject>

#include <kdemacros.h>

namespace Plasma
{

class KDE_EXPORT Svg : public QObject
{
    Q_OBJECT

    public:
        /**
         * Constructs an SVG object that implicitly shares and caches rendering
         * As opposed to QSvgRenderer, which this class uses internally,
         * Plasma::Svg represents an image generated from an SVG. As such, it
         * has a related size and transform matrix (the latter being provided
         * by the painter used to paint the image).
         *
         * The size is initialized to be the SVG's native size.
         *
         * @related Plasma::Theme
         * @arg imagePath the image to show, used to load the image from
         * Plasma::Theme
         * @arg parent options QObject to parent this to
         */
        explicit Svg( const QString& imagePath, QObject* parent = 0 );
        ~Svg();

        /**
         * Paints the SVG represented by this object
         * @arg painter the QPainter to use
         * @arg point the position to start drawing; the entire svg will be
         *      drawn starting at this point.
         */
        void paint( QPainter* painter, const QPoint& point );

        /**
         * Paints the SVG represented by this object
         * @arg painter the QPainter to use
         * @arg x the horizontal coordinate to start painting from
         * @arg y the vertical coordinate to start painting from
         */
        void paint( QPainter* painter, int x, int y );

        /**
         * Paints the SVG represented by this object
         * @arg painter the QPainter to use
         * @arg rect the rect to draw into; if small than the current size
         *           of the 
         *      drawn starting at this point.
         */
        void paint( QPainter* painter, const QRect& rect );

        /**
         * Resizes the rendered image. Rendering will actually take place on
         * the next call to paint.
         * @arg width the new width
         * @arg height the new height
         **/
        void resize( int width, int height );

        /**
         * Resizes the rendered image. Rendering will actually take place on
         * the next call to paint.
         * @arg size the new size of the image
         **/
        void resize( const QSize& size );

    Q_SIGNALS:
        void repaintNeeded();

    private Q_SLOTS:
        void themeChanged();

    private:
        class Private;
        Private* d;
};

} // Plasma namespace

#endif // multiple inclusion guard

