
/*
 *   Copyright 2007 by Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
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

#ifndef __PLASMA_LABEL__
#define __PLASMA_LABEL__

#include <plasma/widgets/widget.h>
#include <QtGui/QPen>
#include <QtGui/QFont>

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
    Q_OBJECT
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( Qt::Alignment alignment READ alignment WRITE setAlignment )
    Q_PROPERTY( QPen pen READ pen WRITE setPen )
    Q_PROPERTY( int maximumWidth READ maximumWidth WRITE setMaximumWidth )

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

        #ifdef REMOVE
	/**
         * Reimplemented from Plasma::Widget.
         */
        QSizeF sizeHint() const;
	#endif
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
         * Sets the maximum width that this label should extend to
         *
         * @arg width the new maximum width in pixels
         */
        void setMaximumWidth(int width);

        /**
         * The width in pixels that this label is constrained to
         */
        int maximumWidth() const;

        /**
         * Sets the font used for the text.
         */
        void setFont(const QFont& font);

        /**
         * Returns the font used for the text.
         */
        QFont font() const;

        /**
         * Paint function.
         */
        void paintWidget(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    private:
        class Private;
        Private * const d;
};

}

#endif /* __PLASMA_LABEL__ */
