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

#include <kdemacros.h>

#include "datavisualization.h"
#include "layoutitem.h"

namespace Plasma
{

class Layout;

/**
 * Class that emulates a QWidget inside plasma
 */
class KDE_EXPORT Widget : public DataVisualization,
			  public QGraphicsItem,
                    	  public LayoutItem
{
	Q_OBJECT

    public:
		Widget(Widget *parent = 0);
        	virtual ~Widget();

		virtual Qt::Orientations expandingDirections() const;

		virtual QSizeF minimumSize() const;
		virtual QSizeF maximumSize() const;

		virtual bool hasHeightForWidth() const;
		virtual qreal heightForWidth(qreal w) const;

		virtual bool hasWidthForHeight() const;
		virtual qreal widthForHeight(qreal h) const;

		QRectF geometry() const;
		void setGeometry(const QRectF& geometry);

		void updateGeometry();

		virtual void invalidate();

		virtual QSizeF sizeHint() const;

		QSizeF size() const;

		virtual QRectF boundingRect() const;

		void resize(const QSizeF& size);
		void resize(qreal w, qreal h);

		void setLayout(Layout *l);
		Layout *layout() const;

		Widget *parent() const;
		void reparent(Widget *w);

		void addChild(Widget *w);

	public Q_SLOTS:
        virtual void data(const DataSource::Data&);


    private:
        class Private;
        Private *const d;
};

} // Plasma namespace

#endif
