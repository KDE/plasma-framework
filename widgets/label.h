
/*
 *   Copyright (C) 2007 by Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
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

#ifndef __PLASMA_LABEL__
#define __PLASMA_LABEL__

#include <plasma/widgets/widget.h>

class QGraphicsTextItem;

namespace Plasma
{

/**
 * Simple Text Label.
 *
 * @author Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
 *
 * This class is a simple text label, it just draws plain text.
 */
class PLASMA_EXPORT Label : public Plasma::Widget
{
    public:

        /**
         * Constructor.
         */
        Label(Widget *parent);

        /**
         * Virtual Destructor.
         */
        virtual ~Label();

        /**
         * Labels can expand in Horizontal and Vertical directions.
         */
        Qt::Orientations expandingDirections() const;

        /**
         * Labels can use height-for-width geometry management.
         */
        bool hasHeightForWidth() const;

        /**
         * Reimplemented from Plasma::Widget.
         */
        qreal heightForWidth(qreal w) const;

        /**
         * Reimplemented from Plasma::Widget.
         */
        QSizeF sizeHint() const;

        /**
         * Sets the text to be displayed.
         */
        void setText(const QString& text);

        /**
         * Returns the displayed text.
         */
        QString text() const;

        /**
         * Sets the alignment of the displayed text.
         */
        void setAlignment(Qt::Alignment align);

        /**
         * Returns the alignment of the displayed text.
         */
        Qt::Alignment alignment() const;

        /**
         * Sets the pen used to paint the text.
         */
        void setPen(const QPen& pen);

        /**
         * Returns the pen used to paint the text.
         */
        QPen pen() const;

        /**
         * Paint function.
         */
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    private:
        class Private;
        Private * const d;
};

}

#endif /* __PLASMA_LABEL__ */
