/*
 *   Copyright 2008 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
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

#ifndef PLASMA_SVGPANEL_H
#define PLASMA_SVGPANEL_H

#include <QtCore/QObject>
#include <QtGui/QPixmap>

#include <plasma/plasma_export.h>

#include <plasma/plasma.h>
#include <plasma/svg.h>

class QPainter;
class QPoint;
class QPointF;
class QRect;
class QRectF;
class QSize;
class QSizeF;
class QMatrix;

namespace Plasma
{

class PLASMA_EXPORT SvgPanel : public QObject
{
    Q_OBJECT
    public:
        /**
         * These flags represents what borders should be drawn
         */
        enum BorderFlag { DrawTopBorder = 1,
                          DrawBottomBorder = 2,
                          DrawLeftBorder = 4,
                          DrawRightBorder = 8,
                          ContentAtOrigin = 16,
                          DrawAllBorders = DrawTopBorder | DrawBottomBorder |
                                           DrawLeftBorder | DrawRightBorder
                        };
        Q_DECLARE_FLAGS(BorderFlags, BorderFlag)

        /**
         * Constructs a new SvgPanel that paints the proper named subelements
         * as borders
         *
         * The size is initialized to be the SVG's native size.
         *
         * @arg imagePath the image to show. If a relative path is passed, then
         *      Plasma::Theme is used to load the SVG.
         * @arg parent options QObject to parent this to
         *
         * @related Plasma::Theme
         */
        explicit SvgPanel(const QString& imagePath = QString(), QObject* parent = 0);
        ~SvgPanel();

        /**
         * Loads a new Svg
         * @arg imagePath the new file
         */
        void setFile(const QString& imagePath);

        /**
         * Convenience method to get the svg filepath and name of svg.
         * @return the svg's filepath including name of the svg.
         */
        QString file() const;

        /**
         * Sets what borders should be painted
         * @arg flags borders we want to paint
         */
        void setBorderFlags(const BorderFlags flags);

        /**
         * Convenience method to get the border flags
         * @return what borders are painted
         */
        BorderFlags borderFlags() const;

        /**
         * Resize the panel maintaining the same border size
         * @arg size the new size of the panel
         */
        void resize(const QSizeF& size);

        /**
         * Returns the margin size given the margin edge we want
         * @arg edge the margin edge we want, top, bottom, left or right
         * @return the margin size
         */
        qreal marginSize(const Plasma::MarginEdge edge) const;

        /**
         * Sets the position of the SvgPanel
         * @arg pos where it should be positioned at
         */
        void setPos( const QPointF& pos );

        /**
         * Returns the position of the SvgPanel
         * @return the position
         */
        QPointF pos() const;

        /**
         * Sets the prefix (@see setPrefix) to 'north', 'south', 'west' and 'east'
         * when the location is TopEdge, BottomEdge, LeftEdge and RightEdge,
         * respectively. Clears the prefix in other cases.
         * @arg location location
         */
        void setLocation(Plasma::Location location);

        /**
         * Returns the set location for the SvgPanel. Returns 0 if no location is set
         * or a custom prefix is set (@see setPrefix)
         * @return the location
         */
        Plasma::Location location() const;

        /**
         * Sets the prefix for the SVG elements to be used for painting. For example,
         * if prefix is 'active', then instead of using the 'top' element of the SVG
         * file to paint the top border, 'active-top' element will be used. The same
         * goes for other SVG elements.
         *
         * If the elements with prefixes are not present, the default ones are used.
         * (for the sake of speed, the test is present only for the 'center' element)
         *
         * Setting the prefix manually resets the location to Floating.
         * If the
         * @arg prefix prefix for the SVG element names
         */
        void setPrefix(const QString & prefix);

        /**
         * Returns the prefix for SVG elements of the SvgPanel
         * @return the prefix
         */
        QString prefix();

        /**
         * Returns a monochrome mask that tightly contains the fully opaque areas of the svg
         * @return a monochrome bitmap of opaque areas
         */
        QBitmap mask() const;

        /**
         * Paints the loaded SVG with the elements that represents the border
         * @arg painter the QPainter to use
         * @arg rect the exposed rect to draw into
         */
        Q_INVOKABLE void paint(QPainter* painter, const QRectF& rect);

    Q_SIGNALS:
        void repaintNeeded();

    private Q_SLOTS:
        //update sizes of the svg elements
        void updateSizes();

    private:
        class Private;
        Private * const d;
};

} // Plasma namespace

Q_DECLARE_OPERATORS_FOR_FLAGS(Plasma::SvgPanel::BorderFlags)

#endif // multiple inclusion guard
