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

#ifndef PLASMA_PANELSVG_H
#define PLASMA_PANELSVG_H

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

class PLASMA_EXPORT PanelSvg : public Svg
{
    Q_OBJECT
    public:
        /**
         * These flags represents what borders should be drawn
         */
        enum EnabledBorder { TopBorder = 1,
                             BottomBorder = 2,
                             LeftBorder = 4,
                             RightBorder = 8,
                             AllBorders = TopBorder | BottomBorder |
                                              LeftBorder | RightBorder
                           };
        Q_DECLARE_FLAGS(EnabledBorders, EnabledBorder)

        /**
         * Constructs a new PanelSvg that paints the proper named subelements
         * as borders. It may also be used as a regular Plasma::Svg object
         * for direct access to elements in the Svg.
         *
         * @arg parent options QObject to parent this to
         *
         * @related Plasma::Theme
         */
        explicit PanelSvg(QObject* parent = 0);
        ~PanelSvg();

        /**
         * Loads a new Svg
         * @arg imagePath the new file
         */
        void setImagePath(const QString& path);

        /**
         * Sets what borders should be painted
         * @arg flags borders we want to paint
         */
        void setEnabledBorders(const EnabledBorders borders);

        /**
         * Convenience method to get the enabled borders
         * @return what borders are painted
         */
        EnabledBorders enabledBorders() const;

        /**
         * Resize the panel maintaining the same border size
         * @arg size the new size of the panel
         */
        void resizePanel(const QSizeF& size);

        /**
         * Returns the margin size given the margin edge we want
         * @arg edge the margin edge we want, top, bottom, left or right
         * @return the margin size
         */
        qreal marginSize(const Plasma::MarginEdge edge) const;

        /**
         * Convenience method that extracts the size of the four margins
         * in the four output parameters
         * @arg left left margin size
         * @arg top top margin size
         * @arg right right margin size
         * @arg bottom bottom margin size
         */
        void getMargins(qreal &left, qreal &top, qreal &right, qreal &bottom) const;

        /**
         * Sets the prefix (@see setElementPrefix) to 'north', 'south', 'west' and 'east'
         * when the location is TopEdge, BottomEdge, LeftEdge and RightEdge,
         * respectively. Clears the prefix in other cases.
         * @arg location location
         */
        void setElementPrefix(Plasma::Location location);

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
        void setElementPrefix(const QString & prefix);

        /**
         * Returns the prefix for SVG elements of the PanelSvg
         * @return the prefix
         */
        QString prefix();

        /**
         * Returns a monochrome mask that tightly contains the fully opaque areas of the svg
         * @return a monochrome bitmap of opaque areas
         */
        QBitmap mask() const;

       /**
        * Sets whether saving all the rendered prefixes in a cache or not
        * @arg cache if use the cache or not
        */
       void setCacheAllRenderedPanels(bool cache);

       /**
        * @return if all the different prefixes should be kept in a cache when rendered
        */
       bool cacheAllRenderedPanels() const;

       /**
        * Deletes the internal cache freeing memory: use this if you want to switch the rendered 
        * element and you don't plan to switch back to the previous one for a long time and you
        * used setUseCache(true)
        */
       void clearCache();

        /**
         * Paints the loaded SVG with the elements that represents the border
         * @arg painter the QPainter to use
         * @arg rect the exposed rect to draw into
         */
        Q_INVOKABLE void paintPanel(QPainter* painter, const QRectF& rect, const QPointF& pos = QPointF(0, 0));

    Q_SIGNALS:
        void repaintNeeded();

    private:
        class Private;
        Private * const d;

        Q_PRIVATE_SLOT(d, void updateSizes())
};

} // Plasma namespace

Q_DECLARE_OPERATORS_FOR_FLAGS(Plasma::PanelSvg::EnabledBorders)

#endif // multiple inclusion guard
