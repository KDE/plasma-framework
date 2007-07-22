/*
 *   Copyright (C) 2007 by Alexander Wiedenbruch <mail@wiedenbruch.de>
 *                      and Matias Valdenegro <mvaldenegro@informatica.utem.cl>
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

#ifndef WIDGET_H_
#define WIDGET_H_

#include <QtGui/QGraphicsItem>

#include <QtCore/QRectF>
#include <QtCore/QSizeF>

#include <plasma/widgets/layoutitem.h>
#include <plasma/plasma_export.h>

namespace Plasma
{

class Layout;

/**
 * Base class for all Widgets in Plasma.
 *
 * @author Alexander Wiedenbruch and Matias Valdenegro.
 *
 * Widgets are the basis for User Interfaces inside Plasma.
 * Widgets are rectangular, but can be in any visible shape by just using transparency to mask
 * out non-rectangular areas.
 *
 * To implement a Widget, just subclass Plasma::Widget and implement at minimum, sizeHint() and paint()
 */
class PLASMA_EXPORT Widget : public QGraphicsItem,
                             public LayoutItem
{
    public:

        /**
         * Constructor.
         */
        explicit Widget(QGraphicsItem *parent = 0);

        /**
         * Virtual Destructor.
         */
        virtual ~Widget();

        /**
         * Returns a bitmask with the directions that this Widget can be expanded.
         */
        virtual Qt::Orientations expandingDirections() const;

        /**
         * Returns the minimum size of this Widget and it's contents.
         */
        virtual QSizeF minimumSize() const;

        /**
         * Returns the maximum size of this Widget.
         */
        virtual QSizeF maximumSize() const;

        /**
         * Returns true whatever this Widget can use height-for-width layout management,
         * false otherwise.
         */
        virtual bool hasHeightForWidth() const;

        /**
         * Returns the corresponding height for a given width.
         * @param w Width
         */
        virtual qreal heightForWidth(qreal w) const;

        /**
         * Returns true whatever this Widget can use width-for-height layout management,
         * false otherwise.
         */
        virtual bool hasWidthForHeight() const;

        /**
         * Returns the corresponding width for a given height.
         * @param h Height
         */
        virtual qreal widthForHeight(qreal h) const;

        /**
         * Returns the geometry of this Widget, in parent coordinates.
         */
        QRectF geometry() const;

        /**
         * Returns the geometry of this Widget, in local coordinates.
         */
        QRectF localGeometry() const;

        /**
         * Sets the geometry of this Widget.
         */
        void setGeometry(const QRectF &geometry);

        /**
         * Propagates the geometry information to associated layouts and other Widgets.
         */
        void updateGeometry();

        /**
         * Invalidates the geometry information.
         */
        virtual void invalidate();

        /**
         * Returns the most appropiate size of this Widget to hold it's contents.
         */
        virtual QSizeF sizeHint() const;

        /**
         * Returns the size of this Widget.
         */
        QSizeF size() const;

        /**
         * Returns the bounding rectangle of this Widget.
         */
        virtual QRectF boundingRect() const;

        /**
         * Resizes this Widget.
         * @param size New size
         */
        void resize(const QSizeF &size);

        /**
         * Resizes this Widget.
         * @param w New width
         * @param h New height
         */
        void resize(qreal w, qreal h);

        /**
         * Sets the Layout that will manage this Widget's childrens.
         */
        void setLayout(Layout *l);

        /**
         * Returns the Layout associated with this Widget.
         */
        Layout *layout() const;

        /**
         * Returns the parent of this Widget.
         */
        Widget *parent() const;

        /**
         * Changes the parent of this Widget.
         */
        void reparent(Widget *w);

        /**
         * Appends a child Widget to this Widget.
         */
        void addChild(Widget *w);

        /**
         * Paint function. Reimplement to draw on this Widget.
         */
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

private:
    class Private;
    Private *const d;
};

} // Plasma namespace

#endif
