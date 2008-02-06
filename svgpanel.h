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
#include <QPixmap>

#include <plasma/plasma_export.h>

#include <plasma/svg.h>
#include <plasma/layouts/layout.h>

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
        enum BorderFlag { DrawTop = 1,
                          DrawBottom = 2,
                          DrawLeft = 4,
                          DrawRight = 8,
                          ContentAtOrigin = 16
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
        qreal marginSize(const Plasma::Layout::MarginEdge edge) const;

        /**
         * Paints the loaded SVG with the elements that represents the border
         * @arg painter the QPainter to use
         * @arg rect the exposed rect to draw into
         */
        Q_INVOKABLE void paint(QPainter* painter, const QRectF& rect);

    private slots:
        //update sizes of the svg elements
        void updateSizes();

    private:
        BorderFlags bFlags;
        QPixmap *cachedBackground;
        Svg *m_background;
        QSizeF panelSize;

        //measures
        int topHeight;
        int topWidth;
        int leftWidth;
        int leftHeight;
        int rightWidth;
        int bottomHeight;

        //size of the svg where the size of the "center"
        //element is contentWidth x contentHeight
        QSizeF scaledSize;
        bool noBorderPadding : 1;
        bool stretchBorders : 1;
};

} // Plasma namespace
#endif // multiple inclusion guard
