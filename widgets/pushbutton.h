/*
 *   Copyright (C) 2007 by Siraj Razick siraj@kdemail.net
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

#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include <QObject>
#include <QGraphicsTextItem>

#include <kdemacros.h>

#include "datavisualization.h"

//TODO
//Please Document this class

namespace Plasma
{

class KDE_EXPORT PushButton : public DataVisualization,public QGraphicsItem
{
    Q_OBJECT
    public:
        typedef enum {RECT=0,ROUND,COUSTOM} ButtonShape;
        typedef enum {NONE,HOVER,PRESSED,RELEASED} ButtonState;

        PushButton(QGraphicsItem *parent = 0);
        ~PushButton();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
        QRectF boundingRect() const;
         void setText(const QString&) ;
        QString text();
        QSize size();
        void setSize(QSize size);
        void setWidth(int width);
        void setHeight(int height);
        void setMaximumWidth(int maxwidth);
        int width();
        int height();

    public slots:
        void data(const DataSource::Data&);
    signals:
        void clicked();
    protected:
        bool isDown();
        void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    private:
        class Private ;
        Private *  const d;

};

} // namespace Plasma

#endif
