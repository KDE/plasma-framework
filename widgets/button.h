/*
 *   Copyright (C) 2007 by Siraj Razick <siraj@kdemali.net>
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

#ifndef BUTTON_H_
#define BUTTON_H_

//QT 
#include <QGraphicsItem>
// KDE 

//Plasma
#include "widget.h"


namespace Plasma
{

class KDE_EXPORT  Button : public Widget
{
    Q_OBJECT 
   public:
        typedef enum {RECT=0,ROUND,COUSTOM} ButtonShape;
        typedef enum {NONE,HOVER,PRESSED,RELEASED} ButtonState;

	Button(QGraphicsItem * parent = 0);
	virtual ~Button();
        void  paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );
        QRectF boundingRect() const; 
        void setText(const QString&) const;
        QString text();
        QSize size();
        void setSize(QSize size);
        void setWidth(int width);
        void setHeight(int height);
        int width();
        int height();
        
    public slots:
        virtual void data(const DataSource::Data&) {};
    signals:
        void clicked();

  protected:
	/**
	initializer called from constructor 
	**/
	void setDefaultStates();	
        virtual void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
        virtual void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
        virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event);
        void drawOverlay(QPainter * painter);
        void drawBackDrop(QPainter * patiner);
        void drawSVG(QPainter * painter , const QString & imageName);
  private:
	class Private;
        Private * const d;

};

} // Plasma namespace 

#endif
