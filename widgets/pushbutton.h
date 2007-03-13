/*
 *   Copyright (C) 2007 by Siraj Razick siraj@kde.org
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
#include <QLayoutItem>
#include <kdemacros.h>

#include "datavisualization.h"

//TODO
//Please Document this class

namespace Plasma
{

class KDE_EXPORT PushButton : public DataVisualization,
                              public QGraphicsItem,
                              public QLayoutItem
{
        Q_OBJECT
    public:
        enum ButtonShape
        {
            Rectangle = 0,
            Round,
            Custom
        };

        enum ButtonState
        {
            None,
            Hover,
            Pressed,
            Released
        };

    public:
        PushButton(QGraphicsItem *parent = 0);
        virtual ~PushButton();

        QString text() const;
        void setText(const QString &name);

        QSize size() const;
        void setSize(QSize size);

        int height() const;
        void setHeight(int height);

        int width() const;
        void setWidth(int width);

        void setIcon(const QString& path);
        void setMaximumWidth(int maxwidth);

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
        virtual QRectF boundingRect() const;

        //layout stufff
        virtual QSize sizeHint() const ; 
        virtual QSize minimumSize() const;
        virtual QSize maximumSize() const ;
        virtual Qt::Orientations expandingDirections() const;
        virtual void setGeometry(const QRect& r);
        virtual QRect geometry() const ;
        virtual bool isEmpty() const {return false;}

    public Q_SLOTS:
        void data(const DataSource::Data&);

    Q_SIGNALS:
        void clicked();

    protected:
        bool isDown();
        void mousePressEvent(QGraphicsSceneMouseEvent *event);

        void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
    private:
        class Private ;
        Private *  const d;

};

} // namespace Plasma

#endif
